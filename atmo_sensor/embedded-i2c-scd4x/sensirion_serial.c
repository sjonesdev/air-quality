#include "sensirion_serial.h"

bool serial_running = false;

bool serial_connected() {
    return (tud_cdc_connected());
}

void serial_begin(unsigned long baud) {

    // int max_index = PINS_COUNT;

    // init_ok = cfg_pins(max_index);

    // if (init_ok) {
    //     UART::g_uarts[channel] = this;

    //     uart_baud.semr_baudrate_bits_b.abcse = 0;
    //     uart_baud.semr_baudrate_bits_b.abcs = 0;
    //     uart_baud.semr_baudrate_bits_b.bgdm = 1;
    //     uart_baud.cks = 0;
    //     uart_baud.brr = 25;
    //     uart_baud.mddr = (uint8_t)256;
    //     uart_baud.semr_baudrate_bits_b.brme = false;

    //     uart_cfg_extend.clock = SCI_UART_CLOCK_INT;
    //     uart_cfg_extend.rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE;
    //     uart_cfg_extend.noise_cancel = SCI_UART_NOISE_CANCELLATION_DISABLE;
    //     uart_cfg_extend.rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX;
    //     uart_cfg_extend.p_baud_setting = &uart_baud;
    //     uart_cfg_extend.flow_control = SCI_UART_FLOW_CONTROL_RTS;
    //     uart_cfg_extend.flow_control_pin = (bsp_io_port_pin_t)UINT16_MAX;
    //     if (rts_pin != -1 && cts_pin != -1) {
    //         uart_cfg_extend.flow_control =
    //             SCI_UART_FLOW_CONTROL_HARDWARE_CTSRTS;
    //     }
    //     uart_cfg_extend.rs485_setting.enable = SCI_UART_RS485_DISABLE;
    //     uart_cfg_extend.rs485_setting.polarity =
    //         SCI_UART_RS485_DE_POLARITY_HIGH;
    //     uart_cfg_extend.rs485_setting.de_control_pin =
    //         (bsp_io_port_pin_t)UINT16_MAX;

    //     uart_cfg.channel = channel;
    //     uart_cfg.p_context = NULL;
    //     uart_cfg.p_extend = &uart_cfg_extend;
    //     uart_cfg.p_transfer_tx = NULL;
    //     uart_cfg.p_transfer_rx = NULL;

    //     switch (config) {
    //         case SERIAL_8N1:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_OFF;
    //             uart_cfg.stop_bits = UART_STOP_BITS_1;
    //             break;
    //         case SERIAL_8N2:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_OFF;
    //             uart_cfg.stop_bits = UART_STOP_BITS_2;
    //             break;
    //         case SERIAL_8E1:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_EVEN;
    //             uart_cfg.stop_bits = UART_STOP_BITS_1;
    //             break;
    //         case SERIAL_8E2:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_EVEN;
    //             uart_cfg.stop_bits = UART_STOP_BITS_2;
    //             break;
    //         case SERIAL_8O1:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_ODD;
    //             uart_cfg.stop_bits = UART_STOP_BITS_1;
    //             break;
    //         case SERIAL_8O2:
    //             uart_cfg.data_bits = UART_DATA_BITS_8;
    //             uart_cfg.parity = UART_PARITY_ODD;
    //             uart_cfg.stop_bits = UART_STOP_BITS_2;
    //             break;
    //     }

    //     uart_cfg.p_callback = UART::WrapperCallback;
    // } else {
    //     return;
    // }

    // init_ok &= setUpUartIrqs(uart_cfg);

    // fsp_err_t err;
    // const bool bit_mod = true;
    // const uint32_t err_rate = 3000;  // means 3%

    // err = R_SCI_UART_BaudCalculate(baudrate, bit_mod, err_rate, &uart_baud);
    // if (uart_baud.mddr == 0) {
    //     err = R_SCI_UART_BaudCalculate(baudrate, false, err_rate,
    //     &uart_baud);
    // }
    // err = R_SCI_UART_Open(&uart_ctrl, &uart_cfg);
    // if (err != FSP_SUCCESS)
    //     while (1)
    //         ;
    // err = R_SCI_UART_BaudSet(&uart_ctrl, (void*)&uart_baud);
    // if (err != FSP_SUCCESS)
    //     while (1)
    //         ;

    // rxBuffer.clear();
    // txBuffer.clear();

    // serial_running = true;
}

size_t serial_write_str(const char* str) {
    return write((const uint8_t*)str, strlen(str));
}

/** Requires TinyUSB initialization, which is done for you on Arduino */
size_t serial_write(const uint8_t* buf, size_t size) {

    if (!serial_connected()) {
        return 0;
    }

    usbd_int_set(false);
    size_t to_send = size, so_far = 0;
    while (to_send) {
        size_t space = tud_cdc_write_available();
        if (!space) {
            usbd_int_set(true);
            tud_cdc_write_flush();
            continue;
        }
        if (space > to_send) {
            space = to_send;
        }
        size_t sent = tud_cdc_write(buf + so_far, space);
        usbd_int_set(true);
        if (sent) {
            so_far += sent;
            to_send -= sent;
            tud_cdc_write_flush();
        } else {
            size = so_far;
            break;
        }
    }
    return size;
}