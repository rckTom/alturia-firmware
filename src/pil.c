#include "daq.h"
#include "drivers/uart.h"
#include "logging/log.h"
#include "zephyr.h"
#include "sys/ring_buffer.h"
#include "pyros.h"
#include "servos.h"

LOG_MODULE_REGISTER(PIL, CONFIG_LOG_DEFAULT_LEVEL);

RING_BUF_DECLARE(txbuf, 512);
RING_BUF_DECLARE(rxbuf, 512);

K_CONDVAR_DEFINE(cond_drdy);
K_MUTEX_DEFINE(pil_data_lock);

struct pil_data {
    uint64_t time;
    float pressure, ax, ay, az, gy, gx, gz;
} __packed;

struct pil_response {
    uint8_t pyros;
    float servos[2];
} __packed;

static enum interface_state {
    DETECT_START,
    READ_MSG_LENGTH,
    READ_PIL_DATA,
} state = DETECT_START;

static struct pil_data pil_data_internal;
static const struct device *uart_dev;
static uint32_t to_receive = 0;

static void uart_irq(const struct device *dev, void *user_data)
{
    ARG_UNUSED(user_data);
    while(uart_irq_update(dev) && uart_irq_is_pending(dev)) {
        /* Receive data */
        if (uart_irq_rx_ready(dev)) {
            uint8_t *buffer;
            uint32_t bsize = ring_buf_put_claim(&rxbuf, &buffer, 512);
            int rsize = uart_fifo_read(dev, buffer, bsize);
            ring_buf_put_finish(&rxbuf, rsize);

            //check amount of data in buffer
            uint32_t numreceived = ring_buf_capacity_get(&rxbuf) - ring_buf_space_get(&rxbuf);

            if(numreceived == 512) {
                LOG_ERR("buffer overflow");
                uart_irq_rx_disable(dev);
                return;
            }

            /* detect start byte */
            if (state == DETECT_START && numreceived >= 1) {
                uint8_t start_byte;
                state = READ_MSG_LENGTH;
                k_condvar_signal(&cond_drdy);
                ring_buf_get(&rxbuf, &start_byte, 1);
            }

            /* read message length */
            if (state == READ_MSG_LENGTH && numreceived >= 4) {
                uint32_t to_read = 4;
                uint8_t *rbuf = (void *)&to_receive;
                while (to_read != 0) {
                    uint32_t bread = ring_buf_get(&rxbuf, rbuf, to_read);
                    numreceived -= bread;
                    to_read -= bread;
                    rbuf += bread;
                }
                LOG_INF("to_receive: %d", to_receive);
                state = READ_PIL_DATA;
            }
            
            /* read message */
            if (state == READ_PIL_DATA && numreceived >= to_receive) {
                uint8_t *rbuf = (void *)&pil_data_internal;
                while (to_receive != 0) {
                    uint32_t bread = ring_buf_get(&rxbuf, rbuf, to_receive);
                    rbuf += bread;
                    to_receive -= bread;
                }
                k_condvar_signal(&cond_drdy);
                state = READ_MSG_LENGTH;
            }
        } 
        
        /* Send data */
        if (uart_irq_tx_ready(dev)) {
            uint8_t *buffer;
            uint32_t bsize = ring_buf_get_claim(&txbuf, &buffer, 512);

            if(bsize == 0) {
                uart_irq_tx_disable(dev);
            } else {
                int ssize = uart_fifo_fill(dev, buffer, bsize);
                ring_buf_get_finish(&txbuf, ssize);
            }
        }
    }
}

int send_pil_response(const struct device *dev)
{
    uint32_t send_bytes = 0;
    const uint32_t msg_length = sizeof(struct pil_response);
    struct pil_response resp = {0};
    resp.pyros =  pyros_get_fire_state(0) | (pyros_get_fire_state(1) << 1) | (pyros_get_fire_state(2) << 2) | (pyros_get_fire_state(3) << 3);
    int rc = servo_get_setpoint(0, resp.servos);

    if (rc < 0) {
        LOG_ERR("unable to get servo value");
        resp.servos[0] = 0.0f;
    }    

    rc = servo_get_setpoint(1, resp.servos + 1);
    if (rc < 0) {
        LOG_ERR("unable to get servo value");
        resp.servos[1] = 0.0f;
    }

    send_bytes = ring_buf_put(&txbuf, (uint8_t*)&msg_length, sizeof(msg_length));
    send_bytes += ring_buf_put(&txbuf, (uint8_t*)&resp, sizeof(struct pil_response));

    if (send_bytes != sizeof(msg_length) + sizeof(struct pil_response)) {
        LOG_ERR("can not send all data");
        return -1;
    }

    uart_irq_tx_enable(dev);
    return 0;
}

int wait_for_pil_data(struct pil_data *data, k_timeout_t timeout)
{
    int rc = k_mutex_lock(&pil_data_lock, timeout);
    
    if(rc < 0) {
        LOG_ERR("unable to lock mutex");
        return rc;
    }
    
    rc = k_condvar_wait(&cond_drdy, &pil_data_lock, timeout);
    
    if (rc != 0) {
        LOG_ERR("unable to wait for condvar");
        goto out;
    }
    
    memcpy(data, &pil_data_internal, sizeof(struct pil_data));

    /* send reponse */
    k_mutex_unlock(&pil_data_lock);
    rc = send_pil_response(uart_dev);

    return rc;

out:
    k_mutex_unlock(&pil_data_lock);
    return rc;
}

void configure_pil()
{
    const struct device *dev = device_get_binding("CDC_ACM_0");
    uart_dev = dev;
    int rc = 0;
    if (dev == NULL) {
        LOG_ERR("unable to get cdc acm device");
        return;
    }

    uint32_t dtr = 0;
    while (true) {
        rc = uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        if (rc != 0) {
            LOG_ERR("unable to get line ctrl signal");
        }
        if (dtr) {
            break;
        } else {
            k_sleep(K_MSEC(1000));
        }
    }

    LOG_INF("dtr set");
    uart_irq_callback_set(dev, uart_irq);
    uart_irq_rx_enable(dev);

    /* Wait until pil is started by host */
    k_mutex_lock(&pil_data_lock, K_FOREVER);
    k_condvar_wait(&cond_drdy, &pil_data_lock, K_FOREVER);
    k_mutex_unlock(&pil_data_lock);
    
    LOG_INF("start pil");
    send_pil_response(dev);

    while(true) {
        struct pil_data data;
        if (wait_for_pil_data(&data, K_FOREVER) < 0) {
            return;
        }
        LOG_INF("pil_data: %d", (int)pil_data_internal.pressure);
    }
}
