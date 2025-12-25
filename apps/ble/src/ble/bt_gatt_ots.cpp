#include <stddef.h>
#include <errno.h>

#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/bluetooth/services/ots.h>

#include <zephyr/sys/byteorder.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BT_OTS);

#include <ble/ble_init_prio.h>

#define BT_GATT_UUID_FTP_SERVICE BT_UUID_DECLARE_16(0x1826)

#define BT_GATT_UUID_FTP_CHAR_DF  BT_UUID_DECLARE_16(0x2ACB)
#define BT_GATT_UUID_FTP_CHAR_PHY BT_UUID_DECLARE_16(0x2ACC)

static int32_t init_ots_instance()
{
    struct bt_ots* ots = bt_ots_free_instance_get();
    if (!ots) {
        LOG_ERR("Failed to retrieve OTS instance");
        return -EINVAL;
    }

    return 0;
}

int bt_ots_init()
{
    int rc = init_ots_instance();
    if (rc != 0) {
        LOG_ERR("Failed to init OTS (err %d)", rc);
        return rc;
    }

    LOG_INF("BT Object Transfer Service Initialized");
    return 0;
}

SYS_INIT(bt_ots_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);