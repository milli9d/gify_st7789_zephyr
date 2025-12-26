#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

void boot_banner()
{
    printk(R"(
         _____________________________   
        /        _____________        \
        | == .  |             |     o |
        |   _   |             |   B   |
        |  / \  |             | A  O  |
        | | O | |             |  O    |
        |  \_/  |             |       |
        | B L E |             | . . . |
        |  :::  |             | . . . |
        |  :::  |_____________| . . . |
        |           G I F Y           |
        \_____________________________/
    )"
           "\n");
}

int main()
{
    boot_banner();

    while (1) {
        k_sleep(K_MSEC(1000));
    }

    return 0;
}