#include "middleware/critical_section.h"

#if defined(ESP_PLATFORM) || defined(ESP32)
  portMUX_TYPE sys_spinlock = portMUX_INITIALIZER_UNLOCKED;
#endif
