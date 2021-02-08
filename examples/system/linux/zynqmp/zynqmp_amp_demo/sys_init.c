
/*
 * Copyright (c) 2021, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <metal/sys.h>
#include "common.h"

int sys_init()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	int ret;

	ret = metal_init(&init_param);
	if (ret)
		LPERROR("Failed to initialize libmetal\n");
#ifdef versal
	/* ensure TTC0 is powered on before using it in demos. */
	printf("\r\nLinux:> request TTC0 node from Xilinx Platform Manager.\r\n");
	ret = system("echo pm_request_node 0x18224024 > /sys/kernel/debug/zynqmp-firmware/pm");
	if (ret < 0) {
		perror("Failed to request TTC0 from Xilinx Platform Manager..\n");
		return -EINVAL;
	}
#endif
	return ret;
}

void sys_cleanup()
{
	metal_finish();
}

void wait_for_interrupt(void) {
	return;
}
