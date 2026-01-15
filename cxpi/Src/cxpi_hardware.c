#include "cxpi_hardware.h"



int Uart_Transmit(const uint8_t *buffer, uint16_t length, uint32_t timeout)
{
    return 0;
}


int Uart_Receive(const uint8_t *buffer, uint16_t length, uint32_t timeout)
{
    return 0;
}


void Gpio_Write(cxpi_pin_t pin, cxpi_pin_t state)
{
    switch (pin)
    {
    case CXPI_TXD_PIN:
        /* code */
        break;
    case CXPI_RXD_PIN:
        /* code */
        break;
    case CXPI_MS_PIN:
        /* code */
        break;
    default:
        break;
    }
}


void Delay(uint16_t ms)
{

}
