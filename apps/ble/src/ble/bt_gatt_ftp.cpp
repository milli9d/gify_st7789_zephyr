#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <zephyr/fs/fs.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(BT_GATT_FTP);

#include <ble/ble_init_prio.h>

#define FILE_PATH_PREFIX "/flash/"

static struct fs_file_t file;
static char             _path[256u];

#define BT_UUID_SVC_FTP \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xDEADD00D, 0x0000, 0x0000, 0xDADA, 0xDEEDDEAD0000))

#define BT_UUID_ATTR_OPEN_FILE \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xDEADD00D, 0x0000, 0x0001, 0xDADA, 0xDEEDDEAD0000))

#define BT_UUID_ATTR_WRITE_CHUNK \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xDEADD00D, 0x0000, 0x0003, 0xDADA, 0xDEEDDEAD0000))

#define BT_UUID_ATTR_CLOSE_FILE \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xDEADD00D, 0x0000, 0x0002, 0xDADA, 0xDEEDDEAD0000))

#pragma pack(push, 1)
typedef struct file_open_data_t {
    uint32_t file_sz;
    uint16_t msg_len;
    char     msg[];
} file_open_data_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct file_close_data_t {
    uint8_t status;
} file_close_data_t;
#pragma pack(pop)

int bt_gatt_ftp_init()
{
    LOG_INF("GATT File Transfer Service Initialized");
    return 0;
}

ssize_t bt_gatt_ftp_write_chunk(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                const void* buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    LOG_HEXDUMP_DBG(buf, len, "Received file chunk:");

    int rc = fs_write(&file, buf, len);
    if (rc != len) {
        LOG_ERR("Writing filesystem failed");
        return rc;
    }

    return len;
}

ssize_t bt_gatt_ftp_write_close_file(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                     const void* buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    if (len < sizeof(file_close_data_t)) {
        LOG_ERR("Invalid file close data length: %u", len);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    file_close_data_t* close_data = (file_close_data_t*)buf;
    LOG_INF("File close status: %u", close_data->status);

    if (static_cast<uint8_t>(close_data->status) != 0) {
        LOG_ERR("File transfer reported error status: %u", close_data->status);
        /* delete file. */
        fs_unlink(_path);
        return len;
    }

    int rc = fs_close(&file);
    if (rc != 0) {
        LOG_ERR("Closing file failed");
        return rc;
    }

    return len;
}

ssize_t bt_gatt_ftp_write_open_file(struct bt_conn* conn, const struct bt_gatt_attr* attr,
                                    const void* buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    if (len < sizeof(file_open_data_t)) {
        LOG_ERR("Invalid file open data length: %u", len);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    file_open_data_t* file_data = (file_open_data_t*)buf;
    LOG_INF("File size: %u bytes", file_data->file_sz);
    LOG_INF("File name length: %u", file_data->msg_len);

    if (len < sizeof(file_open_data_t) + file_data->msg_len) {
        LOG_ERR("Invalid file name length: %u", file_data->msg_len);
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }
    LOG_INF("File name: %.*s", file_data->msg_len, file_data->msg);

    fs_file_t_init(&file);

    snprintf(_path, sizeof(_path), "%s%.*s", FILE_PATH_PREFIX, file_data->msg_len, file_data->msg);

    LOG_INF("Opening file at path: %s", _path);

    int rc = fs_open(&file, _path, FS_O_CREATE | FS_O_WRITE | FS_O_TRUNC);
    if (rc != 0) {
        LOG_ERR("Accessing filesystem failed");
        return rc;
    }

    return len;
}

BT_GATT_SERVICE_DEFINE(
    ftp_svc, BT_GATT_PRIMARY_SERVICE(BT_UUID_SVC_FTP),
    BT_GATT_CHARACTERISTIC(BT_UUID_ATTR_OPEN_FILE, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
                           bt_gatt_ftp_write_open_file, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_ATTR_CLOSE_FILE, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
                           bt_gatt_ftp_write_close_file, NULL),
    BT_GATT_CHARACTERISTIC(BT_UUID_ATTR_WRITE_CHUNK, BT_GATT_CHRC_WRITE, BT_GATT_PERM_WRITE, NULL,
                           bt_gatt_ftp_write_chunk, NULL), );

SYS_INIT(bt_gatt_ftp_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);