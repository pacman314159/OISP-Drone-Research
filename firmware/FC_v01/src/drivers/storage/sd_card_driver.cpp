#include "drivers/storage/sd_card_driver.h"
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <cctype>

SDCardDriver::SDCardDriver(HAL_SPI& spi_bus, int cs_pin, int cd_pin)
  : _spi(spi_bus), _cs_pin(cs_pin), _cd_pin(cd_pin){}

SDCardDriver::~SDCardDriver(){
  close();
}

bool SDCardDriver::init(){
  if(_mounted) return true;
  if(!_spi.is_card_inserted(_cd_pin)) return false;

  _mounted = _spi.mount_fatfs(_cs_pin, _cd_pin, SD_MOUNT_POINT, SD_SPI_FREQ_HZ);
  return _mounted;
}

bool SDCardDriver::is_card_inserted(){
  return _spi.is_card_inserted(_cd_pin);
}

uint16_t SDCardDriver::find_highest_log_id(const char* dir_path){
  DIR* dir = opendir(dir_path);
  if(dir == nullptr) return 0;

  uint16_t max_id = 0;
  struct dirent* entry = nullptr;

  while((entry = readdir(dir)) != nullptr){
    if(strlen(entry->d_name) == 12){
      if(strncasecmp(entry->d_name, "LOG_", 4) == 0 &&
         strncasecmp(&entry->d_name[8], ".BIN", 4) == 0){
        bool valid_digits = true;
        for(uint8_t i = 4; i <= 7; ++i){
          if(!isdigit(static_cast<unsigned char>(entry->d_name[i]))){
            valid_digits = false;
            break;
          }
        }

        if(valid_digits){
          char id_str[5] = {entry->d_name[4], entry->d_name[5], entry->d_name[6], entry->d_name[7], '\0'};
          uint16_t id_val = static_cast<uint16_t>(atoi(id_str));
          if(id_val > max_id) max_id = id_val;
        }
      }
    }
  }

  closedir(dir);
  return max_id;
}

bool SDCardDriver::open_next_log_file(char* out_filename, size_t max_len, uint16_t& out_session_id){
  if(!_mounted && !init()) return false;

  uint16_t max_id = find_highest_log_id(SD_MOUNT_POINT);
  out_session_id = max_id + 1;

  snprintf(out_filename, max_len, "%s/LOG_%04u.BIN", SD_MOUNT_POINT, out_session_id);
  _file = fopen(out_filename, "wb");
  _bytes_written = 0;

  return (_file != nullptr);
}

bool SDCardDriver::write_header(const LogFileHeader& header){
  if(_file == nullptr) return false;
  size_t written = fwrite(&header, 1, sizeof(LogFileHeader), _file);
  _bytes_written += written;
  fflush(_file);
  return (written == sizeof(LogFileHeader));
}

bool SDCardDriver::write_sector(const uint8_t* sector_data, size_t count){
  if(_file == nullptr || sector_data == nullptr) return false;
  size_t written = fwrite(sector_data, 1, count, _file);
  _bytes_written += written;
  return (written == count);
}

void SDCardDriver::flush(){
  if(_file != nullptr){
    fflush(_file);
    fsync(fileno(_file));
  }
}

void SDCardDriver::close(){
  if(_file != nullptr){
    fflush(_file);
    fsync(fileno(_file));
    fclose(_file);
    _file = nullptr;
  }
}

bool SDCardDriver::is_open() const {
  return (_file != nullptr);
}

uint32_t SDCardDriver::get_bytes_written() const {
  return _bytes_written;
}

void SDCardDriver::reset_file_pointer_to_data_start(){
  if(_file != nullptr){
    fseek(_file, sizeof(LogFileHeader), SEEK_SET);
    _bytes_written = sizeof(LogFileHeader);
  }
}
