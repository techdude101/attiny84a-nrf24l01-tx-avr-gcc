#define F_CPU 8000000UL

#include "main.h"

#include <util/delay.h>
#include <util/delay_basic.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// /*
// ATtiny84A pin mapping (your setup):

// PA6 = MOSI
// PA5 = MISO
// PA4 = SCK
// */

// /*
// nRF24 connections:
// MISO → PA6
// MOSI → PA5
// SCK  → PA4
// CSN  → PA3
// CE   → PA2
// */

#define CE_HIGH()  (PORTA |= (1 << PA2))
#define CE_LOW()   (PORTA &= ~(1 << PA2))
#define CSN_HIGH() (PORTA |= (1 << PA3))
#define CSN_LOW()  (PORTA &= ~(1 << PA3))

#define LED_ON()   (PORTB |= (1 << PB0))
#define LED_OFF()  (PORTB &= ~(1 << PB0))

// /* Global variables */
const uint8_t nrf_address[5] = {0xE1, 0xE1, 0xE1, 0xE1, 0xE1};
// /* End of global variables */

// /* ---------------- SPI INIT ---------------- */

void spi_init(void)
{
	//Puts the USI into 3-Wire mode. Uses DO, DI, and USCK pins for SPI communication.
	USICR	|=	(1 << USIWM0);
	USICR	&= ~(1 << USIWM1);
 
	USICR	&= ~(1 << USICS0);
	USICR	|=	(1 << USICS1);
	USICR	|=	(1 << USICLK);
 
	DDRA	|=	(1 << DDA4);	//Set Pin 9 (USCK) to output the clock signal.
	DDRA	|=	(1 << DDA5);	//Sets Pin 8 (DO) to transmit the Master's data.
	DDRA	&= ~(1 << DDA6);	//Sets Pin 7 (DI) to Receive the Slave's data.	
 
	//Set PortA3 as an output pin for SS functionality. (May not need this later)
	DDRA	|=	(1 << DDA3);	//Sets the pin as an output.
	PORTA	&= ~(1 << PORTA3);	//Defaults the output oft he SS (Slave-Select) pin to be a logical 0.
}
 
uint8_t spi_transfer(uint8_t data)
{
	USIDR = data;
 
	for (int i = 0; i < 8; i++)
	{
		USICR	|=	(1 << USITC);
		USICR	|=	(1 << USITC);
	}

    return USIDR;
}

/* ---------------- nRF24 ---------------- */

#define NRF_W_REGISTER   0x20
#define NRF_R_REGISTER   0x00
#define NRF_FLUSH_TX     0xE1
#define NRF_W_TX_PAYLOAD 0xA0

#define REG_CONFIG       0x00
#define REG_STATUS       0x07

#define STATUS_TX_DS  (1 << 5)
#define STATUS_MAX_RT (1 << 4)

#define REG_RF_CH        0x05
#define REG_RF_SETUP     0x06
#define REG_SETUP_AW     0x03
#define REG_EN_AA        0x01
#define REG_EN_RXADDR    0x02
#define REG_RX_PW_P0     0x11
#define REG_TX_ADDR      0x10
#define REG_RX_ADDR_P0   0x0A
#define REG_CONFIG       0x00
#define REG_SETUP_RETR   0x04

#define NRF_PAYLOAD_SIZE 32

void nrf_write_reg(uint8_t reg, uint8_t val)
{
    CSN_LOW();
    _delay_us(10);
    
    spi_transfer(NRF_W_REGISTER | reg);
    spi_transfer(val);
    
    _delay_us(10);
    CSN_HIGH();
    _delay_us(10);
}

uint8_t nrf_read_reg(uint8_t reg)
{
    CSN_LOW();
    _delay_us(10);
    spi_transfer(NRF_R_REGISTER | reg);
    uint8_t val = spi_transfer(0xFF);
    _delay_us(10);
    CSN_HIGH();
    _delay_us(10);
    return val;
}

uint8_t nrf_get_status(void)
{
    CSN_LOW();
    _delay_us(10);

    uint8_t status = spi_transfer(0xFF);

    _delay_us(10);
    CSN_HIGH();

    return status;
}

void nrf_flush_tx(void)
{
    CSN_LOW();
    spi_transfer(NRF_FLUSH_TX);
    CSN_HIGH();
}

void nrf_clear_interrupts(void)
{
    nrf_write_reg(REG_STATUS, STATUS_TX_DS | STATUS_MAX_RT);
}

void nrf_init(void)
{
    CE_LOW();
    CSN_HIGH();
    _delay_ms(100);

    // 1. RF channel = 115
    nrf_write_reg(REG_RF_CH, 115);

    // 2. Data rate = 1Mbps, 0dBm (matches RF24_1MBPS + PA_MAX-ish)
    // RF_SETUP:
    // bit 5 = RF_DR_LOW
    // bit 3 = RF_DR_HIGH
    // 00 = 1Mbps
    // PA level bits 2:1 -> 11 = max
    // nrf_write_reg(REG_RF_SETUP, (1 << 2)); // PA_HIGH + 1Mbps
    nrf_write_reg(REG_RF_SETUP, (0 << 1) | (0 << 2)); // PA_LOW -18dBm + 1Mbps (default reset value)

    // 3. Address width = 5 bytes
    nrf_write_reg(REG_SETUP_AW, 0x03);

    // 4. Enable auto-ack on pipe 0 (Arduino default behavior)
    nrf_write_reg(REG_EN_AA, 0x01);

    // 5. Enable RX pipe 0
    nrf_write_reg(REG_EN_RXADDR, 0x01);

    // 6. Payload size = 32 bytes (fixed)
    nrf_write_reg(REG_RX_PW_P0, NRF_PAYLOAD_SIZE);

    // 7. Auto retransmit (optional but matches RF24 defaults)
    nrf_write_reg(REG_SETUP_RETR, 0x3F);

    // 8. Set TX & RX address (must match Arduino pipe)
    CSN_LOW();
    _delay_us(10);
    
    spi_transfer(NRF_W_REGISTER | REG_TX_ADDR);
    for (uint8_t i = 0; i < 5; i++) spi_transfer(nrf_address[i]);
    _delay_us(5);
    
    CSN_HIGH();

    CSN_LOW();
    _delay_us(10);
    
    spi_transfer(NRF_W_REGISTER | REG_RX_ADDR_P0);
    for (uint8_t i = 0; i < 5; i++) spi_transfer(nrf_address[i]);
    _delay_us(5);
    
    CSN_HIGH();

    // 9. CONFIG register:
    // PWR_UP = 1
    // EN_CRC = 1
    // CRCO = 0 (8-bit CRC)  <-- matches Arduino
    // PRIM_RX = 0 (PTX mode since ATtiny is sending)
    nrf_write_reg(REG_CONFIG, 0x0A);

    _delay_ms(10);
}


void nrf_send_byte(uint8_t data)
{
    // Flush TX FIFO
    nrf_flush_tx();

    // Clear interrupts
    nrf_clear_interrupts();
    CE_LOW();

    CSN_LOW();
    _delay_us(10);
    
    spi_transfer(NRF_W_TX_PAYLOAD); // W_TX_PAYLOAD
    // spi_transfer(0x56); // test byte 65 = A
    // 0x65
    // 0b01100101
    // reversed
    // 0b10100110
    spi_transfer(data);
    
    _delay_us(10);
    CSN_HIGH();

    CE_HIGH();
    _delay_us(15);
    CE_LOW();

    // Wait for TX done (optional but useful)
    while (!(nrf_get_status() & (STATUS_TX_DS | STATUS_MAX_RT)));

    nrf_clear_interrupts();
}

void nrf_send_data_packet(uint8_t *data, uint8_t length)
{
    // Ensure length is valid (max 32 bytes)
    if (length > 32) length = 32;

    CE_LOW();

    // Flush TX FIFO (only if needed, not every time ideally)
    nrf_flush_tx();

    // Clear interrupts
    nrf_clear_interrupts();

    // Write payload
    CSN_LOW();
    _delay_us(5);

    spi_transfer(NRF_W_TX_PAYLOAD);
    for (uint8_t i = 0; i < length; i++)
    {
        spi_transfer(data[i]);
    }

    CSN_HIGH();

    // Pulse CE to start transmission
    CE_HIGH();
    _delay_us(15);   // >10us required
    CE_LOW();

    // Wait for transmission complete
    while (!(nrf_get_status() & (STATUS_TX_DS | STATUS_MAX_RT)));

    // Handle failure (MAX_RT)
    if (nrf_get_status() & STATUS_MAX_RT)
    {
        nrf_flush_tx(); // clear failed packet
    }

    // Clear interrupts
    nrf_clear_interrupts();
}

int main(void)
{
    DDRB |= (1 << PB0); // LED
   
    spi_init();

    // Flash to show we're starting up
    LED_ON();
    _delay_ms(500);
    LED_OFF();
    _delay_ms(500);

    nrf_init();

    uint8_t status = nrf_read_reg(REG_STATUS);

    if (status == 0xFF)
    {
        LED_ON();   // error
        while(1);
    }

    if (status == 0x00) // Flash to show status is 0x00
    {
        while (1)
        {
            LED_ON();   // error
            _delay_ms(100);
            LED_OFF();
            _delay_ms(100);
        }
    }


    while (1)
    {   
        uint8_t payload[NRF_PAYLOAD_SIZE] = {0};

        snprintf((char*)payload, NRF_PAYLOAD_SIZE, "Yay! It works!! :)");

        LED_ON();
        nrf_send_data_packet(payload, NRF_PAYLOAD_SIZE);
        _delay_ms(1000);
        LED_OFF();
        _delay_ms(1000);
    }
}