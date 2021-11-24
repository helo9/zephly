#include <zephyr.h>
#include "irq.h"
#include "kernel.h"
#include <device.h>
#include <stdio.h>
#include <devicetree.h>
#include <drivers/pwm.h>
#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <irq.h>


#define USART2_DEVICE DT_NODELABEL(usart2)

const struct device* usart2 = DEVICE_DT_GET(USART2_DEVICE);

char buffer[256] = {};
volatile uint8_t buffer_idx = 0;
uint32_t time, maxtime = 0;

static void uart_cb_handler(const struct device *dev, void *p)
{
    time = k_cycle_get_32();
	uart_irq_update(dev);

    if (uart_irq_rx_ready(dev)) {
        
        uint8_t c;

        uart_fifo_read(dev, &c, 1);

        if (buffer_idx == 0 && c != 0xA6) {
            return;
        }

        buffer[buffer_idx++] = c;
    }
}

int init_uart() {
    if (!device_is_ready(usart2)) {
        return -EIO;
    }

    uart_irq_rx_disable(usart2);
	uart_irq_tx_disable(usart2);
    uart_irq_callback_user_data_set(usart2, uart_cb_handler, NULL);
    uart_irq_rx_enable(usart2);

    return 0;
}

void main() {


    if (init_uart() < 0){
        printk("USART is not ready!");
        return;
    }

    while (true) {
        uart_irq_rx_disable(usart2);
        printk("recvd: %d, took: %d\n", buffer_idx, maxtime);
        buffer_idx = 0;
        for (int i = 0; i<159; i++) {
            if (buffer[i] == 0xA6 && i > 0) {
                printk("\n");
            } 
            printk("0x%x "  , buffer[i]);
        }
        printk("\n");

        

        uart_irq_rx_enable(usart2);
        k_sleep(K_MSEC(11));
    }
}
