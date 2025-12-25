#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(BT_GATT_DEVINFO);

#include <ble/ble_init_prio.h>

int bt_gatt_devinfo_init()
{
    LOG_INF("GATT Device Information Service Initialized");
    return 0;
}

SYS_INIT(bt_gatt_devinfo_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
