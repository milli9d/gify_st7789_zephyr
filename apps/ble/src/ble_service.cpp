#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>

int ble_service_init() {
    return 0;
}

SYS_INIT(ble_service_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);