/*
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

#ifndef __PWM_H_
#define __PWM_H_

#include <libopencm3/stm32/timer.h>

void pwm_init(void);
// void pwm_init_output_channel(enum tim_oc_id oc_id);
// void pwm_start_timer(void);
// void pwm_set_pulse_width(enum tim_oc_id oc_id, uint32_t pulse_width);

#endif
