#include <scd4x_i2c.h>
#include <sensirion_common.h>
#include <sensirion_config.h>
#include <sensirion_i2c.h>
#include <sensirion_i2c_hal.h>

void sendI2C(char *bytes)
{
    // high r/w bit
    // send start
    // send address
    // send data
    // send stop
    // look for ack?
}

void readI2C(char *dest)
{
    // low r/w bit
}

int main(void)
{
}