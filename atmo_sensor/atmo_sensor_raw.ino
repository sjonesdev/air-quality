#include "Arduino.h"
#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

// extern "C"
// {
//   void sensirion_i2c_hal_init();
//   void scd4x_wake_up();
//   void scd4x_stop_periodic_measurement();
//   void scd4x_reinit();
//   int16_t scd4x_get_serial_number(uint16_t *, uint16_t *, uint16_t *);
//   int16_t scd4x_start_periodic_measurement();
//   void sensirion_i2c_hal_sleep_usec(uint32_t);
//   int16_t scd4x_get_data_ready_flag(bool *);
//   int16_t scd4x_read_measurement(uint16_t *, int32_t *, int32_t *);
// }

int16_t error = 0;

void setup()
{
  Serial.begin(115200);

  sensirion_i2c_hal_init();

  // Clean up potential SCD40 states
  scd4x_wake_up();
  scd4x_stop_periodic_measurement();
  scd4x_reinit();

  uint16_t serial_0;
  uint16_t serial_1;
  uint16_t serial_2;
  error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
  if (error)
  {
    Serial.print("Error executing scd4x_get_serial_number(): ");
    Serial.println(error);
  }
  else
  {
    // Serial.print("serial: 0x%04x%04x%04x\n", serial_0, serial_1, serial_2);
    Serial.print("serial:\n\t");
    Serial.println(serial_0);
    Serial.print("\t");
    Serial.println(serial_1);
    Serial.print("\t");
    Serial.println(serial_2);
  }

  // Start Measurement

  error = scd4x_start_periodic_measurement();
  if (error)
  {
    Serial.print("Error executing scd4x_start_periodic_measurement(): ");
    Serial.println(error);
  }
}

void loop()
{
  // Read Measurement
  sensirion_i2c_hal_sleep_usec(100000);
  bool data_ready_flag = false;
  error = scd4x_get_data_ready_flag(&data_ready_flag);
  if (error)
  {
    Serial.print("Error executing scd4x_get_data_ready_flag(): ");
    Serial.println(error);
    return;
  }
  if (!data_ready_flag)
  {
    return;
  }

  uint16_t co2;
  int32_t temperature;
  int32_t humidity;
  error = scd4x_read_measurement(&co2, &temperature, &humidity);
  if (error)
  {
    Serial.print("Error executing scd4x_read_measurement(): ");
    Serial.println(error);
  }
  else if (co2 == 0)
  {
    Serial.print("Invalid sample detected, skipping.\n");
  }
  else
  {
    Serial.print("CO2: ");
    Serial.println(co2);
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" m°C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println("mRH");
  }
}