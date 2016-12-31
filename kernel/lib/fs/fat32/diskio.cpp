/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <string.h>
#include <lib/kernel/vfs/VirtualFileSystem.h>
#include <fcntl.h>
#include <platform/kprintf.h>
#include "diskio.h"		/* FatFs lower layer API */
#include "ffconf.h"

/* Definitions of physical drive number for each drive */
#define DEV_PFLASH        0    /* Example: Map PFlash to physical drive 0 */

std::unique_ptr<FileDescriptor> _fd;
/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(
        BYTE pdrv        /* Physical drive nmuber to identify the drive */
) {
    switch (pdrv) {
        case DEV_PFLASH:
            if (nullptr == _fd) {
                return STA_NOINIT;
            }

            return STA_PROTECT;

        default:
            break;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
        BYTE pdrv                /* Physical drive number to identify the drive */
) {
    switch (pdrv) {
        case DEV_PFLASH:
            if (nullptr != _fd) {
                return STA_PROTECT;
            }

            _fd = VirtualFileSystem::instance().open("/pflash/userdata", O_RDONLY);
            if (nullptr != _fd) {
                return STA_PROTECT;
            }

            break;

        default:
            break;
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(
        BYTE pdrv,        /* Physical drive nmuber to identify the drive */
        BYTE *buff,        /* Data buffer to store read data */
        DWORD sector,    /* Start sector in LBA */
        UINT count        /* Number of sectors to read */
) {
    off_t offset;
    size_t size;
    switch (pdrv) {
        case DEV_PFLASH:
            // translate the arguments here
            if (nullptr == _fd) {
                return RES_NOTRDY;
            }

            offset = sector * _MIN_SS;
            if (offset != _fd->seek(offset, SEEK_SET)) {
                return RES_ERROR;
            }

            size = count * _MIN_SS;
            if (size != (size_t)_fd->read(buff, size)) {
                return RES_ERROR;
            }

            return RES_OK;

        default:
            break;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write(
        BYTE pdrv,            /* Physical drive nmuber to identify the drive */
        const BYTE *buff,    /* Data to be written */
        DWORD sector,        /* Start sector in LBA */
        UINT count            /* Number of sectors to write */
) {
    off_t offset;
    size_t size;

    switch (pdrv) {
        case DEV_PFLASH:
            // translate the arguments here
            if (nullptr == _fd) {
                return RES_NOTRDY;
            }

            offset = sector * _MIN_SS;
            if (offset != _fd->seek(offset, SEEK_SET)) {
                return RES_ERROR;
            }

            size = count * _MIN_SS;
            if (size != (size_t)_fd->write(buff, size)) {
                return RES_ERROR;
            }

            return RES_OK;

        default:
            break;
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(
        BYTE pdrv,        /* Physical drive nmuber (0..) */
        BYTE cmd,        /* Control code */
        void *buff        /* Buffer to send/receive control data */
) {
    kprintf("disk_ioctl: pdrv %d cmd %d buf %p\n", pdrv, cmd, buff);
    switch (pdrv) {
        case DEV_PFLASH:
            return RES_OK;

        default:
            break;
    }

    return RES_PARERR;
}

