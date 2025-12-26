#include <stddef.h>
#include <errno.h>
#include <array>

#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <zephyr/bluetooth/att.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BT_GAP_Peripheral);

#include <ble/ble_init_prio.h>

enum {
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_BITS,
};

static ATOMIC_DEFINE(state, STATE_BITS);

std::array<uint8_t, 6u> mfg_data = { 0xAD, 0xDE, 'G', 'I', 'F', 'Y' };

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data.data(), mfg_data.size()),
};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, sizeof(CONFIG_BT_DEVICE_NAME) - 1),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_OTS_VAL)),
};

static void adv_restart_work_handler(struct k_work* work);
static K_WORK_DELAYABLE_DEFINE(adv_restart_work, adv_restart_work_handler);

static void adv_restart_work_handler(struct k_work* work)
{
    int rc = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (rc) {
        LOG_ERR("Advertising failed to start (err %d)", rc);
        // Retry after 1 second if it failed
        k_work_schedule(&adv_restart_work, K_MSEC(1000));
    } else {
        LOG_INF("Advertising successfully restarted");
    }
}

static void mtu_updated(struct bt_conn* conn, uint16_t tx, uint16_t rx)
{
    printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}
static struct bt_gatt_cb gatt_callbacks = { .att_mtu_updated = mtu_updated };

static void mtu_exchange_cb(struct bt_conn* conn, uint8_t err,
                            struct bt_gatt_exchange_params* params)
{
    printk("%s: MTU exchange %s (%u)\n", __func__, err == 0U ? "successful" : "failed",
           bt_gatt_get_mtu(conn));
}

static struct bt_gatt_exchange_params mtu_exchange_params = { .func = mtu_exchange_cb };

static int mtu_exchange(struct bt_conn* conn)
{
    int err;

    printk("%s: Current MTU = %u\n", __func__, bt_gatt_get_mtu(conn));

    printk("%s: Exchange MTU...\n", __func__);
    err = bt_gatt_exchange_mtu(conn, &mtu_exchange_params);
    if (err) {
        printk("%s: MTU exchange failed (err %d)", __func__, err);
    }

    return err;
}

static void connected(struct bt_conn* conn, uint8_t err)
{
    bt_conn_ref(conn);

    (void)mtu_exchange(conn);

    if (err) {
        printk("Connection failed, err 0x%02x %s\n", err, bt_hci_err_to_str(err));
    } else {
        printk("Connected\n");
        (void)atomic_set_bit(state, STATE_CONNECTED);
    }
}

static void disconnected(struct bt_conn* conn, uint8_t reason)
{
    bt_conn_unref(conn);
    printk("Disconnected, reason 0x%02x %s\n", reason, bt_hci_err_to_str(reason));
    (void)atomic_set_bit(state, STATE_DISCONNECTED);

    // Schedule advertising restart after a short delay to allow stack cleanup
    k_work_schedule(&adv_restart_work, K_MSEC(100));
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected    = connected,
    .disconnected = disconnected,
};

int bt_gap_init()
{
    int rc = bt_enable(NULL);
    if (rc) {
        LOG_ERR("Bluetooth init failed (err %d)", rc);
        return rc;
    }
    LOG_INF("Bluetooth initialized");

    bt_gatt_cb_register(&gatt_callbacks);

    rc = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (rc) {
        LOG_ERR("Advertising failed to start (err %d)", rc);
        return rc;
    }

    LOG_INF("Advertising successfully started");
    return 0;
}

SYS_INIT(bt_gap_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);