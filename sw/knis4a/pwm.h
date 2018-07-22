#ifndef __PWM_H_
#define __PWM_H_


void pwm_init(void);
void led_color(float red, float green, float blue);
// void pwm_init_output_channel(enum tim_oc_id oc_id);
// void pwm_start_timer(void);
// void pwm_set_pulse_width(enum tim_oc_id oc_id, uint32_t pulse_width);

#endif
