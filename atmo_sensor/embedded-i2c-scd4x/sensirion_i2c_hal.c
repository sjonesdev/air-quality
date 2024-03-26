/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

// https://github.com/renesas/ra-fsp-examples/blob/2595a1519a991cc08ad1e33b03e0b0c242c482ce/example_projects/ek_ra4m1/iic_slave/iic_slave_ek_ra4m1_ep/readme.txt#L58C5-L66C29
// RA4M1_EK
// --------
// Channel 0 has been used by IIC slave and channel 1 been used by IIC master.
// 1) Slave IIC pins
//     IIC0 P401  ----> SDA
//     IIC0 P400  ----> SCL
// 2) Master IIC pins
//     IIC1 P206  ----> SDA
//     IIC1 P100  ----> SCL

// const uint16_t P400[] = {
// PIN_PWM|CHANNEL_6|PWM_CHANNEL_A|GPT_ODD_CFG,
// PIN_SCL|CHANNEL_0,
// PIN_INTERRUPT|CHANNEL_0,
// SCI_CHANNEL|PIN_SCK|CHANNEL_0|SCI_EVEN_CFG,
// SCI_CHANNEL|PIN_SCK|CHANNEL_1|SCI_ODD_CFG|LAST_ITEM_GUARD
// };
// const uint16_t P401[] = {
// PIN_PWM|CHANNEL_6|PWM_CHANNEL_B|GPT_ODD_CFG,
// PIN_SDA|CHANNEL_0,
// PIN_INTERRUPT|CHANNEL_5,
// SCI_CHANNEL|PIN_CTS_RTS_SS|CHANNEL_0|SCI_EVEN_CFG,
// SCI_CHANNEL|PIN_TX_MOSI_SDA|CHANNEL_1|SCI_ODD_CFG|LAST_ITEM_GUARD
// };

// const uint16_t P100[] = {
// PIN_ANALOG|CHANNEL_22,
// PIN_PWM|CHANNEL_5|PWM_CHANNEL_B|GPT_ODD_CFG,
// PIN_SCL|CHANNEL_1,
// PIN_INTERRUPT|CHANNEL_2,
// SCI_CHANNEL|PIN_RX_MISO_SCL|CHANNEL_0|SCI_EVEN_CFG,
// SCI_CHANNEL|PIN_SCK|CHANNEL_1|SCI_ODD_CFG,
// PIN_MISO|CHANNEL_0|LAST_ITEM_GUARD
// };
// const uint16_t P206[] = {
// PIN_SDA|CHANNEL_1,
// PIN_INTERRUPT|CHANNEL_0,
// SCI_CHANNEL|PIN_RX_MISO_SCL|CHANNEL_0|SCI_EVEN_CFG|LAST_ITEM_GUARD
// };

#include "sensirion_i2c_hal.h"
#include "sensirion_common.h"
#include "sensirion_config.h"

#include "r_iic_master.h"
#include <assert.h>

#define END_TX_OK 0
#define END_TX_DATA_TOO_LONG 1
#define END_TX_NACK_ON_ADD 2
#define END_TX_NACK_ON_DATA 3
#define END_TX_ERR_FSP 4
#define END_TX_TIMEOUT 5
#define END_TX_NOT_INIT 6
#define TIMEOUT_MS 1000

// why
#define CHANNEL_POS (6)
#define CHANNEL_MASK (0x7C0)
#define GET_CHANNEL(x) ((x & CHANNEL_MASK) >> CHANNEL_POS)
// endwhy

typedef enum {
    I2C_STATUS_UNSET,
    I2C_STATUS_RX_COMPLETED,
    I2C_STATUS_TX_COMPLETED,
    I2C_STATUS_TRANSACTION_ABORTED,
    I2C_STATUS_RX_REQUEST,
    I2C_STATUS_TX_REQUEST,
    I2C_STATUS_GENERAL_CALL
} i2c_status_t;
i2c_status_t i2c_status = I2C_STATUS_UNSET;

bool initd = false;

#define RA4M1_I2C_CHANNEL_SLAVE 0
#define RA4M1_I2C_CHANNEL_MASTER 1

iic_master_extended_cfg_t i2c_extend;
iic_master_instance_ctrl_t i2c_ctrl;
i2c_master_cfg_t i2c_cfg = {
    // .channel       = RA4M1_I2C_CHANNEL_MASTER,
    // .rate = I2C_MASTER_RATE_FAST,
    // .slave         = I2C_SLAVE_EEPROM,
    // .addr_mode     = I2C_MASTER_ADDR_MODE_7BIT,
    // .p_callback    = i2c_callback,     // Callback
    // .p_context = &i2c_ctrl,
    // .p_transfer_tx = NULL,
    // .p_transfer_rx = NULL,
    // .p_extend      = &g_iic_master_cfg_extend
};

void i2c_callback(i2c_master_callback_args_t* p_args) {
    /* +++++ MASTER I2C not SCI Callback ++++++ */
    i2c_master_cfg_t* cfg = (i2c_master_cfg_t*)p_args->p_context;

    if (cfg->channel != i2c_cfg.channel) {
        return;
    }

    if (p_args->event == I2C_MASTER_EVENT_ABORTED) {
        i2c_status = I2C_STATUS_TRANSACTION_ABORTED;
    } else if (p_args->event == I2C_MASTER_EVENT_RX_COMPLETE) {
        i2c_status = I2C_STATUS_RX_COMPLETED;
    } else if (p_args->event == I2C_MASTER_EVENT_TX_COMPLETE) {
        i2c_status = I2C_STATUS_TX_COMPLETED;
    }
}

/**
 * Select the current i2c bus by index.
 * All following i2c operations will be directed at that bus.
 *
 * THE IMPLEMENTATION IS OPTIONAL ON SINGLE-BUS SETUPS (all sensors on the same
 * bus)
 *
 * @param bus_idx   Bus index to select
 * @returns         0 on success, an error code otherwise
 */
int16_t sensirion_i2c_hal_select_bus(uint8_t bus_idx) {
    /* TODO:IMPLEMENT or leave empty if all sensors are located on one single
     * bus NOTE:this is for arduino so I believe this is the case.
     */
    return NO_ERROR;  // NOT_IMPLEMENTED_ERROR;
}

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_hal_init(void) {
    if (initd) {
        sensirion_i2c_hal_free();
    }
    i2c_cfg.rxi_irq = FSP_INVALID_VECTOR;
    i2c_cfg.txi_irq = FSP_INVALID_VECTOR;
    i2c_cfg.tei_irq = FSP_INVALID_VECTOR;
    i2c_cfg.eri_irq = FSP_INVALID_VECTOR;

    i2c_cfg.p_extend = &i2c_extend;
    i2c_cfg.p_callback = i2c_callback;

    i2c_extend.timeout_mode = IIC_MASTER_TIMEOUT_MODE_SHORT;
    i2c_extend.timeout_scl_low = IIC_MASTER_TIMEOUT_SCL_LOW_DISABLED;

    i2c_cfg.channel = RA4M1_I2C_CHANNEL_MASTER;
    i2c_cfg.rate = I2C_MASTER_RATE_STANDARD;
    i2c_extend.clock_settings.brl_value = 27;
    i2c_extend.clock_settings.brh_value = 26;
    int clock_divisor =
        (R_FSP_SystemClockHzGet(BSP_FEATURE_SCI_CLOCK) / 48000000u) - 1;
    i2c_extend.clock_settings.cks_value = 2 + clock_divisor;
    i2c_cfg.slave = 0x62;  // default address for SCD4x dev boards from Adafruit
    i2c_cfg.addr_mode = I2C_MASTER_ADDR_MODE_7BIT;
    i2c_cfg.p_transfer_tx = NULL;
    i2c_cfg.p_transfer_rx = NULL;

    i2c_cfg.p_context = &i2c_cfg;
    i2c_cfg.ipl = (12);

    // set clock
    fsp_err_t err = R_IIC_MASTER_Open(&i2c_ctrl, &i2c_cfg);

    assert(FSP_SUCCESS == err);

    initd = true;
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
 */
void sensirion_i2c_hal_free(void) {
    if (!initd)
        return;

    initd = false;
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
    /* ??? does this function make sense only for MASTER ???? */

    fsp_err_t err = FSP_ERR_ASSERTION;
    if (initd) {
        err =
            R_IIC_MASTER_SlaveAddressSet(&i2c_ctrl, address, i2c_cfg.addr_mode);
        if (err == FSP_SUCCESS) {
            i2c_status = I2C_STATUS_UNSET;
            err = R_IIC_MASTER_Read(&i2c_ctrl, data, count, false);
        }
        // uint32_t const start = millis();
        while (/*((millis() - start) < TIMEOUT_MS) && */
               i2c_status == I2C_STATUS_UNSET && err == FSP_SUCCESS) {
        }
    }

    if (i2c_status == I2C_STATUS_RX_COMPLETED) {
        return count;
    }

    if (i2c_status == I2C_STATUS_UNSET) {
        R_IIC_MASTER_Abort(&i2c_ctrl);
    }

    return 0; /* ???????? return value ??????? */
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data,
                               uint16_t count) {
    uint8_t rv = END_TX_OK;
    fsp_err_t err = FSP_ERR_ASSERTION;
    if (initd) {
        err =
            R_IIC_MASTER_SlaveAddressSet(&i2c_ctrl, address, i2c_cfg.addr_mode);
        if (err == FSP_SUCCESS) {
            i2c_status = I2C_STATUS_UNSET;
            err = R_IIC_MASTER_Write(&i2c_ctrl, data, count, false);
        }
        // uint32_t const start = millis();
        while (/*((millis() - start) < TIMEOUT_MS) && */
               i2c_status == I2C_STATUS_UNSET && err == FSP_SUCCESS) {
        }

        if (err != FSP_SUCCESS) {
            rv = END_TX_ERR_FSP;
        } else if (i2c_status == I2C_STATUS_UNSET) {
            rv = END_TX_TIMEOUT;
            R_IIC_MASTER_Abort(&i2c_ctrl);
        }
        /* as far as I know is impossible to distinguish between NACK on ADDRESS
          and NACK on DATA */
        else if (i2c_status == I2C_STATUS_TRANSACTION_ABORTED) {
            rv = END_TX_NACK_ON_ADD;
        }
    } else {
        rv = END_TX_NOT_INIT;
    }

    return rv;
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */
void sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
    R_BSP_SoftwareDelay(useconds, BSP_DELAY_UNITS_MICROSECONDS);
}
