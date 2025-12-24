#include <stddef.h>
#include <errno.h>

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BT_GAP_Peripheral);

#define BT_GATT_UUID_FTP_SERVICE BT_UUID_DECLARE_16(0x1826)

#define BT_GATT_UUID_FTP_CHAR_DF  BT_UUID_DECLARE_16(0x2ACB)
#define BT_GATT_UUID_FTP_CHAR_PHY BT_UUID_DECLARE_16(0x2ACC)


int bt_gatt_ftp_init()
{
    printk("GATT FTP Service Initialized\n");
    return 0;
}

SYS_INIT(bt_gatt_ftp_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);