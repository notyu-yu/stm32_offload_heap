#include "uart_dma.h"

// DMA status indicators
static int receiving=0;
static int transmitting=0;

// Setup uart transmission
static void uart_tx_setup(void) {
	// Clear control register
	DMA2_Stream7->CR = 0;
	// Wait for DMA to disable
	while(DMA2_Stream7->CR & (1<<0));
	// Select channel 4 for usart1_tx
	DMA2_Stream7->CR |= (0x4<<25);
	// Enable tx complete interrupt
	DMA2_Stream7->CR |= DMA_SxCR_TCIE;
	// Enable memory increment mode
	DMA2_Stream7->CR |= DMA_SxCR_MINC;
	// Priority level high
	DMA2_Stream7->CR |= (0x2<<16);
	// DIR bit set to 01: source SxM0AR, dest SxPAR
	DMA2_Stream7->CR |= (0x1 << 6);
}

// Setup uart reception
static void uart_rx_setup(void) {
	// Enable receive DMA
	USART1->CR3 |= USART_CR3_DMAR;
	// Clear control register
	DMA2_Stream2->CR = 0;
	// Wait for DMA to disable
	while(DMA2_Stream2->CR & (1<<0));
	// Select channel 4 for usart1_rx
	DMA2_Stream2->CR |= (0x4<<25);
	// Enable rx complete interrupt
	DMA2_Stream2->CR |= DMA_SxCR_TCIE;
	// Enable memory increment mode
	DMA2_Stream2->CR |= DMA_SxCR_MINC;
	// Priority level high
	DMA2_Stream2->CR |= (0x2<<16);
	// DIR bit set to 00: source SxPAR, dest SxM0AR
	DMA2_Stream2->CR &= ~(0xC << 6);
}

// Start uart transmission of size bytes of data
void uart_tx_start(void * data, size_t size) {
	uart_tx_wait();
	uart_tx_setup();

	// Source memory address
	DMA2_Stream7->M0AR = (uint32_t)data;
	// Destination memory address
	DMA2_Stream7->PAR = (uint32_t)&(USART1->DR);
	// Transfer size
	DMA2_Stream7->NDTR = size;

	// Enable transfer Complete interrupt
	NVIC_SetPriority(DMA2_Stream7_IRQn, 4);
	NVIC_EnableIRQ(DMA2_Stream7_IRQn);

	// Enable DMA
	DMA2_Stream7->CR |= DMA_SxCR_EN;

	transmitting=1;
}

// Wait for uart transmission to finish
void uart_tx_wait(void) {
	while (transmitting);
}

// Start uart reception of size bytes of data into buffer
void uart_rx_start(void * buffer, size_t size) {
	uart_rx_wait();
	uart_rx_setup();

	// Source memory address
	DMA2_Stream2->PAR = (uint32_t)&(USART1->DR);
	// Destination memory address
	DMA2_Stream2->M0AR = (uint32_t)buffer;
	// Transfer size
	DMA2_Stream2->NDTR = size;

	// Enable transfer Complete interrupt
	NVIC_SetPriority(DMA2_Stream2_IRQn, 5);
	NVIC_EnableIRQ(DMA2_Stream2_IRQn);

	// Enable DMA
	DMA2_Stream2->CR |= DMA_SxCR_EN;

	receiving=1;
}

// Wait for uart reception to finish
void uart_rx_wait(void) {
	while (receiving);
}

// UART reception finish interrupt
void DMA2_Stream2_IRQHandler(void)
{
    // clear stream receive complete interrupt - bit11 for stream 5
    if (DMA2->LISR & DMA_LISR_TCIF2) {
        // clear interrupt
        DMA2->LIFCR |= DMA_LISR_TCIF2;
		receiving = 0;
		// Disable receive DMA
		USART1->CR3 &= ~USART_CR3_DMAR;
    }
}

// UART transmission finish interrupt
void DMA2_Stream7_IRQHandler(void)
{
    // clear stream transfer complete interrupt - bit21 for stream 6
    if (DMA2->HISR & DMA_HISR_TCIF7) {
        // clear interrupt
        DMA2->HIFCR |= DMA_HISR_TCIF7;
		transmitting = 0;
    }
}

// Setup UART DMA
void uart_dma_init(void) {
	// Enable transmit DMA
	USART1->CR3 |= USART_CR3_DMAT;
	// Enable receive DMA
	USART1->CR3 |= USART_CR3_DMAR;
	// Clear TC bit
	USART1->SR &= ~USART_SR_TC;
	// Enable DMA2 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
}
