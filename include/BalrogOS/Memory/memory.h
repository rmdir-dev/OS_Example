#pragma once

/*
ADDRESSES
*/
#define KERNEL_OFFSET 0xFFFFFF8000000000

#include <stdint.h>

/*
addr = 0x101000
0x101000 & ~0xFFFFFF8000000000
0x101000 & 7FFFFFFFFF
= 0x101000
*/
#define VIRTUAL_TO_PHYSICAL(addr) (uintptr_t)addr & ~KERNEL_OFFSET

/*
addr = 0x101000
0x101000 | 0xFFFFFF8000000000
= 0xFFFFFF8000101000
*/
#define PHYSICAL_TO_VIRTUAL(addr) (uintptr_t)addr | KERNEL_OFFSET

/*
----------------------------------------------------------------------------------
            PAGING
----------------------------------------------------------------------------------
*/
#define PML4T_OFFSET(addr)  ((((uintptr_t)(addr)) >> 39) & 0x1ff)
#define PDPT_OFFSET(addr)   ((((uintptr_t)(addr)) >> 30) & 0x1ff)
#define PDT_OFFSET(addr)    ((((uintptr_t)(addr)) >> 21) & 0x1ff)
#define PT_OFFSET(addr)     ((((uintptr_t)(addr)) >> 12) & 0x1ff)

#define PAGE_FLAG_MASK      0xfff

#define   PAGE_PRESENT        0x001
#define   PAGE_WRITE          0x002
#define   PAGE_USER           0x004
#define   PAGE_WRITETHROUGH   0x008
#define   PAGE_NOCACHE        0x010
#define   PAGE_ACCESSED       0x020
#define   PAGE_DIRTY          0x040
#define   PAGE_HUGE           0x080
#define   PAGE_GLOBAL         0x100 

/*
----------------------------------------------------------------------------------
            CONVERSION
----------------------------------------------------------------------------------
*/

#define ONE_MiB 1048576
#define ONE_kiB 1024

#define BYTE_TO_MiB(bytes)  (bytes / ONE_MiB)
#define BYTE_TO_KiB(bytes)  (bytes / ONE_kiB)

#define USABLE_MEMORY               1
#define RESERVED_MEMORY             2
#define ACPI_RECLAIMEBLE_MEMORY     3
#define ACPI_NVS_MEMORY             4
#define AREA_BAD_MEMORY             5

/*
1 page = 4096 bytes
*/
#define PAGE_SIZE 4096

typedef struct SMAP_entry_st
{
    /*
        Base address 
        The begining of the memory map
    */
    uint64_t BaseAddress;

    /*
        Length in Byte
    */
    uint64_t Length;

    /*
    Types values :
        1 Usable memory
        2 Reserved memory
        3 ACPI reclaimed memory
        4 ACPI NVS Memory
        5 Aera contining bad memory
    */
    uint32_t Type;

    uint32_t ACPI;
} __attribute__((packed)) SMAP_entry;