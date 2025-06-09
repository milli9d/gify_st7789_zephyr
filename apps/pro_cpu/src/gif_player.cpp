#include <stdio.h>
#include <stdint.h>
#include <span>
#include <memory>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>

#include <AnimatedGIF.h>

#include <gifs/big_scr.h>
// #include <gifs/sunset_gif.h>

#include <gif_player.hpp>

namespace gify {

#define GIF_PLAYER_STACK_SIZE 2048u

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)
#define TFT_LED_GPIO     GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, tft_led_gpios)

AnimatedGIF _gif;
uint8_t     ucFrameBuffer[(TFT_W * TFT_H) +
                      ((TFT_W * TFT_H) * BPP_COOKED)] __attribute__((section("storage2"))) = {}; // holds current canvas as 8-bpp and
                                                                // 16-bit output after that

K_THREAD_STACK_DEFINE(gif_player_stack, GIF_PLAYER_STACK_SIZE);
struct k_thread gif_player_thread_data;

void clear_screen(const struct device* scr)
{
    uint16_t*                        _buf  = (uint16_t*)calloc(240 * 280 * sizeof(uint16_t), 0u);
    struct display_buffer_descriptor _desc = { .buf_size         = 240u * 280u * 2u,
                                               .width            = 240u,
                                               .height           = 280u,
                                               .pitch            = 240u,
                                               .frame_incomplete = false };

    int rc = display_write(scr, 0u, 0u, &_desc, (void*)&_buf[0u]);
    if (rc != 0) {
        printk("Failed to write to display: %d\n", rc);
    }

    free(_buf);
    _buf = nullptr;
}

void gif_player::_run(void* p0, void* p1, void* p2)
{
    // Cast the argument to the correct type
    gif_player* player = static_cast<gif_player*>(p0);
    if (player == nullptr) {
        printf("Error: pointer to base class is NULL\n");
        return;
    }

    /* turn on TFT LED */
    const struct gpio_dt_spec tft_led = TFT_LED_GPIO;
    gpio_pin_configure_dt(&tft_led, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_HIGH);

    const struct device* dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(dev)) {
        printk("Device %s not ready\n", dev->name);
        return;
    }

    if (display_set_pixel_format(dev, PIXEL_FORMAT_RGB_565) != 0) {
        printk("Failed to set required pixel format\n");
        return;
    }

    struct display_capabilities caps;
    display_get_capabilities(dev, &caps);

    int rc = display_blanking_off(dev);
    if (rc != 0) {
        printk("Failed to turn on display blanking: %d\n", rc);
    }

    clear_screen(dev);

    while (1) {
        _gif.begin(GIF_PALETTE_RGB565_BE); // Choose the "OLED" version of 1-bpp
                                           // output (vertical bytes, LSB on top)

        // For 1-bit output, we can pass NULL for the GIFDraw method pointer; it
        // won't be needed
        if (_gif.open((uint8_t*)gif, sizeof(gif), NULL)) {
            // For 1-bit output, the whole frame (8-bit + 16-bit) is written
            // into the buffer we set below
            _gif.setFrameBuf(ucFrameBuffer);
            // We want the library to generate ready-made (COOKED) pixels
            _gif.setDrawType(GIF_DRAW_COOKED);
            printf("Successfully opened GIF; Canvas size = %d x %d\n", _gif.getCanvasWidth(),
                   _gif.getCanvasHeight());
            GIFINFO info = {};
            _gif.getInfo(&info);
            printf("Number of frames = %d\n"
                   "Duration [ms] = %d\n"
                   "Max Delay [ms] = %d]\n"
                   "Min Delay [ms] = %d]\n",
                   info.iFrameCount, info.iDuration, info.iMaxDelay, info.iMinDelay);

#ifndef FRAME_RATE
            uint64_t avg_frame_delay_us = ((info.iMaxDelay + info.iMinDelay) / 2u) * 1000u;
#else
            uint64_t avg_frame_delay_us = 1000000u / FRAME_RATE;
#endif

            int count = 0;
            while (_gif.playFrame(false, NULL)) { // live dangerously; run unthrottled :)
                uint64_t  start_time_ticks = k_uptime_get();
                uint16_t* current_frame    = (uint16_t*)(ucFrameBuffer + (TFT_W * TFT_H));

                struct display_buffer_descriptor _desc = { .buf_size =
                                                               ((TFT_W * TFT_H) * BPP_COOKED),
                                                           .width            = TFT_W,
                                                           .height           = TFT_H,
                                                           .pitch            = TFT_W,
                                                           .frame_incomplete = false };

                rc = display_write(dev, 0u, 0u, &_desc, (void*)current_frame);
                if (rc != 0) {
                    printk("Failed to write to display: %d\n", rc);
                }

                // make things run at proper frame rate
                uint64_t time_spent_us = k_ticks_to_us_ceil64(k_uptime_get() - start_time_ticks);
                if (time_spent_us > avg_frame_delay_us) {
                    printf("Frame %d took too long to draw: %lld us\n", count, time_spent_us);
                    continue;
                }
                k_sleep(K_USEC(avg_frame_delay_us - time_spent_us));
            }
            _gif.close();
        }
    }
}

gif_player::gif_player()
{
    // Constructor implementation
    printf("gif_player constructor called\n");

    // Create the task for the gif_player
    k_thread_create(&gif_player_thread_data, gif_player_stack,
                    K_THREAD_STACK_SIZEOF(gif_player_stack), _run, this, NULL, NULL,
                    K_PRIO_PREEMPT(0), 0, K_NO_WAIT);
    k_thread_name_set(&gif_player_thread_data, "GIF_PLAYER");
    k_thread_start(&gif_player_thread_data);
}

gif_player::~gif_player()
{
    // Destructor implementation
    printf("gif_player destructor called\n");
}

} // namespace gify
