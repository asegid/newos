/*
** Copyright 2001-2002, Graham Batty. All rights reserved.
** Distributed under the terms of the NewOS License.
*/
#include <kernel/kernel.h>
#include <boot/stage2.h>
#include <kernel/debug.h>
#include <kernel/heap.h>
#include <kernel/fs/devfs.h>
#include <string.h>
#include <newos/errors.h>
#include <kernel/net/ethernet.h>
#include <kernel/dev/arch/i386/pcnet32/pcnet32_dev.h>

#include "pcnet32_priv.h"

#define debug_level_flow 3
#define debug_level_error 3
#define debug_level_info 3

#define DEBUG_MSG_PREFIX "PCNET -- "

#include <kernel/debug_ext.h>

static int pcnet32_open(dev_ident ident, dev_cookie *cookie)
{
	pcnet32 *nic = (pcnet32 *)ident;

	*cookie = nic;

	return 0;
}

static int pcnet32_freecookie(dev_cookie cookie)
{
	return 0;
}

static int pcnet32_seek(dev_cookie cookie, off_t pos, seek_type st)
{
	return ERR_NOT_ALLOWED;
}

static int pcnet32_close(dev_cookie cookie)
{
	return 0;
}

static ssize_t pcnet32_read(dev_cookie cookie, void *buf, off_t pos, ssize_t len)
{
	pcnet32 *nic = (pcnet32 *)cookie;

	if(len < ETHERNET_MAX_SIZE)
		return ERR_VFS_INSUFFICIENT_BUF;
	return pcnet32_rx(nic, buf, len);
}

static ssize_t pcnet32_write(dev_cookie cookie, const void *buf, off_t pos, ssize_t len)
{
	pcnet32 *nic = (pcnet32 *)cookie;

	if(len < 0)
		return ERR_INVALID_ARGS;

	return pcnet32_xmit(nic, buf, len);
}

static int pcnet32_ioctl(dev_cookie cookie, int op, void *buf, size_t len)
{
	pcnet32 *nic = (pcnet32 *)cookie;
	int err;

	SHOW_FLOW(3, "op %d, buf %p, len %Ld\n", op, buf, (long long)len);

	if(!nic)
		return ERR_IO_ERROR;

	switch(op) {
		case 10000: // get the ethernet MAC address
			if(len >= sizeof(nic->mac_addr)) {
				memcpy(buf, nic->mac_addr, sizeof(nic->mac_addr));
				err = 0;
			} else {
				err = ERR_VFS_INSUFFICIENT_BUF;
			}
			break;
		default:
			err = ERR_INVALID_ARGS;
	}

	return err;
}

static struct dev_calls pcnet32_hooks = {
	&pcnet32_open,
	&pcnet32_close,
	&pcnet32_freecookie,
	&pcnet32_seek,
	&pcnet32_ioctl,
	&pcnet32_read,
	&pcnet32_write,
	/* no paging here */
	NULL,
	NULL,
	NULL
};

int pcnet32_dev_init(kernel_args *ka)
{
	pcnet32 *nic;

	SHOW_FLOW0(3, "entry");

	nic = pcnet32_new(
		PCNET_INIT_MODE0 | PCNET_INIT_RXLEN_128 | PCNET_INIT_TXLEN_32,
		2048, 2048);

	if (nic == NULL)
	{
		SHOW_FLOW0(3, "pcnet_new returned 0.");
		return 0;
	}

	if (pcnet32_detect(&nic) > -1)
	{
		if (pcnet32_init(nic) < 0)
		{
			SHOW_FLOW0(3, "pcnet_init failed.");
			
			pcnet32_delete(nic);
			return 0;
		}

		pcnet32_start(nic);

		if (devfs_publish_device("net/pcnet32/0", nic, &pcnet32_hooks) < 0)
		{
			SHOW_FLOW0(3, "failed to register device /dev/net/pcnet32/0");
			return 0;
		}
	}

	return 0;
}
