#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/init.h>

#include <openamp/open_amp.h>

#include <ipc.h>

static const struct device *const ipm_handle =
	DEVICE_DT_GET(DT_CHOSEN(zephyr_ipc));

int32_t init_ipc()
{
	if (!device_is_ready(ipm_handle)) {
		printk("IPM device is not ready\n");
		return -ENODEV;
	}

    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    int status = metal_init(&metal_params);
    if (status != 0) {
        printk("metal_init: failed - error code %d\n", status);
        return status;
    }



    return 0;
}
