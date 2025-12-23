#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

int main()
{
    printk("Hello Zephyr BLE Gify App!\n");

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}