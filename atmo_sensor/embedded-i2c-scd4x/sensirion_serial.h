#ifndef SENSIRION_SERIAL_H
#define SENSIRION_SERIAL_H

#include "sensirion_config.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ARDUINO_R4_RHPORT 0

void serial_begin(unsigned long baud);
size_t serial_write_str(const char* str);
size_t serial_write(const uint8_t* buf, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  // SENSIRION_SERIAL_H