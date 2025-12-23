#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>

#include "pin_defines.hpp"
#include "gif_player.hpp"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main()
{
    LOG_INF("Gify ST7789 ESP32 Demo!");
    static gify::gif_player gif_player;
    k_thread_suspend(k_current_get());
    return 0;
}
