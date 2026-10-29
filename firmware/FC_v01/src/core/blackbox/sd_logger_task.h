#pragma once

extern volatile bool sd_stop_logging_requested;

void sd_logger_task(void* arg);
