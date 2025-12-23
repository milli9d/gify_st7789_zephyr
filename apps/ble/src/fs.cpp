#include <cstddef>
#include <cstdint>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/drivers/gpio.h>

#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/drivers/flash.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(LittleFS);

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
    .type        = FS_LITTLEFS,
    .mnt_point   = "/mnt",
    .fs_data     = &storage,
    .storage_dev = (void*)FIXED_PARTITION_ID(lfs_partition),
};

static int littlefs_flash_erase(unsigned int id)
{
    const struct flash_area* pfa;
    int                      rc = flash_area_open(id, &pfa);
    if (rc < 0) {
        LOG_ERR("FAIL: unable to find flash area %u: %d\n", id, rc);
        return rc;
    }

    LOG_PRINTK("Area %u at 0x%x on %s for %u bytes\n", id, (unsigned int)pfa->fa_off,
               pfa->fa_dev->name, (unsigned int)pfa->fa_size);

    /* Optional wipe flash contents */
    rc = flash_area_flatten(pfa, 0, pfa->fa_size);
    LOG_ERR("Erasing flash area ... %d", rc);

    flash_area_close(pfa);
    return rc;
}

int init_file_system()
{
    struct fs_mount_t*       mp  = &lfs_storage_mnt;
    const struct flash_area* pfa = nullptr;

    int rc = flash_area_open((uintptr_t)mp->storage_dev, &pfa);
    if (rc < 0) {
        LOG_ERR("FAIL: unable to find flash area %u: %d\n", mp->storage_dev, rc);
        return rc;
    }
    LOG_INF("Flash Area Size is %lu kb", pfa->fa_size / 1024u);

    // rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
    // if (rc < 0) {
    //     return rc;
    // }

    rc = fs_mount(mp);
    if (rc < 0) {
        LOG_ERR("FAIL: mount id %" PRIuPTR " at %s: %d\n", (uintptr_t)mp->storage_dev,
                mp->mnt_point, rc);
        return rc;
    }

    LOG_INF("LittleFS initialized on Flash.");
    return 0;
}

SYS_INIT(init_file_system, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

/* shell command for erasing flash */

#include <zephyr/shell/shell.h>

static int cmd_erase_flash(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    /* re mount */
    int rc = fs_unmount(&lfs_storage_mnt);
    if (rc < 0) {
        shell_print(shell, "Error unmounting LittleFS before erase: %d", rc);
        return rc;
    }

    rc = littlefs_flash_erase(FIXED_PARTITION_ID(lfs_partition));
    if (rc < 0) {
        shell_print(shell, "Error erasing flash: %d", rc);
        return rc;
    }

    rc = fs_mount(&lfs_storage_mnt);
    if (rc < 0) {
        shell_print(shell, "Error mounting LittleFS after erase: %d", rc);
        return rc;
    }

    shell_print(shell, "Flash erase completed.");
    return 0;
}

/* flash submenu containing erase command */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_flash,
                               SHELL_CMD(erase, NULL, "Erase LittleFS flash partition",
                                         cmd_erase_flash),
                               SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(flash, &sub_flash, "Flash operations", NULL);
