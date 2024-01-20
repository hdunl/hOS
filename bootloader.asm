section .text
global start
bits 16

start:
    jmp short skip_bpb
    nop

    OEMLabel          db "hOS    "
    BytesPerSector    dw 512
    SectorsPerCluster db 1
    ReservedForBoot   dw 1
    NumberOfFats      db 2
    RootDirEntries    dw 224
    TotalSectorsShort dw 2880
    MediaDescriptor   db 0xF0
    SectorsPerFat     dw 9
    SectorsPerTrack   dw 18
    NumberOfHeads     dw 2
    HiddenSectors     dd 0
    TotalSectorsLong  dd 0

skip_bpb:
    cli
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x8000
    sti

    call enableA20

    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:init_pm

enableA20:
    call keyboard_wait
    mov al, 0xAD
    out 0x64, al

    call keyboard_wait
    mov al, 0xD0
    out 0x64, al

    call keyboard_wait
    in al, 0x60
    push ax

    call keyboard_wait
    mov al, 0xD1
    out 0x64, al

    call keyboard_wait
    pop ax
    or al, 2
    out 0x60, al

    call keyboard_wait
    mov al, 0xAE
    out 0x64, al

    call keyboard_wait
    ret

keyboard_wait:
    wait_loop:
        in al, 0x64
        test al, 2
        jnz wait_loop
    ret

[BITS 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x80000

    jmp 0x08:0x100000

gdt_start:
    dq 0
code_seg:
    dw 0xFFFF
    dw 0
    db 0
    db 10011010b
    db 11001111b
    db 0
data_seg:
    dw 0xFFFF
    dw 0
    db 0
    db 10010010b
    db 11001111b
    db 0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_start + code_seg - gdt_start
DATA_SEG equ gdt_start + data_seg - gdt_start

times 510-($-$$) db 0
dw 0xAA55
