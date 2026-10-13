#pragma once
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include "config.h"
#include "platforms/hal/hal_spi.h"

struct __attribute__((packed)) LogFileHeader {
  char     magic[4];
  uint16_t session_id;
  char     board_name[24];
  uint32_t boot_timestamp_us;
  uint16_t registered_packets_mask;
  uint8_t  limit_policy;
  uint32_t max_file_size_bytes;
  uint32_t circular_head_sector;
  char     reserved[467];
};
static_assert(sizeof(LogFileHeader) == 512, "LogFileHeader must be exactly 512 bytes");

class SDCardDriver {
public:
  SDCardDriver(HAL_SPI& spi_bus, int cs_pin, int cd_pin);
  ~SDCardDriver();

  bool init();
  bool is_card_inserted();
  bool open_next_log_file(char* out_filename, size_t max_len, uint16_t& out_session_id);
  bool write_header(const LogFileHeader& header);
  bool write_sector(const uint8_t* sector_data, size_t count = 512);
  void flush();
  void close();
  bool is_open() const;
  uint32_t get_bytes_written() const;
  void reset_file_pointer_to_data_start();

private:
  uint16_t find_highest_log_id(const char* dir_path);

  HAL_SPI& _spi;
  int _cs_pin;
  int _cd_pin;
  FILE* _file{nullptr};
  uint32_t _bytes_written{0};
  bool _mounted{false};
};
