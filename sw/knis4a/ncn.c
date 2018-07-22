/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Bart Van Der Meerssche <bart@flukso.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include "log.h"
#include "ncn.h"

static struct ncn_tx_buffer tx_buffer = {
	.empty = true
};

void ncn_setup(void)
{
	nvic_enable_irq(NVIC_USART1_IRQ);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_USART1);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO7);
	gpio_set_af(GPIOB, GPIO_AF0, GPIO6);
	gpio_set_af(GPIOB, GPIO_AF0, GPIO7);
	usart_set_baudrate(USART1, 38400);
	usart_set_databits(USART1, 8);
	usart_set_parity(USART1, USART_PARITY_NONE);
//	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_enable_rx_interrupt(USART1);
	usart_enable(USART1);
}

void usart1_isr(void)
{
	if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
	    ((USART_ISR(USART1) & USART_ISR_RXNE) != 0)) {

		log_write(LOG_TOPIC_NCN, LOG_LVL_TRACE, "rx: %x", usart_recv(USART1));
	}
	if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
	    ((USART_ISR(USART1) & USART_ISR_TXE) != 0)) {

		if (tx_buffer.idx < tx_buffer.fill) {
			log_write(LOG_TOPIC_NCN, LOG_LVL_TRACE, "tx: %x",
			    tx_buffer.data[tx_buffer.idx]);
			usart_send(USART1, tx_buffer.data[tx_buffer.idx++]);
		} else {
			usart_disable_tx_interrupt(USART1);
			tx_buffer.empty = true;
		}
	}
}

bool ncn_tx(char *frame, size_t len)
{
	if (!tx_buffer.empty || len > NCN_TX_BUFFER_SIZE) {
		return false;
	}
	tx_buffer.empty = false;
	tx_buffer.idx = 0;
	tx_buffer.fill = len;
	memcpy(tx_buffer.data, frame, len);
	usart_enable_tx_interrupt(USART1);
	return true;
}
