# Deep Dive: `stabilizerTask` & `sensorsTask`

## 1. The Big Picture — One Cycle at 1 kHz

The following sequence diagram shows exactly what happens in **every single 1 ms cycle** of the Crazyflie's flight controller:

```mermaid
sequenceDiagram
    participant IMU as BMI088 Gyro (HW)
    participant EXTI as EXTI14 ISR
    participant ST as sensorsTask (Pri 4)
    participant EQ as measurementsQueue
    participant STAB as stabilizerTask (Pri 5)
    participant EST as State Estimator
    participant CTRL as Controller (PID)
    participant PD as Power Distribution
    participant MOT as Motors (PWM)

    IMU->>EXTI: Data-Ready INT (PC14 rising edge)
    Note over EXTI: imuIntTimestamp = usecTimestamp()
    EXTI->>ST: vTaskNotifyGiveFromISR()

    activate ST
    ST->>ST: Read gyro & accel via I2C/SPI
    ST->>ST: Bias subtract, scale, align, LPF
    ST->>EQ: estimatorEnqueue(gyro)
    ST->>EQ: estimatorEnqueue(accel)
    Note over ST: Every 20th cycle: read baro, enqueue
    ST->>ST: xQueueOverwrite(gyroDataQueue)
    ST->>ST: xQueueOverwrite(accelDataQueue)
    ST->>STAB: xSemaphoreGive(dataReady)
    deactivate ST

    Note over STAB: Preempts sensorsTask (Pri 5 > 4)
    activate STAB
    STAB->>STAB: sensorsAcquire() — drain queues
    STAB->>EST: stateEstimator(&state, tick)
    Note over EST: Fuse IMU + external measurements
    EST-->>STAB: Updated state (pos, vel, attitude)
    STAB->>STAB: commanderGetSetpoint(&setpoint)
    STAB->>STAB: supervisorUpdate() & Override
    STAB->>CTRL: controller(&control, &setpoint, &sensors, &state)
    Note over CTRL: Cascaded PID → roll/pitch/yaw/thrust
    CTRL-->>STAB: control_t {thrust, roll, pitch, yaw}
    STAB->>PD: powerDistribution(&control) → motor mixing
    PD-->>STAB: motorPwm[M1..M4]
    STAB->>MOT: motorsSetRatio(M1..M4)
    deactivate STAB
```

---

## 2. `sensorsTask` — The Sensor Producer

> **Source:** [sensors_bmi088_bmp3xx.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/hal/src/sensors_bmi088_bmp3xx.c#L292-L377)

### 2.1 What It Does

The `sensorsTask` is the **data producer** of the flight control pipeline. It:

1. **Sleeps** until the IMU hardware fires a data-ready interrupt
2. **Reads** raw gyro and accelerometer data over I2C/SPI from the BMI088
3. **Processes** the raw data (calibration, bias subtraction, coordinate alignment, low-pass filtering)
4. **Publishes** the processed data through two parallel channels:
   - **`measurementsQueue`** → for the state estimator (Kalman filter)
   - **Single-element overwrite queues** (`gyroDataQueue`, `accelerometerDataQueue`) → for the stabilizer to read via `sensorsAcquire()`
5. **Signals** the `stabilizerTask` via the `dataReady` semaphore

### 2.2 Frequency

| Sensor | Hardware ODR | Software Read Rate | Mechanism |
|---|---|---|---|
| **BMI088 Gyroscope** | 1000 Hz | **1000 Hz** | Interrupt-driven (`EXTI14`) |
| **BMI088 Accelerometer** | 1600 Hz | **1000 Hz** (polled in sync with gyro) | Read alongside gyro each cycle |
| **BMP388/390 Barometer** | 50 Hz | **50 Hz** | Decimated: read every 20th cycle (`SENSORS_DELAY_BARO = 1000/50 = 20`) |
| **Magnetometer** | 20 Hz | **20 Hz** | Decimated: read every 50th cycle |

The master clock for the entire system is the **BMI088 gyroscope's 1 kHz data-ready interrupt**.

### 2.3 Interrupt → Task Wakeup Chain

```
BMI088 Gyro INT3 pin → STM32 PC14 → EXTI Line 14
    → EXTI15_10_IRQHandler()
        → EXTI14_Callback()
            → sensorsBmi088Bmp3xxDataAvailableCallback()
```

The callback ([line 994](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/hal/src/sensors_bmi088_bmp3xx.c#L994-L1004)):
```c
void sensorsBmi088Bmp3xxDataAvailableCallback(void) {
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  imuIntTimestamp = usecTimestamp();                         // Capture exact HW timestamp
  vTaskNotifyGiveFromISR(sensorsTaskHandle, &xHigherPriorityTaskWoken);  // Wake sensorsTask
  if (xHigherPriorityTaskWoken) { portYIELD(); }            // Context switch immediately
}
```

### 2.4 Signal Processing Pipeline

Every 1 ms, the raw sensor data goes through this pipeline:

```mermaid
graph LR
    subgraph "1. Raw Read (I2C/SPI)"
        RG["gyroRaw (int16 × 3)"]
        RA["accelRaw (int16 × 3)"]
    end
    subgraph "2. Calibration"
        BG["Subtract gyroBias"]
        SA["Scale & normalize accel"]
    end
    subgraph "3. Scaling"
        SG["× DEG_PER_LSB\n(0.061 °/s/LSB)"]
        SGA["× G_PER_LSB\n(7.3e-4 G/LSB)"]
    end
    subgraph "4. Alignment"
        AA["sensorsAlignToAirframe()\n(3D Euler rotation)"]
        AG["sensorsAccAlignToGravity()\n(pitch/roll trim)"]
    end
    subgraph "5. Low-Pass Filter"
        LG["2nd-order Biquad\nGyro: fc=80 Hz"]
        LA["2nd-order Biquad\nAccel: fc=30 Hz"]
    end
    subgraph "6. Output"
        OG["sensorData.gyro\n(°/s, body frame)"]
        OA["sensorData.acc\n(Gs, body frame)"]
    end

    RG --> BG --> SG --> AA --> LG --> OG
    RA --> SA --> SGA --> AA
    AA --> AG --> LA --> OA
```

**Key constants:**
- Gyro scale: $(2 \times 2000) / 65536 \approx 0.061\text{ °/s/LSB}$ (±2000 °/s range)
- Accel scale: $(2 \times 24) / 65536 \approx 7.3 \times 10^{-4}\text{ G/LSB}$ (±24G range)
- Gyro LPF cutoff: **80 Hz**
- Accel LPF cutoff: **30 Hz**

---

## 3. `stabilizerTask` — The Flight Controller

> **Source:** [stabilizer.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/stabilizer.c#L289-L382)

### 3.1 What It Does

The `stabilizerTask` is the **real-time control loop**. Every 1 ms it:

1. **Blocks** on `sensorsWaitDataReady()` — waits for the `dataReady` semaphore from `sensorsTask`
2. **Acquires** the latest sensor data from the overwrite queues
3. **Estimates** the current state (position, velocity, attitude) using the selected estimator
4. **Gets** the desired setpoint from the commander
5. **Runs** the safety supervisor (can override setpoints or kill motors)
6. **Calculates** the control output (thrust, roll, pitch, yaw torques) via the selected controller
7. **Distributes** the control output to individual motor PWM signals
8. **Sends** PWM to the 4 motors

### 3.2 Sub-Rate Execution

Not every step runs at the full 1 kHz. The `RATE_DO_EXECUTE` macro ([line 375](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/interface/stabilizer_types.h#L375)) decimates certain operations:

```c
#define RATE_DO_EXECUTE(RATE_HZ, TICK) ((TICK % (RATE_MAIN_LOOP / RATE_HZ)) == 0)
```

| Sub-System | Rate | Defined As |
|---|---|---|
| **Main stabilizer loop** | 1000 Hz | `RATE_MAIN_LOOP` |
| **Attitude controller (PID inner loop)** | 500 Hz | `ATTITUDE_RATE` |
| **Position controller (PID outer loop)** | 100 Hz | `POSITION_RATE` |
| **High-level commander** | 100 Hz | `RATE_HL_COMMANDER` |
| **Supervisor safety checks** | 25 Hz | `RATE_SUPERVISOR` |

### 3.3 The Main Loop (annotated)

```c
// stabilizer.c:315-381
while(1) {
    sensorsWaitDataReady();                    // BLOCK until sensorsTask gives dataReady
    sensorsAcquire(&sensorData);              // Drain latest data from overwrite queues

    // --- State Estimation ---
    stateEstimator(&state, stabilizerStep);    // Complementary filter or EKF

    // --- Safety Supervisor ---
    const bool areMotorsAllowedToRun = supervisorAreMotorsAllowedToRun();
    crtpCommanderBlock(!areMotorsAllowedToRun);  // Block incoming commands if unsafe

    // --- Setpoint Acquisition ---
    if (crtpCommanderHighLevelGetSetpoint(&tempSetpoint, &state, stabilizerStep)) {
        commanderSetSetpoint(&tempSetpoint, COMMANDER_PRIORITY_HIGHLEVEL);
    }
    commanderGetSetpoint(&setpoint, &state);

    // --- Supervisor Override ---
    supervisorUpdate(&sensorData, &setpoint, stabilizerStep);
    collisionAvoidanceUpdateSetpoint(&setpoint, &sensorData, &state, stabilizerStep);
    supervisorOverrideSetpoint(&setpoint);    // Can force thrust=0 if tumbling

    // --- Controller ---
    controller(&control, &setpoint, &sensorData, &state, stabilizerStep);

    // --- Motor Output ---
    if (areMotorsAllowedToRun) {
        controlMotors(&control);               // Power dist → battery comp → cap → PWM
    } else {
        motorsStop();                          // Force all motors to 0
    }

    xSemaphoreGive(xRateSupervisorSemaphore); // Signal rate supervisor
    stabilizerStep++;
}
```

---

## 4. Shared Resources & Protection

### 4.1 Complete IPC Map

| Resource | Type | Producer | Consumer | Protection |
|---|---|---|---|---|
| `dataReady` | Binary Semaphore (static) | `sensorsTask` | `stabilizerTask` | Signaling only; no data race |
| `gyroDataQueue` | 1-element Queue (static) | `sensorsTask` | `stabilizerTask` | `xQueueOverwrite` / `xQueueReceive` — FreeRTOS thread-safe |
| `accelerometerDataQueue` | 1-element Queue (static) | `sensorsTask` | `stabilizerTask` | Same as above |
| `barometerDataQueue` | 1-element Queue (static) | `sensorsTask` | `stabilizerTask` | Same as above |
| `measurementsQueue` | 20-element Queue (static) | `sensorsTask` + Deck tasks | State Estimator (EKF) | `xQueueSend` / `xQueueSendFromISR` — thread/ISR-safe |
| `canStartMutex` | Mutex (static) | `systemTask` | All tasks (`systemWaitStart()`) | Blocks all flight tasks until boot complete |
| `xRateSupervisorSemaphore` | Binary Semaphore | `stabilizerTask` | `rateSupervisorTask` | Rate monitoring; if not received in 2s → ASSERT |
| `imuIntTimestamp` | `volatile uint64_t` | EXTI ISR | `sensorsTask` | `volatile` ensures compiler doesn't optimize away; single-writer pattern |

### 4.2 Why It's Safe — Priority-Based Preemption

The key insight is that **the task priorities enforce a strict execution order**:

```
Priority 5 (HIGHEST) ─── stabilizerTask, rateSupervisorTask
Priority 4            ─── sensorsTask
Priority 3            ─── syslinkTask, usblinkTask, flowdeckTask, lighthouseTask
Priority 2            ─── systemTask, crtpTx/Rx, commanderHighLevel, kalmanTask
Priority 1            ─── logTask, paramTask, workerTask
Priority 0 (LOWEST)   ─── pmTask, proximityTask
```

> [!IMPORTANT]
> When `sensorsTask` (Pri 4) calls `xSemaphoreGive(dataReady)`, FreeRTOS immediately preempts it in favor of `stabilizerTask` (Pri 5), which is blocked on `xSemaphoreTake(dataReady)`. This guarantees **minimal and deterministic latency** (~μs) between sensor data becoming available and the control loop processing it.

### 4.3 Why Single-Element Queues with `xQueueOverwrite`?

The sensor queues are **1-element deep** and use `xQueueOverwrite()` instead of `xQueueSend()`:

```c
// sensors_bmi088_bmp3xx.c:109-116
static xQueueHandle accelerometerDataQueue;
STATIC_MEM_QUEUE_ALLOC(accelerometerDataQueue, 1, sizeof(Axis3f));  // Only 1 slot!
```

This is a **deliberate design choice**:
- `xQueueOverwrite` **never blocks** and always succeeds — it overwrites stale data
- The consumer (`stabilizerTask`) always gets the **freshest** sample, never stale data
- If the consumer is slow, old data is simply discarded — this is correct for a real-time control loop where freshness matters more than completeness

---

## 5. From Controller Output to Motor PWM

### 5.1 The Cascaded PID Controller

> **Source:** [controller_pid.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/controller/controller_pid.c#L56-L163)

The default PID controller uses a **cascaded (two-loop) architecture**:

```mermaid
graph LR
    subgraph "Outer Loop (100 Hz)"
        SP["Setpoint\n(x,y,z position)"]
        PC["Position PID"]
    end
    subgraph "Middle Loop (500 Hz)"
        AD["Attitude Desired\n(roll, pitch, yaw)"]
        AC["Attitude PID"]
    end
    subgraph "Inner Loop (500 Hz)"
        RD["Rate Desired\n(roll_rate, pitch_rate, yaw_rate)"]
        RC["Rate PID"]
        GYRO["Gyro Feedback\n(°/s)"]
    end
    subgraph "Output"
        CO["control_t\n{thrust, roll, pitch, yaw}"]
    end

    SP --> PC -->|thrust, roll°, pitch°| AD
    AD --> AC -->|roll_rate, pitch_rate, yaw_rate| RD
    RD --> RC --> CO
    GYRO -->|Direct feedback| RC
```

**At 100 Hz** — Position PID:
```c
positionController(&actuatorThrust, &attitudeDesired, setpoint, state);
```
Converts position/velocity errors → desired attitude angles (roll, pitch) and thrust.

**At 500 Hz** — Attitude PID:
```c
attitudeControllerCorrectAttitudePID(
    state->attitude.roll, state->attitude.pitch, state->attitude.yaw,
    attitudeDesired.roll, attitudeDesired.pitch, attitudeDesired.yaw,
    &rateDesired.roll, &rateDesired.pitch, &rateDesired.yaw);
```
Converts attitude errors → desired angular rates.

**At 500 Hz** — Rate PID:
```c
attitudeControllerCorrectRatePID(
    sensors->gyro.x, -sensors->gyro.y, sensors->gyro.z,  // Gyro feedback
    rateDesired.roll, rateDesired.pitch, rateDesired.yaw);

attitudeControllerGetActuatorOutput(&control->roll, &control->pitch, &control->yaw);
```
Converts angular rate errors → torque commands (roll, pitch, yaw numbers).

### 5.2 Power Distribution (Motor Mixing)

> **Source:** [power_distribution_quadrotor.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/power_distribution_quadrotor.c#L84-L117)

The controller outputs `{thrust, roll, pitch, yaw}`. The **mixer** converts these into individual motor thrusts.

**Legacy mode** (simple integer mixing):
```c
// Motor layout (X configuration):
//    M1(CW)   M4(CCW)
//        \   /
//         \ /
//         / \
//        /   \
//    M2(CCW)  M3(CW)

r = control->roll / 2.0f;
p = control->pitch / 2.0f;

M1 = thrust - r + p + yaw;
M2 = thrust - r - p - yaw;
M3 = thrust + r - p + yaw;
M4 = thrust + r + p - yaw;
```

**Force/Torque mode** (SI-unit based, used by Mellinger/Lee controllers):
$$M_i = \frac{T}{4} \pm \frac{\tau_x}{4 \cdot l_{arm}} \pm \frac{\tau_y}{4 \cdot l_{arm}} \pm \frac{\tau_z}{4 \cdot k_\tau}$$

Where $l_{arm} = 0.707 \times \text{ARM\_LENGTH}$ (diagonal distance from center to motor).

### 5.3 Battery Compensation

> **Source:** [stabilizer.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/stabilizer.c#L208-L219)

After mixing, the motor thrusts are compensated for battery voltage sag:

```c
static void batteryCompensation(...) {
    float b = 0.01f;  // Low-pass filter coefficient
    static float supplyVoltage = 4.2;
    supplyVoltage = supplyVoltage + b * (pmGetBatteryVoltage() - supplyVoltage);

    for (int motor = 0; motor < 4; motor++) {
        motorThrustBatCompUncapped->list[motor] =
            motorsCompensateBatteryVoltage(motor, thrust, supplyVoltage);
    }
}
```

This ensures that as the battery voltage drops from 4.2V to ~3.0V, the PWM duty cycle is increased proportionally to maintain consistent thrust.

### 5.4 Capping & Final PWM Output

> **Source:** [power_distribution_quadrotor.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/power_distribution_quadrotor.c#L160-L190)

```c
bool powerDistributionCap(const motors_thrust_uncapped_t* uncapped, motors_thrust_pwm_t* motorPwm) {
    // Find the highest motor thrust
    int32_t highestThrustFound = 0;
    for (int i = 0; i < 4; i++) {
        if (uncapped->list[i] > highestThrustFound)
            highestThrustFound = uncapped->list[i];
    }

    // If any motor exceeds UINT16_MAX, reduce ALL motors equally
    int32_t reduction = 0;
    if (highestThrustFound > UINT16_MAX) {
        reduction = highestThrustFound - UINT16_MAX;
        isCapped = true;
    }

    for (int i = 0; i < 4; i++) {
        int32_t capped = uncapped->list[i] - reduction;
        motorPwm->list[i] = max(capped, idleThrust);  // Ensure minimum idle thrust
    }
}
```

> [!TIP]
> The capping strategy **preserves stability over thrust**: when one motor saturates, all motors are reduced equally, maintaining the roll/pitch/yaw torque ratios even if total thrust drops. This is a critical safety design.

The final PWM values (0–65535) are sent to the hardware:
```c
motorsSetRatio(MOTOR_M1, motorPwm->motors.m1);
motorsSetRatio(MOTOR_M2, motorPwm->motors.m2);
motorsSetRatio(MOTOR_M3, motorPwm->motors.m3);
motorsSetRatio(MOTOR_M4, motorPwm->motors.m4);
```

---

## 6. Complete Signal Flow Summary

```mermaid
graph TD
    subgraph "Hardware Layer"
        HW_IMU["BMI088 IMU\n(I2C/SPI, 1kHz DRDY)"]
        HW_BARO["BMP388/390\n(I2C, 50Hz)"]
        HW_FLOW["Opt. Flow Deck"]
        HW_UWB["Loco/LH Deck"]
        HW_MOT["4× Brushless Motors"]
    end

    subgraph "sensorsTask (Pri 4, 1 kHz)"
        S1["Read raw gyro+accel"]
        S2["Bias subtract + scale"]
        S3["Align to airframe"]
        S4["2nd-order LPF"]
        S5["xQueueOverwrite"]
        S6["estimatorEnqueue"]
        S7["xSemaphoreGive(dataReady)"]
    end

    subgraph "stabilizerTask (Pri 5, 1 kHz)"
        T1["sensorsWaitDataReady()"]
        T2["sensorsAcquire()"]
        T3["stateEstimator() — 1kHz"]
        T4["commanderGetSetpoint()"]
        T5["supervisorUpdate() — 25Hz"]
        T6["controller() — 500Hz att / 100Hz pos"]
        T7["powerDistribution() — mixer"]
        T8["batteryCompensation()"]
        T9["powerDistributionCap()"]
        T10["motorsSetRatio(M1..M4)"]
    end

    subgraph "Estimator (EKF / Complementary)"
        E1["Dequeue measurements"]
        E2["Predict (gyro integration)"]
        E3["Update (accel, baro, flow, UWB)"]
        E4["Output: state_t\n(pos, vel, att, quat)"]
    end

    HW_IMU -->|EXTI IRQ| S1
    HW_BARO -->|I2C poll| S1
    S1 --> S2 --> S3 --> S4
    S4 --> S5
    S4 --> S6
    S5 --> S7

    S6 --> E1
    HW_FLOW -->|measurementsQueue| E1
    HW_UWB -->|measurementsQueue| E1
    E1 --> E2 --> E3 --> E4

    S7 -->|dataReady sem| T1
    T1 --> T2
    T2 --> T3
    E4 -.->|state_t| T3
    T3 --> T4 --> T5 --> T6 --> T7 --> T8 --> T9 --> T10
    T10 --> HW_MOT
```

## 7. Available Controller Types

The firmware supports **5 built-in controllers** selectable at runtime via the `stabilizer.controller` parameter:

| ID | Controller | Use Case |
|---|---|---|
| 1 | **PID** (default) | Standard cascaded PID. Simple, well-tuned for manual flight |
| 2 | **Mellinger** | Geometric tracking controller (force/torque). Aggressive acrobatics |
| 3 | **INDI** | Incremental Nonlinear Dynamic Inversion. Robust to model uncertainty |
| 4 | **Brescianini** | Nonlinear attitude controller |
| 5 | **Lee** | SE(3) geometric controller. Precise trajectory tracking |

All controllers share the same interface ([controller.c](file:///d:/OneDrive_MSFT/Work/Drone_Research/research_docs/crazyflie-firmware/src/modules/src/controller/controller.c)):
```c
void controller(control_t *control, const setpoint_t *setpoint,
                const sensorData_t *sensors, const state_t *state,
                const stabilizerStep_t stabilizerStep) {
    controllerFunctions[currentController].update(control, setpoint, sensors, state, stabilizerStep);
}
```

This **function pointer table pattern** makes it trivial to swap controllers at runtime without recompiling.
