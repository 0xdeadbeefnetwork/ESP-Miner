#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/uart.h"

#include "esp_log.h"
#include "soc/uart_struct.h"

#include "bm1397.h"
#include "bm1368.h"
#include "serial.h"
#include "utils.h"

#define ECHO_TEST_TXD (17)
#define ECHO_TEST_RXD (18)
#define BUF_SIZE (1024)

static const char *TAG = "serial";

void SERIAL_init(void)
{
    ESP_LOGI(TAG, "Initializing serial");
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    if (uart_param_config(UART_NUM_1, &uart_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure UART parameters");
        return;
    }

    if (uart_set_pin(UART_NUM_1, ECHO_TEST_TXD, ECHO_TEST_RXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART pins");
        return;
    }

    if (uart_driver_install(UART_NUM_1, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, 0) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install UART driver");
        return;
    }
}

void SERIAL_set_baud(int baud)
{
    ESP_LOGI(TAG, "Changing UART baud to %i", baud);
    if (uart_set_baudrate(UART_NUM_1, baud) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set UART baud rate");
    }
}

int SERIAL_send(uint8_t *data, int len, bool debug)
{
    if (debug) {
        printf("tx: ");
        prettyHex((unsigned char *)data, len);
        printf("\n");
    }

    int bytes_sent = uart_write_bytes(UART_NUM_1, (const char *)data, len);
    if (bytes_sent < 0) {
        ESP_LOGE(TAG, "Failed to send data over UART");
    }

    return bytes_sent;
}

int16_t SERIAL_rx(uint8_t *buf, uint16_t size, uint16_t timeout_ms)
{
    int16_t bytes_read = uart_read_bytes(UART_NUM_1, buf, size, timeout_ms / portTICK_PERIOD_MS);
    if (bytes_read < 0) {
        ESP_LOGE(TAG, "Failed to read data from UART");
        return -1;
    }

    #if BM1937_SERIALRX_DEBUG || BM1366_SERIALRX_DEBUG || BM1368_SERIALRX_DEBUG
    size_t buff_len = 0;
    if (bytes_read > 0) {
        uart_get_buffered_data_len(UART_NUM_1, &buff_len);
        printf("rx: ");
        prettyHex((unsigned char*) buf, bytes_read);
        printf(" [%d]\n", buff_len);
    }
    #endif

    return bytes_read;
}

void SERIAL_debug_rx(void)
{
    int ret;
    uint8_t buf[100];

    ret = SERIAL_rx(buf, sizeof(buf), 20);
    if (ret < 0) {
        ESP_LOGE(TAG, "Unable to read data");
        return;
    }

    // Utilize the received data as needed here...
}

void SERIAL_clear_buffer(void)
{
    ESP_LOGI(TAG, "Flushing UART buffer");
    if (uart_flush(UART_NUM_1) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to flush UART buffer");
    }
}
