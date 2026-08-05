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
SemaphoreHandle_t i2cMutex;
TaskHandle_t TaskRollPitch_Handle;
TaskHandle_t TaskYaw_Handle;

// ==========================================
// 1. DATA ACQUISTION
// ==========================================

// Function for the 500Hz Task
void readIMUData(Vector3 &gyro, Vector3 &accel) {
// Wait for the I2C bus to be free
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
        mpu.getRawAll();
        xSemaphoreGive(i2cMutex);
    }
    
    gyro = {mpu.getGyroX(), mpu.getGyroY(), mpu.getGyroZ()};
    accel = {mpu.getAccX(), mpu.getAccY(), mpu.getAccZ()}; 
    accel.normalize();
}

// ============================================================
// 2. READ MAGNETOMETER DATA (75 Hz)
// ============================================================
bool readMagData(Vector3 &mag_vec)
{
    bool ready = false;
    // --------------------------------------------------------
    // Protect I2C bus
    // --------------------------------------------------------
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
    {
        // Check DRDY flag set by ISR
        ready = mag.isDataReady();

        // Only read sensor when new data is available
        if (ready)
        {
            mag.getRawAll();
        }

        xSemaphoreGive(i2cMutex);
    }

    // --------------------------------------------------------
    // No new magnetometer data
    // --------------------------------------------------------
    if (!ready)
    {
        return false;
    }

    // --------------------------------------------------------
    // Convert raw data to magnetic field
    // --------------------------------------------------------
    mag_vec = {
        mag.getX(),
        mag.getY(),
        mag.getZ()
    };

    // --------------------------------------------------------
    // Normalize magnetic vector
    // --------------------------------------------------------
    mag_vec.normalize();

    return true;
}

// ==========================================
// TELEMETRY "BLACK BOX" (Stores latest data)
// ==========================================
struct TelemetryData {
    // Timing
    float dt = 0.0f;
    uint32_t pilot_total_us = 0;
    uint32_t nav_total_us = 0;

    // Raw Sensors
    Vector3 accel;
    Vector3 gyro;
    Vector3 mag;
    bool mag_updated = false;

    // Output Angles
    float ekf_roll = 0.0f, ekf_pitch = 0.0f, ekf_yaw = 0.0f;
};

// Global instance to hold our data
TelemetryData tlm;

// ==========================================
// SEPARATED PRINT FUNCTION
// ==========================================
void printEKFValidation() {
    // 1. Calculate "Raw" Angles from Accelerometer for comparison
    // These will be noisy! It proves your EKF is doing its job by smoothing them.
    float raw_roll = atan2f(tlm.accel(1,0), tlm.accel(2,0)) * 57.2958f;
    float raw_pitch = atan2f(-tlm.accel(0,0), sqrtf(tlm.accel(1,0)*tlm.accel(1,0) + tlm.accel(2,0)*tlm.accel(2,0))) * 57.2958f;

    // 2. Print the beautifully formatted validation block
    Serial.println("\n============= EKF VALIDATION =============");
    Serial.printf("TIMING  | dt: %.4f s | Pilot: %lu us | Nav: %lu us\n", 
                  tlm.dt, tlm.pilot_total_us, tlm.nav_total_us);
    
    Serial.println("-------- RAW SENSORS --------");
    Serial.printf("GYRO    | X: %7.2f | Y: %7.2f | Z: %7.2f\n", tlm.gyro(0,0), tlm.gyro(1,0), tlm.gyro(2,0));
    Serial.printf("ACCEL   | X: %7.2f | Y: %7.2f | Z: %7.2f\n", tlm.accel(0,0), tlm.accel(1,0), tlm.accel(2,0));
    Serial.printf("MAG     | X: %7.2f | Y: %7.2f | Z: %7.2f [Ready: %d]\n", tlm.mag(0,0), tlm.mag(1,0), tlm.mag(2,0), tlm.mag_updated);
    
    Serial.println("-------- FILTER COMPARISON --------");
    Serial.printf("RAW ANG | Roll: %7.2f | Pitch: %7.2f\n", raw_roll, raw_pitch);
    Serial.printf("EKF ANG | Roll: %7.2f | Pitch: %7.2f | Yaw: %7.2f\n", tlm.ekf_roll, tlm.ekf_pitch, tlm.ekf_yaw);
    Serial.println("==========================================");
}

// ==========================================
// UTILITY: TASK PROFILER (Stopwatch)
// ==========================================
class TaskProfiler {
public:
    uint32_t i2c_time;
    uint32_t ekf_time;
    uint32_t total_time;

private:
    uint32_t t_start;
    uint32_t t_i2c;
    uint32_t t_ekf;

public:
    // 1. Call this at the very beginning of the loop
    void start() {
        t_start = micros();
    }

    // 2. Call this right after talking to sensors
    void markI2C() {
        t_i2c = micros();
        i2c_time = t_i2c - t_start;
    }

    // 3. Call this right after the EKF mutex is given back
    void markEKF() {
        t_ekf = micros();
        ekf_time = t_ekf - t_i2c;
        total_time = t_ekf - t_start;
    }
};




// ==========================================
// 3. EKF CLASS
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

    void getEulerAngles(float &roll, float &pitch, float &yaw) {
        float q0 = x(0,0), q1 = x(1,0), q2 = x(2,0), q3 = x(3,0);
        roll  = atan2f(2.0f*(q0*q1 + q2*q3), q0*q0 - q1*q1 - q2*q2 + q3*q3) * 57.2958f;
        pitch = asinf(-2.0f*(q1*q3 - q0*q2)) * 57.2958f;
        yaw   = atan2f(2.0f*(q1*q2 + q0*q3), q0*q0 + q1*q1 - q2*q2 - q3*q3) * 57.2958f;
        
    }
};


DroneEKF ekf;

// ==========================================
// 4. RTOS TASKS (Data flow and Execution)
// ==========================================


// ============================================================
//      TASK: ROLL + PITCH (500 Hz / 2ms)
// ============================================================
void Task_ROLL_PITCH(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2);

    Vector3 accel, gyro;
    float roll = 0.0f, pitch = 0.0f, yaw = 0.0f;
    uint32_t lastTimeUs = micros();
    uint16_t printCounter = 0;

    TaskProfiler profiler; // <--- Create our stopwatch

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        profiler.start(); // START STOPWATCH

        // 1. READ IMU
        readIMUData(accel, gyro);
        profiler.markI2C(); // RECORD I2C TIME

        // 2. CALCULATE DT
        uint32_t nowUs = micros();
        float dt = (nowUs - lastTimeUs) * 1e-6f;
        lastTimeUs = nowUs;
        if (dt <= 0.0f || dt > 0.1f) dt = 0.002f; // Safety Catch

        // 3. EKF MATH
        if (xSemaphoreTake(ekfMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            ekf.predict(dt, gyro);
            ekf.updateAccel(accel);
            ekf.getEulerAngles(roll, pitch, yaw);
            xSemaphoreGive(ekfMutex);
        }
        profiler.markEKF();

        // ==========================================
        // 4. SAVE TO TELEMETRY & PRINT
        // ==========================================
        tlm.accel = accel;
        tlm.gyro = gyro;
        tlm.dt = dt;
        tlm.ekf_roll = roll;
        tlm.ekf_pitch = pitch;
        tlm.ekf_yaw = yaw;
        tlm.pilot_total_us = profiler.total_time;

        printCounter++;
        if (printCounter >= 100) { // Call print function at 5Hz
            printEKFValidation();
            printCounter = 0;
    }
}
}
// ============================================================
// TASK: YAW (~77 Hz / 13 ms)
// ============================================================
void Task_YAW(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(13);

    Vector3 mag_vec;
    uint16_t printCounter = 0;
    TaskProfiler profiler; // <--- Create our stopwatch

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        profiler.start(); // START STOPWATCH

        // 1. READ MAGNETOMETER
        bool gotData = readMagData(mag_vec);
        profiler.markI2C(); // RECORD I2C TIME

        // 2. EKF UPDATE
        if (gotData) {
            if (xSemaphoreTake(ekfMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
                ekf.updateMag(mag_vec);
                xSemaphoreGive(ekfMutex);
            }
        }
        profiler.markEKF(); // RECORD MATH AND TOTAL TIME

        // ==========================================
        // UPDATE TELEMETRY (No printing here)
        // ==========================================
        tlm.mag_updated = gotData;
        if (gotData) {
            tlm.mag = mag_vec;
        }
        tlm.nav_total_us = profiler.total_time;
    } 
}


// ==========================================
// 4. SETUP & LOOP
// ==========================================
void setup() {
    Serial.begin(115200);
    uint8_t DRDY_PIN = 7;
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

    mag.attachDRDYInterrupt(DRDY_PIN);
    printEKFValidation;



    // Start RTOS
    ekfMutex = xSemaphoreCreateMutex();
    i2cMutex = xSemaphoreCreateMutex();
    
    xTaskCreatePinnedToCore(Task_ROLL_PITCH, "Roll Pitch", 8192, NULL, 2, &TaskRollPitch_Handle, 1);
    xTaskCreatePinnedToCore(Task_YAW, "Yaw", 4096, NULL, 1, &TaskYaw_Handle, 0);

}

void loop(){
    vTaskDelete(NULL); 
}