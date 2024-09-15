#include "Arduino.h"
#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"
#include "r_iic_master.h"

int16_t error = 0;

void setup()
{
  Serial.begin(115200);

  delay(1000);
  Serial.println("Hello there.");
  // serial_write((const uint8_t*)"test1\n", 6);

  sensirion_i2c_hal_init();
  Serial.println("Initialized HAL via driver");


  // Clean up potential SCD40 states
  scd4x_wake_up();
  Serial.println("Woke up sensor");
  scd4x_stop_periodic_measurement();
  Serial.println("Stopped periodic measurement");
  scd4x_reinit();

  Serial.println("Initialized SCD40");

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

  Serial.println("Got Serial No.");

  // Start Measurement

  error = scd4x_start_periodic_measurement();
  if (error)
  {
    Serial.print("Error executing scd4x_start_periodic_measurement(): ");
    Serial.println(error);
  }

  Serial.println("Started Periodic Measurement");
}

void loop()
{
  // Read Measurement
  sensirion_i2c_hal_sleep_usec(100000);
  Serial.println("Slept");
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
    Serial.println("Data not ready");
    return;
  }
  Serial.println("Data ready");

  uint16_t co2;
  int32_t temperature;
  int32_t humidity;
  error = scd4x_read_measurement(&co2, &temperature, &humidity);
  Serial.println("Read measurement");
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