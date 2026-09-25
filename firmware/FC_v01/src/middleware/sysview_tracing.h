#pragma once
#include <cstdint>
#include "config.h"

#if (defined(CONFIG_SYSVIEW_ENABLE) && CONFIG_SYSVIEW_ENABLE) || (defined(CONFIG_SEGGER_SYSVIEW_ENABLE) && CONFIG_SEGGER_SYSVIEW_ENABLE) || (defined(CONFIG_APPTRACE_SV_ENABLE) && CONFIG_APPTRACE_SV_ENABLE)
  #include "SEGGER_SYSVIEW.h"

  void sysview_init_markers();

  inline void SYSVIEW_START(uint32_t task_id){
    SEGGER_SYSVIEW_OnUserStart(task_id + 1);
  }

  inline void SYSVIEW_END(uint32_t task_id){
    SEGGER_SYSVIEW_OnUserStop(task_id + 1);
  }
#else
  inline void sysview_init_markers(){}
  inline void SYSVIEW_START(uint32_t task_id){ (void)task_id; }
  inline void SYSVIEW_END(uint32_t task_id){ (void)task_id; }
#endif
