#include <stdio.h>
#include <stdlib.h>
#include <ctime>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LVGL);

#include "pin_defines.hpp"

#include <lvgl.h>
#include <lvgl_input_device.h>

static void init_tft()
{
    const struct device* display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        LOG_ERR("Device not ready, aborting test");
        return;
    }

    /* turn on TFT LED */
    const struct gpio_dt_spec tft_led = TFT_LED_GPIO;
    gpio_pin_configure_dt(&tft_led, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);

    display_set_orientation(display_dev, DISPLAY_ORIENTATION_ROTATED_180);

    display_blanking_off(display_dev);
}

static void lvgl_demo()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0x003a57),
                              LV_PART_MAIN);

    lv_obj_t* hello_world_label;

    lv_obj_t* hello_world_button = lv_button_create(lv_screen_active());
    lv_obj_align(hello_world_button, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(hello_world_button, NULL, LV_EVENT_CLICKED, NULL);
    hello_world_label = lv_label_create(hello_world_button);
}

static void lv_task_handler(struct k_timer* timer)
{
    lv_timer_handler();
}

K_TIMER_DEFINE(lvgl_timer, lv_task_handler, nullptr);

int main()
{
    printk("Welcome to ST7789 demo!\n");

    init_tft();
    // lvgl_demo();

    k_timer_start(&lvgl_timer, K_NO_WAIT, K_NO_WAIT);

    k_thread_suspend(k_current_get());

    return 0;
}
