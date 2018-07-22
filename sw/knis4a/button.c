/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Bart Van Der Meerssche <bart@flukso.net>
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

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/exti.h>
#include "usb.h"
#include "log.h"
#include "button.h"

void button_setup(void)
{
	nvic_enable_irq(NVIC_EXTI0_1_IRQ);
	nvic_enable_irq(NVIC_EXTI4_15_IRQ);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
	                GPIO1 | GPIO4 | GPIO6 | GPIO7 | GPIO15);
	gpio_mode_setup(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO9);
	exti_select_source(EXTI1, GPIOA);
	exti_select_source(EXTI4, GPIOA);
	exti_select_source(EXTI6, GPIOA);
	exti_select_source(EXTI7, GPIOA);
	exti_select_source(EXTI9, GPIOB);
	exti_select_source(EXTI15, GPIOA);
	exti_set_trigger(EXTI1 | EXTI4 | EXTI6 | EXTI7 | EXTI9 | EXTI15,
	                 EXTI_TRIGGER_BOTH);
	exti_reset_request(EXTI1 | EXTI4 | EXTI6 | EXTI7 | EXTI9 | EXTI15);
	exti_enable_request(EXTI1 | EXTI4 | EXTI6 | EXTI7 | EXTI9 | EXTI15);
}

static void button_handler(enum button btn, bool low)
{
	if (low) {
		log_write(LOG_TOPIC_BUTTON, LOG_LVL_INFO, "%i pushed", btn + 1);
	} else {
		log_write(LOG_TOPIC_BUTTON, LOG_LVL_INFO, "%i released", btn + 1);
	}
}

void exti0_1_isr(void)
{
	if (exti_get_flag_status(EXTI1)) {
		exti_reset_request(EXTI1);
		button_handler(BUTTON_5, gpio_get(GPIOA, GPIO1) == 0);
	}
}

void exti4_15_isr(void)
{
	if (exti_get_flag_status(EXTI4)) {
		exti_reset_request(EXTI4);
		button_handler(BUTTON_3, gpio_get(GPIOA, GPIO4) == 0);
	}
	if (exti_get_flag_status(EXTI6)) {
		exti_reset_request(EXTI6);
		button_handler(BUTTON_6, gpio_get(GPIOA, GPIO6) == 0);
	}
	if (exti_get_flag_status(EXTI7)) {
		exti_reset_request(EXTI7);
		button_handler(BUTTON_4, gpio_get(GPIOA, GPIO7) == 0);
	}
	if (exti_get_flag_status(EXTI9)) {
		exti_reset_request(EXTI9);
		button_handler(BUTTON_1, gpio_get(GPIOB, GPIO9) == 0);
	}
	if (exti_get_flag_status(EXTI15)) {
		exti_reset_request(EXTI15);
		button_handler(BUTTON_2, gpio_get(GPIOA, GPIO15) == 0);
	}
}

