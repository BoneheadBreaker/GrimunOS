#include <stdint.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

// Use the port functions you already written
extern "C" {
    u8 inb(u16 port);
    void outb(u16 port, u8 v);
}

// Read a 16-bit word from an x86 port
static inline u16 inw(u16 port) {
    u16 r;
    __asm__ volatile("inw %1, %0" : "=a"(r) : "Nd"(port));
    return r;
}

// Basic ATA PIO Driver to read an individual 512-byte sector
extern "C" int ata_read_sector(u32 lba, u8* buffer) {
    // Select master drive and top 4 bits of the LBA address
    outb(0x1F6, (lba >> 24) | 0xE0);
    
    // Send sector count (1 sector at a time)
    outb(0x1F2, 1);
    
    // Send the rest of the LBA address across ports
    outb(0x1F3, (u8)lba);
    outb(0x1F4, (u8)(lba >> 8));
    outb(0x1F5, (u8)(lba >> 16));
    
    // Command 0x20: Read Sectors with Retry
    outb(0x1F7, 0x20);

    // Wait for the drive to be ready (BSY flag cleared, DRQ flag set)
    u8 status;
    while (1) {
        status = inb(0x1F7);
        if ((status & 0x80) == 0 && (status & 0x08) != 0) {
            break; // Drive is ready to give us data
        }
        if (status & 0x01) {
            return 0; // Error occurred on drive read
        }
    }

    // Transfer 256 words (512 bytes) from hard drive data port (0x1F0)
    u16* ptr = (u16*)buffer;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(0x1F0);
    }

    return 1; // Success
}
