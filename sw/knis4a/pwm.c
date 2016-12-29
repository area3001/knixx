/*
 * This file is part of the PWM-Servo example.
 *
 * Copyright (C) 2011 Stefan Wendler <sw@kaltpost.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "pwm.h"

void pwm_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_TIM1);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO13 | GPIO14 | GPIO15);
	rcc_periph_clock_enable(RCC_TIM1);
	timer_reset(TIM1);
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1, TIM_CR1_DIR_UP);
	timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM2);
	timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM2);
	timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM2);
	timer_enable_oc_output(TIM1, TIM_OC1);
	timer_enable_oc_output(TIM1, TIM_OC2);
	timer_enable_oc_output(TIM1, TIM_OC3);
	timer_enable_break_main_output(TIM1);
	timer_set_oc_value(TIM1, TIM_OC1, 200);
	timer_set_oc_value(TIM1, TIM_OC2, 200);
	timer_set_oc_value(TIM1, TIM_OC3, 200);
	timer_set_period(TIM1, 1000);
	timer_enable_counter(TIM1);
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
