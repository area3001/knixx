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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "led.h"

typedef struct {
  uint32_t  	gpioport;
	uint16_t  	gpios;
} led_gpio;

const led_gpio leds[LEDS_COUNT]={
  { .gpioport = GPIOC, .gpios = GPIO13 },
  { .gpioport = GPIOB, .gpios = GPIO3 },
  { .gpioport = GPIOA, .gpios = GPIO3 },
  { .gpioport = GPIOB, .gpios = GPIO0 },
  { .gpioport = GPIOA, .gpios = GPIO2 },
  { .gpioport = GPIOA, .gpios = GPIO5 }
};

void led_setup(void)
{
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_GPIOC);
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2 | GPIO3 | GPIO5);
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO0 | GPIO3);
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

  set_led(LED_A2);
}

void set_led(e_leds led){
  int i;
  for(i=0; i<LEDS_COUNT; i++) {
    gpio_clear(leds[i].gpioport, leds[i].gpios);
  }
  gpio_set(leds[led].gpioport, leds[led].gpios);
}

// void tim6_isr(void)
// {
// 	TIM_SR(TIM6) &= ~TIM_SR_UIF;
//   currentLed++;
//   currentLed%=MAX_LEDS;
//   for(i=0; i< MAX_LEDS; i++)
//   {
//     if(i == currentLed) gpio_set(GPIOB,GPIO3);
//   }
//
// 	gpio_toggle(GPIOC,GPIO13);
//   gpio_toggle(GPIOB,GPIO3);    // LED_A2
//   gpio_toggle(GPIOA,GPIO3);  // LED_A3
//   gpio_toggle(GPIOB,GPIO0);  // LED_A4
//   gpio_toggle(GPIOA,GPIO2);  // LED_A5
//   gpio_toggle(GPIOA,GPIO5);  // LED_A6
// }
//
// /*
//  * setup timer 6 to generate an overflow interrupt at 1ms
//  * It is used to switch leds
//  */
// void led_setup(void)
// {
// 	rcc_periph_reset_pulse(RST_TIM6);
// 	/* 48Mhz / 10khz -1. */
// 	timer_set_prescaler(TIM6, 4799); /* 48Mhz/10000hz - 1 */
// 	/* 10khz for 10 ticks = 1 khz overflow = 1ms overflow interrupts */
// 	timer_set_period(TIM6, 10);
//
// 	nvic_enable_irq(NVIC_TIM6_IRQ);
// 	timer_enable_update_event(TIM6); /* default at reset! */
// 	timer_enable_irq(TIM6, TIM_DIER_UIE);
// 	timer_enable_counter(TIM6);
// }
