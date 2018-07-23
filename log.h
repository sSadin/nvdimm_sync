#ifndef _LOG_H_
#define _LOG_H_

#include <linux/module.h> 


#define LOG_INFO 5
#define DEBUG
#define DRV_RDMA KBUILD_MODNAME "_rdma: "

#ifndef LOG_LEVEL
#ifdef DEBUG
#define LOG_LEVEL 10
#else // !DEBUG
#define LOG_LEVEL 1
#endif // DEBUG
#endif // LOG_LEVEL

#define LOG_KERN(LEVEL, STR) do {\
        if ((LEVEL) < LOG_LEVEL) {\
            printk(KERN_WARNING "%-30s | %3d |  ",__FUNCTION__,__LINE__); \
            printk(DRV_RDMA); \
            printk STR; \
        } \
} while(0);

#endif // _LOG_H_

