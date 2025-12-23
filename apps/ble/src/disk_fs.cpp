#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/display/cfb.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/random/random.h>
#include <zephyr/input/input.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <zephyr/storage/disk_access.h>
#include <ff.h>

LOG_MODULE_REGISTER(diskfs);

#define DISK_DRIVE_NAME "SD"
#define DISK_MOUNT_PT   "/SD:"

static FATFS             fat_fs;
static struct fs_mount_t mp = {
    .type      = FS_FATFS,
    .mnt_point = DISK_MOUNT_PT,
    .fs_data   = &fat_fs,
    .flags     = FS_MOUNT_FLAG_NO_FORMAT,
};

static const char* disk_mount_pt = DISK_MOUNT_PT;

int diskfs_svc_init()
{
    // FATFS mkfs parameters: auto-select FAT type and cluster size.
    MKFS_PARM mkfs_cfg{};
    mkfs_cfg.fmt     = FM_ANY | FM_SFD; // Format as super-floppy (no partition table)
    mkfs_cfg.n_fat   = 1;               // Single FAT table
    mkfs_cfg.align   = 0;               // Query sector size from diskio
    mkfs_cfg.n_root  = CONFIG_FS_FATFS_MAX_ROOT_ENTRIES;
    mkfs_cfg.au_size = 0; // Auto calculate cluster size

    mp.mnt_point = disk_mount_pt;

    /* Format the drive (mkfs) using FATFS across full device and then mount it */
    const char* fat_dev = DISK_DRIVE_NAME ":"; // e.g. "SD:"
    int         ret     = fs_mkfs(FS_FATFS, (uintptr_t)fat_dev, &mkfs_cfg, 0);
    if (ret != 0) {
        LOG_ERR("mkfs FATFS failed: %d", ret);
    }

    int res = fs_mount(&mp);
    if (res == 0u) {
        LOG_INF("FATFS disk mounted.");
    } else {
        LOG_ERR("Error mounting FATFS disk: %d", res);
    }

    /* Optional: print basic FS stats to verify mount */
    if (res == 0) {
        struct fs_statvfs stat;

        int sret = fs_statvfs(mp.mnt_point, &stat);
        if (sret == 0) {
            LOG_INF("FATFS stats: bsize=%lu blocks=%lu free=%lu", (unsigned long)stat.f_bsize,
                    (unsigned long)stat.f_blocks, (unsigned long)stat.f_bfree);
        } else {
            LOG_ERR("fs_statvfs failed: %d", sret);
        }
    }

    return 0;
}

SYS_INIT(diskfs_svc_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);