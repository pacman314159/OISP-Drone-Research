#include <Arduino.h>
#include "core/telemetry/rc_receiver_task.h"
#include "config.h"
#include "middleware/ipc.h"
#include "middleware/sysview_tracing.h"
#include "drivers/rc_rx/fs_ia6b.h"
#include "app/fsm.h"

void rc_receiver_task(void* arg){
  if(arg == nullptr) return;

  FS_iA6B* rx = static_cast<FS_iA6B*>(arg);
  const uint32_t task_id = TASK_RC_RECEIVER_ID;
  const TickType_t period_ticks = pdMS_TO_TICKS(1000 / TASK_RC_RECEIVER_FREQ_HZ);

  RcChannels channels = {};
  TickType_t last_wake_time = xTaskGetTickCount();
  bool last_signal_ok = false;

  Serial.println("[RC] RC Receiver Task started (FlySky FS-iA6B PPM via RMT).");

  while(true){
    TickType_t now = xTaskGetTickCount();
    if((now - last_wake_time) > (period_ticks * 2))
      last_wake_time = now;
    vTaskDelayUntil(&last_wake_time, period_ticks);

    SYSVIEW_START(task_id);

    // Bounded timeout (5 ms << 20 ms task period) prevents scheduler overruns when RC is disconnected
    bool ok = rx->update(channels, 5);
    uint32_t now_us = static_cast<uint32_t>(micros());

    if(ok && !channels.failsafe){
      rc_data.push(channels);
      rc_timestamp.push(now_us);

      fsm_set_event(EVT_RC_SIGNAL_OK);
      fsm_clear_event(EVT_RC_SIGNAL_FAIL);

      if(!last_signal_ok){
        last_signal_ok = true;
        Serial.printf("[RC] Signal LOCKED! Active channels: %u\n", channels.channel_count);
      }
    }else if(channels.failsafe){
      fsm_clear_event(EVT_RC_SIGNAL_OK);
      fsm_set_event(EVT_RC_SIGNAL_FAIL);

      if(last_signal_ok){
        last_signal_ok = false;
        Serial.println("[RC] WARNING: Signal LOST / FAILSAFE asserted!");
      }
    }

    SYSVIEW_END(task_id);
  }
}
