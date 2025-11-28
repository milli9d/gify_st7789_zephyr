#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#define POWER_OFF_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, power_off_gpios)
#define GYRO_CS_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, gyro_cs_gpios)
#define BUMPER_LED_GPIO GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, bumper_leds_gpios)

#define TFT1_CS_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, tft1_cs_gpios)
#define TFT1_LED_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, tft1_led_gpios)

#define TFT2_CS_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, tft2_cs_gpios)
#define TFT2_LED_GPIO DT_GPIO_LABEL(ZEPHYR_USER_NODE, tft2_led_gpios)
