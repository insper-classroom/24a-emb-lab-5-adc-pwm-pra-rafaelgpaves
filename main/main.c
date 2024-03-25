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

void x_task() {

    adc_init();
    adc_gpio_init(26);
    // adc_select_input(0);
    adc_set_round_robin(0b00011);

    adc_t data_x;
    data_x.axis = 0;

    while (true) {

        uint16_t result_x = adc_read();
        data_x.val = (result_x - 2047) * 255 / 2047;
        xQueueSend(xQueueAdc, &data_x, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void y_task() {

    adc_init();
    adc_gpio_init(27);
    // adc_select_input(1);
    adc_set_round_robin(0b00011);

    adc_t data_y;
    data_y.axis = 1;

    while (true) {

        uint16_t result_y = adc_read();
        data_y.val = (result_y - 2047) * 255 / 2047;
        xQueueSend(xQueueAdc, &data_y, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void uart_task(void *p) {
    adc_t data;

    while (1) {
        if (xQueueReceive(xQueueAdc, &data, pdMS_TO_TICKS(10))) {
            if (data.val < -30 || data.val > 30) {
                printf("%d: %d\n", data.axis, data.val);
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
