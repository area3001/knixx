#ifndef LED_H_
#define LED_H_

typedef enum {
  LED_A1,
  LED_A2,
  LED_A3,
  LED_A4,
  LED_A5,
  LED_A6,
  LEDS_COUNT
} e_leds;

/**
 * Initialize and start the PWM used for the LEDs.
 */
void led_setup(void);
void set_led(e_leds led);

#endif
