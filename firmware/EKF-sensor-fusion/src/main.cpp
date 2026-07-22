#include <Arduino.h>
#include <Wire.h>
#include "math/Matrix.h"
#include "sensor/GY87_MPU6050.h"
#include "sensor/GY87_HMC5883L.h"

// ==========================================
// 0. HARDWARE OBJECTS
// ==========================================
GY87_MPU6050 mpu;
GY87_HMC5883L mag;

// RTOS Handles
SemaphoreHandle_t ekfMutex;
TaskHandle_t TaskRollPitch_Handle;
TaskHandle_t TaskYaw_Handle;

// ==========================================
// 1. HARDWARE ABSTRACTION LAYER (Data Acquisition)
// ==========================================

// Function for the 500Hz Task
void readIMUData(Vector3 &gyro, Vector3 &accel) {
    mpu.getRawAll();
    gyro = {mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ()};
    accel = {mpu.getAccX(), mpu.getAccY(), mpu.getAccZ()}; 
    accel.normalize();
}

// Function for the 75Hz Task
bool readMagData(Vector3 &mag_vec) {
    if (mag.isDataReady()) {
        mag.getRawAll();
        mag_vec = {mag.getX(), mag.getY(), mag.getZ()};
        mag_vec.normalize();
        return true; // Tell the task we successfully got data
    }
    return false; // No new data available
}

// ==========================================
// 2. EKF CLASS (Encapsulates Math and State)
// ==========================================
class DroneEKF {
private:
    // Internal State Memory
    Vector7 x;      // State Vector [q0, q1, q2, q3, bx, by, bz]
    Matrix<7,7> P;  // Covariance Matrix
    Matrix<7,7> Q;  // Process Noise
    Matrix<3,3> Ra; // Accel Noise
    Matrix<3,3> Rm; // Mag Noise

    float earth_bx;
    float earth_bz;

    void normalizeQuaternion() {
        float magnitude = sqrtf(x(0,0)*x(0,0) + x(1,0)*x(1,0) + x(2,0)*x(2,0) + x(3,0)*x(3,0));
        if(magnitude > 0.0f) {
            for(int i=0; i<4; i++) x(i,0) /= magnitude;
        }
    }

public:
    // Constructor to initialize matrices
    DroneEKF() {
        earth_bx = 1.0f; 
        earth_bz = 0.0f;
        x = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        P = Matrix<7,7>::identity() * 0.01f;
        Q = Matrix<7,7>::zeros();
        
        float q_var = 0.0001f; 
        float b_var = 1e-6f;   
        for(int i=0; i<4; i++) Q(i,i) = q_var;
        for(int i=4; i<7; i++) Q(i,i) = b_var;

        Ra = Matrix<3,3>::identity() * 0.1f;
        Rm = Matrix<3,3>::identity() * 0.5f;
    }

    void predict(float dt, Vector3 gyro_raw) {
        float q0 = x(0,0), q1 = x(1,0), q2 = x(2,0), q3 = x(3,0);
        float dt2 = dt / 2.0f;

        float wx = (gyro_raw(0,0) * 0.0174533f) - x(4,0);  
        float wy = (gyro_raw(1,0) * 0.0174533f) - x(5,0);
        float wz = (gyro_raw(2,0) * 0.0174533f) - x(6,0);

        x(0,0) = q0 - dt2*wx*q1 - dt2*wy*q2 - dt2*wz*q3;
        x(1,0) = q1 + dt2*wx*q0 - dt2*wy*q3 + dt2*wz*q2;
        x(2,0) = q2 + dt2*wx*q3 + dt2*wy*q0 - dt2*wz*q1;
        x(3,0) = q3 - dt2*wx*q2 + dt2*wy*q1 + dt2*wz*q0;

        Matrix<7,7> F_mat = Matrix<7,7>::identity();
        F_mat(0,1) = -dt2*wx; F_mat(0,2) = -dt2*wy; F_mat(0,3) = -dt2*wz; F_mat(0,4) =  dt2*q1; F_mat(0,5) =  dt2*q2; F_mat(0,6) =  dt2*q3;
        F_mat(1,0) =  dt2*wx; F_mat(1,2) =  dt2*wz; F_mat(1,3) = -dt2*wy; F_mat(1,4) = -dt2*q0; F_mat(1,5) =  dt2*q3; F_mat(1,6) = -dt2*q2;
        F_mat(2,0) =  dt2*wy; F_mat(2,1) = -dt2*wz; F_mat(2,3) =  dt2*wx; F_mat(2,4) = -dt2*q3; F_mat(2,5) = -dt2*q0; F_mat(2,6) =  dt2*q1;
        F_mat(3,0) =  dt2*wz; F_mat(3,1) =  dt2*wy; F_mat(3,2) = -dt2*wx; F_mat(3,4) =  dt2*q2; F_mat(3,5) = -dt2*q1; F_mat(3,6) = -dt2*q0;

        P = (F_mat * P * F_mat.transpose()) + Q;
    }

    void updateAccel(Vector3 z_a) {
        float q0 = x(0,0), q1 = x(1,0), q2 = x(2,0), q3 = x(3,0);

        Vector3 h_a = { 2.0f*(q1*q3 - q0*q2), 2.0f*(q0*q1 + q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3 };

        Matrix<3,7> Ha = Matrix<3,7>::zeros();
        Ha(0,0) = -2.0f*q2; Ha(0,1) =  2.0f*q3; Ha(0,2) = -2.0f*q0; Ha(0,3) =  2.0f*q1;
        Ha(1,0) =  2.0f*q1; Ha(1,1) =  2.0f*q0; Ha(1,2) =  2.0f*q3; Ha(1,3) =  2.0f*q2;
        Ha(2,0) =  2.0f*q0; Ha(2,1) = -2.0f*q1; Ha(2,2) = -2.0f*q2; Ha(2,3) =  2.0f*q3;

        Vector3 y_a = z_a - h_a;
        Matrix<3,3> S = (Ha * P * Ha.transpose()) + Ra;
        Matrix<7,3> K = P * Ha.transpose() * S.inverse3x3();

        x = x + (K * y_a);
        P = (Matrix<7,7>::identity() - (K * Ha)) * P;
        normalizeQuaternion();
    }

    void updateMag(Vector3 z_m) {
        float q0 = x(0,0), q1 = x(1,0), q2 = x(2,0), q3 = x(3,0);
        float bx = earth_bx, bz = earth_bz;

        Vector3 h_m = {
            bx*(q0*q0 + q1*q1 - q2*q2 - q3*q3) + 2.0f*bz*(q1*q3 - q0*q2),
            2.0f*bx*(q1*q2 - q0*q3) + 2.0f*bz*(q0*q1 + q2*q3),
            2.0f*bx*(q0*q2 + q1*q3) + bz*(q0*q0 - q1*q1 - q2*q2 + q3*q3)
        };

        Matrix<3,7> Hm = Matrix<3,7>::zeros();
        Hm(0,0) = 2.0f*bx*q0 - 2.0f*bz*q2; Hm(0,1) = 2.0f*bx*q1 + 2.0f*bz*q3; Hm(0,2) = -2.0f*bx*q2 - 2.0f*bz*q0; Hm(0,3) = -2.0f*bx*q3 + 2.0f*bz*q1;
        Hm(1,0) = -2.0f*bx*q3 + 2.0f*bz*q1; Hm(1,1) = 2.0f*bx*q2 + 2.0f*bz*q0; Hm(1,2) = 2.0f*bx*q1 + 2.0f*bz*q3; Hm(1,3) = -2.0f*bx*q0 + 2.0f*bz*q2;
        Hm(2,0) = 2.0f*bx*q2 + 2.0f*bz*q0; Hm(2,1) = 2.0f*bx*q3 - 2.0f*bz*q1; Hm(2,2) = 2.0f*bx*q0 - 2.0f*bz*q2; Hm(2,3) = 2.0f*bx*q1 + 2.0f*bz*q3;

        Vector3 y_m = z_m - h_m;
        Matrix<3,3> S = (Hm * P * Hm.transpose()) + Rm;
        Matrix<7,3> K = P * Hm.transpose() * S.inverse3x3();

        x = x + (K * y_m);
        P = (Matrix<7,7>::identity() - (K * Hm)) * P;
        normalizeQuaternion();
    }

    // Clean API for Control Team to read Data
    void getEulerAngles(float &roll, float &pitch, float &yaw) {
        float q0 = x(0,0), q1 = x(1,0), q2 = x(2,0), q3 = x(3,0);
        roll  = atan2f(2.0f*(q0*q1 + q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3) * 57.2958f;
        pitch = asinf(-2.0f*(q1*q3 - q0*q2)) * 57.2958f;
        yaw   = atan2f(2.0f*(q1*q2 + q0*q3), q0*q0 + q1*q1 - q2*q2 - q3*q3) * 57.2958f;
        
    }
};

// Global Instance of the EKF
DroneEKF ekf;

// ==========================================
// 3. RTOS TASKS (Data flow and Execution)
// ==========================================
// CORE 1: High Speed Pilot Task (500Hz)
void Task_ROLL_PITCH(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 2; 
    float dt = 0.002f;

    Vector3 gyro, accel; // Temporary storage
    float roll, pitch, yaw; // Variables to store our angles
    int printTimer = 0;     // Counter to slow down the Serial monitor

    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // 1. ACQUIRE DATA (Clean and abstracted)
        readIMUData(gyro, accel);

        // 2. RUN EKF
        if (xSemaphoreTake(ekfMutex, portMAX_DELAY) == pdTRUE) {
            ekf.predict(dt, gyro);
            ekf.updateAccel(accel);
            xSemaphoreGive(ekfMutex);
        
        // 3. GET THE DATA OUT OF THE EKF
            ekf.getEulerAngles(roll, pitch, yaw);
            
            xSemaphoreGive(ekfMutex);
        }

        // 4. PRINT SAFELY (Only 1 out of every 50 loops -> 10Hz)
        printTimer++;
        if (printTimer >= 50) {
            Serial.printf("Roll: %.2f | Pitch: %.2f | Yaw: %.2f\n", roll, pitch, yaw);
            printTimer = 0; // Reset the counter    
        }
    }
}

// CORE 0: Low Speed Navigator Task (75Hz)
void Task_YAW(void *pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = 13; 

    Vector3 mag_vec;

    while(1) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // 1. ACQUIRE DATA
        if (readMagData(mag_vec)) { 
            // 2. RUN EKF
            if (xSemaphoreTake(ekfMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
                ekf.updateMag(mag_vec);
                xSemaphoreGive(ekfMutex);
            }
        }
    }
}

// ==========================================
// 4. SETUP & LOOP
// ==========================================
void setup() {
    Serial.begin(115200);

    uint8_t SDA_PIN = 10; 
    uint8_t SCL_PIN = 9;

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000); 
    delay(50);
    
    mpu.init(SDA_PIN, SCL_PIN);
    mpu.enableBypass(); 
    delay(100);
    
    mag.init(SDA_PIN, SCL_PIN);
    mag.setMeasMode(CONTINUOUS_MODE);
    mag.setOutputRate(RATE_75);
    mag.setGain(FIELD_RANGE_1_3);

    // EKF is now automatically initialized by its Constructor!

    // Start RTOS
    ekfMutex = xSemaphoreCreateMutex();
    
    xTaskCreatePinnedToCore(Task_ROLL_PITCH, "Raw Pitch", 8192, NULL, 2, &TaskRollPitch_Handle, 1);
    xTaskCreatePinnedToCore(Task_YAW, "Yaw", 4096, NULL, 1, &TaskYaw_Handle, 0);



}

void loop() {
    vTaskDelete(NULL); 
}