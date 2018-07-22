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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "pwm.h"

#define PWM_FREQ 500 // Desired update rate of the LED PWM signal.

void pwm_init(void)
{
  // Enable the GPIO and timer clock that will be used.
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_TIM1);

  // Now setup the LED GPIOs to show their associated timer PWM channel output.
  // The mapping of LED GPIOs to timer channels is (from Table 10 of the datasheet):
  //  - PB13 (red LED) = Timer 1, channel 1
  //  - PB14 (green LED) = Timer 1, channel 2
  //  - PB15 (blue LED) = Timer 1, channel 3

  // Setup each LED to use its alternative function (GPIO_MODE_AF).
  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO13 | GPIO14 | GPIO15);

  // Configure the alternate function for each LED to use the timer PWM output.
  // See Table 13 in the datasheet for full matrix of pins and their alternate
  // function values.
  gpio_set_af(GPIOB, GPIO_AF2, GPIO13 | GPIO14 | GPIO15);

  // Configure both timers for PWM mode.  In PWM mode the timers will count up
  // for a specified period and the output of each timer channel will be set
  // high or low depending on if the timer value is above or below a threshold.

  // First configure timer 1.
  // Reset the timer configuration and then set it up to use the CPU clock,
  // center-aligned PWM, and an increasing rate.
  timer_reset(TIM1);
  timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
  // Divide counter by 48 to scale it down from 48mhz clock speed to a 1mhz rate.
  timer_set_prescaler(TIM1, 48);
  // Set timer period by solving:
  //   PWM frequency = timer clock speed / timer period
  timer_set_period(TIM1, 1000000/PWM_FREQ);
  timer_enable_break_main_output(TIM1);  // Must be called for advanced timers
                                         // like this one.  Unclear what this
                                         // does or why it's necessary but the
                                         // libopencm3 timer and STM32 docs
                                         // mention it.

  // Now setup each timer channel that is connected to a LED.  Each channel can
  // have a different PWM threshold set so that it can be uniquely controlled
  // (the rest of the timer configuration like period, etc. is shared by all
  // channels on a timer).

  // PB13 Red LED is timer 1, channel 1.
  // Enable the channel PWM output.
  timer_enable_oc_output(TIM1, TIM_OC1);
  // Set the PWM mode to 2 which means the PWM signal is low (LED turns on) when
  // the timer value is below the threshold and high (LED off) above it.
  timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM2);

  // PB14 Green LED is timer 1, channel 2.
  timer_enable_oc_output(TIM1, TIM_OC2);
  timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM2);

  // PB15 Blue LED is timer 1, channel 3.
  timer_enable_oc_output(TIM1, TIM_OC3);
  timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM2);

  // Turn on the timer.
  timer_enable_counter(TIM1);

  // Set the threshold for each channel to zero so they are off.  Setting the
  // threshold to any value between 0 and 2000 will set the LED intensity (with
  // 0 = off and 2000 = fully lit).
  timer_set_oc_value(TIM1, TIM_OC1, 2000);  // Red LED
  timer_set_oc_value(TIM1, TIM_OC2, 2000);  // Green LED
  timer_set_oc_value(TIM1, TIM_OC3, 2000); // Blue LED
}

// void pwm_init_output_channel(enum tim_oc_id oc_id)
// {
//      timer_disable_oc_output(TIM1, oc_id);
//      timer_set_oc_mode(TIM1, oc_id, TIM_OCM_PWM2);
//      timer_set_oc_value(TIM1, oc_id, 0);
//      timer_enable_oc_output(TIM1, oc_id);
// }
//
// void pwm_set_pulse_width(enum tim_oc_id oc_id, uint32_t pulse_width)
// {
//      timer_set_oc_value(TIM1, oc_id, pulse_width);
// }
//
// void pwm_start_timer(void)
// {
//      timer_enable_counter(TIM1);
// }

// Set the color of the LED to the provided red, green, and blue intensity
// value.  Each component should be a value from 0 to 1.0 where 0 is off and
// 1.0 is full intensity.  For example a cyan (full green and blue) color would
// be led_color(0, 1, 1).
void led_color(float red, float green, float blue) {
  // Scale each floating point value to be within the range of possible timer
  // periods (0 to 2000 if running at 500hz), then set the timer channel
  // threshold to that value.
  timer_set_oc_value(TIM1, TIM_OC1, red*(1000000/PWM_FREQ));
  timer_set_oc_value(TIM1, TIM_OC2, green*(1000000/PWM_FREQ));
  timer_set_oc_value(TIM1, TIM_OC3, blue*(1000000/PWM_FREQ));
}
