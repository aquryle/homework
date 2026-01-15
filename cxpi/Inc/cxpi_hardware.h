#ifndef __CXPI_HARDWARE_H__
#define __CXPI_HARDWARE_H__

#include "cxpi_user.h"

typedef enum {
    CXPI_TXD_PIN,
    CXPI_RXD_PIN,
    CXPI_MS_PIN,
} cxpi_pin_t;

typedef enum {
    CXPI_PIN_LOW = 0,
    CXPI_PIN_HIGH = 1,
} pin_state_t;


extern int Uart_Transmit(const uint8_t *buffer, uint16_t length, uint32_t timeout);
extern int Uart_Receive(const uint8_t *buffer, uint16_t length, uint32_t timeout);
extern void Gpio_Write(cxpi_pin_t pin, cxpi_pin_t state);
extern void Delay(uint16_t ms);



#endif /* __CXPI_HARDWARE_H__ */