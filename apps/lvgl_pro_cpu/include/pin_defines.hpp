#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#define TFT_LED_GPIO     GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, tft_led_gpios)
