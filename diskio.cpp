#include "ff.h"       // MUST come first to define BYTE, UINT, LBA_t
#include "diskio.h"   // Comes second

typedef unsigned char u8;
typedef unsigned int u32;
// ... keep the rest of the code exactly the same ...


// Tell the compiler to look for the driver function we wrote in ata.cpp
extern "C" int ata_read_sector(u32 lba, u8* buffer);

extern "C" {

    // 1. Get Drive Status
    DSTATUS disk_status(BYTE pdrv) {
        if (pdrv != 0) return STA_NOINIT; // Changed from STA_NODRIVE
        return 0; 
    }

    // 2. Initialize a Drive
    DSTATUS disk_initialize(BYTE pdrv) {
        if (pdrv != 0) return STA_NOINIT; // Changed from STA_NODRIVE
        return 0; 
    }


    // 3. Read Sectors from Disk
    DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
        if (pdrv != 0) return RES_PARERR;

        // Read the requested number of sectors one by one
        for (UINT i = 0; i < count; i++) {
            if (!ata_read_sector(sector + i, buff + (i * 512))) {
                return RES_ERROR; // Fail if hardware read fails
            }
        }
        return RES_OK;
    }

    // 4. Write Sectors to Disk (Stabbed out for now)
    DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
        if (pdrv != 0) return RES_PARERR;
        // We will focus on reading files safely first before writing data
        return RES_OK;
    }

    // 5. Miscellaneous Control Functions
    DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
        if (pdrv != 0) return RES_PARERR;

        // FatFs needs to know basic features about the size of the disk layout
        switch (cmd) {
            case CTRL_SYNC:
                return RES_OK;
            case GET_SECTOR_SIZE:
                *(WORD*)buff = 512;
                return RES_OK;
            case GET_BLOCK_SIZE:
                *(DWORD*)buff = 1; // 1 sector per erase block
                return RES_OK;
            default:
                return RES_PARERR;
        }
    }
}
