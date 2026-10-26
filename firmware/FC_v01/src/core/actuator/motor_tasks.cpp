#include "core/actuator/motor_tasks.h"
#include <Arduino.h>
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/critical_section.h"
#include "middleware/sysview_tracing.h"
#include "drivers/motor/motor_driver.h"

// =============================================================================
// LAYER 4 FLIGHT CORE & ALGORITHMS - ACTUATOR TASK RUNNER (250 Hz)
// =============================================================================

void motor_task(void* arg){
  MotorDriver* motor = static_cast<MotorDriver*>(arg);
  const uint32_t task_id = TASK_MOTOR_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_MOTOR_FREQ_HZ);

  TickType_t last_wake_time = xTaskGetTickCount();

  while(true){
    // Anti-windup overrun guard: if behind schedule reset it
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2)) last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    if(motor == nullptr) continue;

    SYSVIEW_START(task_id); //=========================================================================

    // 1. FAST-PATH EMERGENCY NOTIFICATION: First check (received) + Second check (signal verification)
    uint32_t notify_bits = 0;
    if(xTaskNotifyWait(0, ULONG_MAX, &notify_bits, 0) == pdTRUE){
      if((notify_bits & NOTIFY_MOTOR_EMERGENCY_STOP) != 0){
        motor->disarm_all();
        MotorCommand dummy;
        while(xQueueReceive(motor_cmd_queue, &dummy, 0) == pdTRUE);
        SYSVIEW_END(task_id);
        continue;
      }
    }

    // 2. ROUTINE COMMAND QUEUE: Process lifecycle transitions (Option 2)
    MotorCommand cmd;
    while(xQueueReceive(motor_cmd_queue, &cmd, 0) == pdTRUE){
      if(cmd == MOTOR_CMD_ARM)
        motor->arm_all();
      else if(cmd == MOTOR_CMD_DISARM || cmd == MOTOR_CMD_EMERGENCY_STOP)
        motor->disarm_all();
    }

    // 3. ACTUATION SAFETY GATING: If actuator is disarmed, bypass motor control completely
    if(!motor->is_armed()){
      SYSVIEW_END(task_id);
      continue;
    }

    /*
     * FUTURE DEVELOPMENT - INDI CONTROLLER CONSUMPTION & QUAD-X MOTOR MIXING:
     *
     * 1. Safely snapshot the latest INDI controller outputs from Layer 3 IPC:
     *    IndiControllerOutput local_indi;
     *    SYS_ENTER_CRITICAL();
     *    local_indi = indi_output;
     *    SYS_EXIT_CRITICAL();
     *
     * 2. Ground & Disarm Safety Lockout:
     *    If the drone is disarmed or collective thrust is at ground idle (< 1%):
     *    if(!local_indi.is_armed || local_indi.thrust_cmd <= 0.01f){
     *      motor->disarm_all();
     *      SYSVIEW_END(task_id);
     *      continue;
     *    }
     *
     * 3. Quad-X Motor Mixing (Normalized domain [0.0, 1.0]):
     *    float m[MOTOR_COUNT];
     *    m[0] = local_indi.thrust_cmd - local_indi.u_in.x + local_indi.u_in.y + local_indi.u_in.z; // M1: Rear-Right  (CW)
     *    m[1] = local_indi.thrust_cmd - local_indi.u_in.x - local_indi.u_in.y - local_indi.u_in.z; // M2: Front-Right (CCW)
     *    m[2] = local_indi.thrust_cmd + local_indi.u_in.x - local_indi.u_in.y + local_indi.u_in.z; // M3: Front-Left  (CW)
     *    m[3] = local_indi.thrust_cmd + local_indi.u_in.x + local_indi.u_in.y - local_indi.u_in.z; // M4: Rear-Left   (CCW)
     *
     * 4. Attitude-Priority Saturation Capping:
     *    Ensure no single motor exceeds 1.0, pulling down collective throttle if needed
     *    to preserve roll/pitch attitude authority during motor saturation:
     *    float max_m = m[0];
     *    for(uint8_t i = 1; i < MOTOR_COUNT; ++i) if(m[i] > max_m) max_m = m[i];
     *    if(max_m > 1.0f){
     *      float excess = max_m - 1.0f;
     *      for(uint8_t i = 0; i < MOTOR_COUNT; ++i) m[i] -= excess;
     *    }
     *
     * 5. Actuate Motors:
     *    motor->set_all_normalized(m);
     */

    SYSVIEW_END(task_id); //=========================================================================
  }
}

