#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BT_GATT_DEVINFO);

#include <ble/ble_init_prio.h>

#define BT_UUID_SVC_DEV_INFO    BT_UUID_DECLARE_32(0xDEADD00D)

#define BT_UUID_ATTR_FW_VERSION BT_UUID_DECLARE_32(0x0001)

int bt_gatt_devinfo_init()
{
    LOG_INF("GATT Device Information Service Initialized");
    return 0;
}

ssize_t bt_gatt_devinfo_read_fw_version(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                        void* buf, uint16_t len, uint16_t offset)
{
    const char* fw_version = "v1.0.0";
    return bt_gatt_attr_read(conn, attr, buf, len, offset, fw_version, strlen(fw_version));
}

BT_GATT_SERVICE_DEFINE(devinfo_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_SVC_DEV_INFO),
                       BT_GATT_CHARACTERISTIC(BT_UUID_ATTR_FW_VERSION, BT_GATT_CHRC_READ,
                                              BT_GATT_PERM_READ, bt_gatt_devinfo_read_fw_version,
                                              NULL, NULL), );

SYS_INIT(bt_gatt_devinfo_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
