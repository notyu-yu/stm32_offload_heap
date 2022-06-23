#include "uart_dma.h"

static int receiving=0;
static int transmitting=0;

// Setup uart transmission
static void uart_tx_setup(void) {
	// Clear control register
	DMA1_Stream6->CR = 0;
	// Wait for DMA to disable
	while(DMA1_Stream6->CR & (1<<0));
	// Select channel 4 for usart2_tx
	DMA1_Stream6->CR |= (0x4<<25);
	// Enable tx complete interrupt
	DMA1_Stream6->CR |= DMA_SxCR_TCIE;
	// Enable memory increment mode
	DMA1_Stream6->CR |= DMA_SxCR_MINC;
	// Priority level high
	DMA1_Stream6->CR |= (0x2<<16);
	// DIR bit set to 01: source SxM0AR, dest SxPAR
	DMA1_Stream6->CR |= (0x1 << 6);
}

// Setup uart reception
static void uart_rx_setup(void) {
	// Enable receive DMA
	USART2->CR3 |= USART_CR3_DMAR;
	// Clear control register
	DMA1_Stream5->CR = 0;
	// Wait for DMA to disable
	while(DMA1_Stream5->CR & (1<<0));
	// Select channel 4 for usart2_rx
	DMA1_Stream5->CR |= (0x4<<25);
	// Enable rx complete interrupt
	DMA1_Stream5->CR |= DMA_SxCR_TCIE;
	// Enable memory increment mode
	DMA1_Stream5->CR |= DMA_SxCR_MINC;
	// Priority level high
	DMA1_Stream5->CR |= (0x2<<16);
	// DIR bit set to 00: source SxPAR, dest SxM0AR
	DMA1_Stream5->CR &= ~(0xC << 6);
}

// Start uart transmission of size bytes of data
void uart_tx_start(void * data, size_t size) {
	uart_tx_wait();
	uart_tx_setup();

	// Source memory address
	DMA1_Stream6->M0AR = (uint32_t)data;
	// Destination memory address
	DMA1_Stream6->PAR = (uint32_t)&(USART2->DR);
	// Transfer size
	DMA1_Stream6->NDTR = size;

	// Enable transfer Complete interrupt
	NVIC_SetPriority(DMA1_Stream6_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);

	// Enable DMA
	DMA1_Stream6->CR |= DMA_SxCR_EN;

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
	DMA1_Stream5->PAR = (uint32_t)&(USART2->DR);
	// Destination memory address
	DMA1_Stream5->M0AR = (uint32_t)buffer;
	// Transfer size
	DMA1_Stream5->NDTR = size;

	// Enable transfer Complete interrupt
	NVIC_SetPriority(DMA1_Stream5_IRQn, 3);
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	// Enable DMA
	DMA1_Stream5->CR |= DMA_SxCR_EN;

	receiving=1;
}

// Wait for uart reception to finish
void uart_rx_wait(void) {
	while (receiving);
}

// UART reception finish interrupt
void DMA1_Stream5_IRQHandler(void)
{
    // clear stream receive complete interrupt - bit11 for stream 5
    if (DMA1->HISR & DMA_HISR_TCIF5) {
        // clear interrupt
        DMA1->HIFCR |= DMA_HISR_TCIF5;
		receiving = 0;
		// Disable receive DMA
		USART2->CR3 &= ~USART_CR3_DMAR;
    }
}

// UART transmission finish interrupt
void DMA1_Stream6_IRQHandler(void)
{
    // clear stream transfer complete interrupt - bit21 for stream 6
    if (DMA1->HISR & DMA_HISR_TCIF6) {
        // clear interrupt
        DMA1->HIFCR |= DMA_HISR_TCIF6;
		transmitting = 0;
    }
}

// Setup UART DMA
void uart_dma_init(void) {
	// Enable transmit DMA
	USART2->CR3 |= USART_CR3_DMAT;
	// Enable receive DMA
	USART2->CR3 |= USART_CR3_DMAR;
	// Clear TC bit
	USART2->SR &= ~USART_SR_TC;
	// Enable DMA1 clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
}
