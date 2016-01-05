#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#include <linux/ioctl.h>
#include <linux/types.h>

#define	WATCHDOG_IOCTL_BASE	'W'

#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE, 6, int)

void WdtServiceStart();
void WdtServiceStop();

#endif  // __WATCHDOG_H__

