#ifndef UART_H
#define UART_H

#include <mega128a.h> 
#include <stdint.h>

#ifndef RXB8
#define RXB8 1
#endif

#ifndef TXB8
#define TXB8 0
#endif

#ifndef UPE
#define UPE 2
#endif

#ifndef DOR
#define DOR 3
#endif

#ifndef FE
#define FE 4
#endif

#ifndef UDRE
#define UDRE 5
#endif

#ifndef RXC
#define RXC 7
#endif

#define RX_BUFFER_SIZE0 32
#define RX_BUFFER_SIZE1 512

#define FRAMING_ERROR (1<<FE)
#define PARITY_ERROR (1<<UPE)
#define DATA_OVERRUN (1<<DOR)
#define DATA_REGISTER_EMPTY (1<<UDRE)
#define RX_COMPLETE (1<<RXC)

extern uint8_t _FlagPackageTerminate0;         // Use in uart.c, main.c, debug.c
extern uint8_t rxPackage0[];           // Use in uart.c, debug.c

void init_uart(uint8_t channel, uint32_t baud);
uint8_t getchar(void);
void putchar0(uint8_t c);
void putchar1(uint8_t c);
uint16_t read_uart1(uint8_t *readbuf, uint16_t maxread, uint16_t interchar);
uint16_t write_uart1(uint8_t writebuf[], uint16_t len);
void xbee_clearRcvBuffer0(void);

void send_uart(uint8_t port, uint8_t *buffer);

#endif

