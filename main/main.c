/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void write_package(adc_t data) {
    int val = data.val;
    int msb = val >> 8;
    int lsb = val & 0xFF;

    uart_putc_raw(uart0, data.axis);
    uart_putc_raw(uart0, lsb);
    uart_putc_raw(uart0, msb);
    uart_putc_raw(uart0, -1);
}

void x_task() {

    adc_init();
    adc_gpio_init(26);
    adc_set_round_robin(0b00011);

    adc_t data_x;
    data_x.axis = 0;

    while (true) {

        uint16_t result_x = adc_read();
        data_x.val = (result_x - 2047) * 255 / 2048;
        xQueueSend(xQueueAdc, &data_x, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void y_task() {

    adc_init();
    adc_gpio_init(27);
    adc_set_round_robin(0b00011);

    adc_t data_y;
    data_y.axis = 1;

    while (true) {

        uint16_t result_y = adc_read();
        data_y.val = - (result_y - 2047) * 255 / 2047;
        xQueueSend(xQueueAdc, &data_y, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void uart_task(void *p) {
    adc_t data_to_send;

    while (1) {
        if (xQueueReceive(xQueueAdc, &data_to_send, pdMS_TO_TICKS(10))) {
            if (data_to_send.val < -30 || data_to_send.val > 30) {
                // printf("%d: %d\n", data.axis, data.val);
                write_package(data_to_send);
            }
        }
    }
}

int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(uart_task, "uart_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "x_task", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
