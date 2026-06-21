[org 0x7c00]                        
KERNEL_LOCATION equ 0x1000

mov [BOOT_DISK], dl                 

xor ax, ax                          
mov es, ax
mov ds, ax
mov bp, 0x8000
mov sp, bp

mov bx, KERNEL_LOCATION

; --- READ 1: Fill up the remaining space on Head 0 ---
mov ah, 0x02
mov al, 17              ; Read 17 sectors total (Sector 2 to 18)
mov ch, 0x00            ; Cylinder 0
mov dh, 0x00            ; Head 0
mov cl, 0x02            ; Start at Sector 2
mov dl, [BOOT_DISK]
int 0x13                
jc .disk_error

; --- READ 2: Switch to Head 1 to safely bypass the real-mode stack memory ---
; Advance memory pointer to avoid overwriting previously loaded sectors
; 17 sectors * 512 bytes = 8704 bytes (0x2200)
; Destination becomes: 0x1000 + 0x2200 = 0x3200
add bx, 0x2200          

mov ah, 0x02
mov al, 30              ; Read 30 more sectors from the opposite side of the disk
mov ch, 0x00            ; Still Cylinder 0
mov dh, 0x01            ; SWITCH TO HEAD 1 (Back side of the disk)
mov cl, 0x01            ; New track reads start back at Sector 1
mov dl, [BOOT_DISK]
int 0x13
jc .disk_error

jmp .boot_continue

.disk_error:
    jmp $               ; Halt here if a hardware disk read fails

.boot_continue:
; --- Your existing video configuration logic continues here ---
mov ah, 0x0
mov al, 0x3
int 0x10                ; Force text mode


CODE_SEG equ GDT_code - GDT_start
DATA_SEG equ GDT_data - GDT_start

cli
lgdt [GDT_descriptor]
mov eax, cr0
or eax, 1
mov cr0, eax
jmp CODE_SEG:start_protected_mode

jmp $
                                    
BOOT_DISK: db 0

GDT_start:
    GDT_null:
        dd 0x0
        dd 0x0

    GDT_code:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10011010
        db 0b11001111
        db 0x0

    GDT_data:
        dw 0xffff
        dw 0x0
        db 0x0
        db 0b10010010
        db 0b11001111
        db 0x0

GDT_end:

GDT_descriptor:
    dw GDT_end - GDT_start - 1
    dd GDT_start

[bits 32]
start_protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    mov ebp, 0x90000        
    mov esp, ebp

    jmp KERNEL_LOCATION

times 510-($-$$) db 0              
dw 0xaa55
