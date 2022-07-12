#include "uart.h"

void uart_tx_start(void * data, size_t size); // Start dma transmission
void uart_tx_wait(void); // Wait for transmission to finish
void uart_rx_start(void * buffer, size_t size); // Start dma reception
void uart_rx_wait(void); // Wait for reception to finish
void uart_dma_init(void); // Setup dma for UART
