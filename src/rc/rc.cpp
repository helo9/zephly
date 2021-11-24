#include <zephyr.h>
#include <device.h>
#include <irq.h>
#include <drivers/uart.h>
#include <drivers/gpio.h>
#include <string.h>
#include "rc.hpp"
#include "spm_srxl.h"

#define SRXL_DEVICE_ID  0x31
#define SRXL_DEVICE_PRIORITY 20
#define SRXL_DEVICE_INFO    (SRXL_DEVINFO_NO_RF)
#define SRXL_SUPPORTED_BAUD_RATES   0

#define SRXL_UART_HANDLE 1
#define SRXL_BUS_IDX 0

#define SPEKTRUM_SRXL_MIN_LENGTH 5

#define USART2_DEVICE DT_NODELABEL(usart2)
#define GPIO_DEVICE DT_NODELABEL(gpiod)

const struct device* usart2 = DEVICE_DT_GET(USART2_DEVICE);
const struct device* gpio = DEVICE_DT_GET(GPIO_DEVICE);

struct rc_buffer_t {
    uint8_t buffer[SRXL_MAX_BUFFER_SIZE];
    uint8_t cursor;
    bool ready = false;
};

struct rc_buffer_t rc_buffer[2];
uint8_t active_buffer_idx = 0;
volatile bool received = false;

struct rc_buffer_t tx_buffer = {.ready = true};
uint8_t tx_id = 0;

int last_err = 0;
int tx_count = 0;

inline struct rc_buffer_t& get_active_buffer() {
    return (active_buffer_idx == 0 ? rc_buffer[0] : rc_buffer[1]);
}

inline struct rc_buffer_t& get_inactive_buffer() {
    return (active_buffer_idx == 1 ? rc_buffer[0] : rc_buffer[1]);
}

inline void switch_buffer() {
    get_active_buffer().ready = true;
    
    auto& inactive_buffer = get_inactive_buffer();
    inactive_buffer.ready = false;
    inactive_buffer.cursor = 0;
   
    active_buffer_idx = active_buffer_idx == 0 ? 1 : 0;
}

inline void handle_rx(const struct device *dev) {
    auto& active_buffer = get_active_buffer();

    int ret = uart_fifo_read(dev, &(active_buffer.buffer[active_buffer.cursor]), 1);
    active_buffer.cursor++;
    if (ret < 0) {
        last_err = -ret;
    }

    // if first byte is not SPEKTRUM_SRXL_ID begin again
    if (active_buffer.cursor == 1 && active_buffer.buffer[0] != SPEKTRUM_SRXL_ID){
        active_buffer.cursor = 0;
        return;
    }

    // indicate, that we received something
    if(!received) {
        received = true;
    }

    // check whether message is complete
    if (active_buffer.cursor >= SPEKTRUM_SRXL_MIN_LENGTH) {
        uint8_t length = active_buffer.buffer[2];

        // switch over when complete
        if (active_buffer.cursor == length || active_buffer.cursor > SRXL_MAX_BUFFER_SIZE) {
            switch_buffer();
        }

    }
}

inline void handle_tx(const struct device *dev) {

    // reached end of buffer, stop
    if(tx_id >= tx_buffer.cursor-1) {
        tx_buffer.ready = true;
        uart_irq_tx_disable(dev);
        gpio_pin_set(gpio, 11, 0);
        tx_count++;
    }

    int ret = uart_fifo_fill(dev, &tx_buffer.buffer[tx_id], tx_buffer.cursor-tx_id);

    if (ret < 0) {
        last_err = ret;
        return;
    }

    // move write cursor forward
    tx_id += ret;


}

static void rc_uart_cb_handler(const struct device *dev, void *_) {
    uart_irq_update(dev);

    if (uart_irq_rx_ready(dev)) {
        handle_rx(dev);
    }

    if (uart_irq_tx_ready(dev)) {
        handle_tx(dev);
    }
}

int rc_init() {

    if (!device_is_ready(usart2) || !device_is_ready(gpio)) {
        return -EIO;
    }

    gpio_pin_configure(gpio, 11, GPIO_OUTPUT);
	gpio_pin_set(gpio, 11, 0);

    // Init the local SRXL device
    if(!srxlInitDevice(SRXL_DEVICE_ID, SRXL_DEVICE_PRIORITY, SRXL_DEVICE_INFO, 0xaba)) {
        return -ENOTSUP;
    }

    // Init the SRXL bus: The bus index must always be < SRXL_NUM_OF_BUSES -- in this case, it can only be 0
    if(!srxlInitBus(SRXL_BUS_IDX, SRXL_UART_HANDLE, SRXL_SUPPORTED_BAUD_RATES)) {
        return -ENOTSUP;
    }

    // Set and enable ISR for uart2 rx
    uart_irq_rx_disable(usart2);
	uart_irq_tx_disable(usart2);
    uart_irq_callback_user_data_set(usart2, rc_uart_cb_handler, NULL);
    uart_irq_rx_enable(usart2);

    get_active_buffer().cursor = 0;

    return 0;
}

int rc_update(uint32_t dt) {
    int ret = 0;

    // Nothing received, update state machine with timeout time
    if (!received) {
        srxlRun(SRXL_BUS_IDX, (int16_t)dt);
        return 0;
    }

    auto read_buffer = get_inactive_buffer();

    // Disable rx interrupt while parsing buffer
    uart_irq_rx_disable(usart2);

    if (read_buffer.ready) {
        
        if(!srxlParsePacket(SRXL_BUS_IDX, read_buffer.buffer, read_buffer.cursor)) {
            ret = -EINVAL;
        }

        read_buffer.ready = false;
    }

    // Reenable rx interrupts again
    uart_irq_rx_enable(usart2);

    return ret;
}

struct RCInput rc_get() {
    
    uart_irq_rx_disable(usart2);

    struct RCInput input = {
        srxlChData.values[1],
        srxlChData.values[2],
        srxlChData.values[0],
        srxlChData.values[3],
    };

    uart_irq_rx_enable(usart2);

    return input;
}

extern "C" {

void transmit_uart(uint8_t * pBuffer, uint8_t length) {

    if (length > SRXL_MAX_BUFFER_SIZE) {
        return;
    }

    // still sending, don't mix it up
    if (!tx_buffer.ready) {
        return;
    }
    
    memcpy(tx_buffer.buffer, pBuffer, length);
    tx_buffer.cursor = length;
    tx_buffer.ready = false;

    gpio_pin_set(gpio, 11, 1);
    uart_irq_tx_enable(usart2);
}

}
