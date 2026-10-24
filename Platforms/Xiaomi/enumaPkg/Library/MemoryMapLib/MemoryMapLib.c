#include <Library/MemoryMapLib.h>

//
// enumaPkg memory map: the [MemoryMap] and [RegisterMap] sections of THIS DEVICE's own uefiplat.cfg,
// row for row, in file order. Nothing here is copied from another board.
//
// Source: the UEFIReader drop of the official fastboot ROM's XBL image --
//   ROM   : enuma_images_OS1.0.3.0.TKZCNXM_13.0 (Xiaomi Pad 5 Pro 5G, enuma, SM8250-AC)
//   image : xbl.elf, build BOOT.XF.3.2-00304-SM8250-2
//   file  : <drop>/RawFiles/uefiplat.cfg
//   sha256: 29affca455090a26b223ebae2c02bf294547bb699f9b65a891af3c186d99c610   (7,953 bytes)
//   rows  : [MemoryMap] 22 + [RegisterMap] 23 = 45 entries
//
// The same cfg also carries, outside these two sections, three [Config] keys that are NOT table rows and
// are therefore not repeated here (they are enforced by the Core/PEI side):
//   UnusableDDRMemoryAtBeginning = 1, UnusableDDRMemoryStartAddr = 0x80000000,
//   UnusableDDRMemorySizeAtBeginning = 0x600000, MaxMemoryRegions = 74.
//
// What differs from pipaPkg's table of the same SoC (do not "fix" one to match the other -- these are
// per-image facts):
//   * pipa has a "HYP" row at 0x80000000+0x00600000; enuma's cfg expresses that window as the
//     UnusableDDRMemory* keys above, so there is no row for it here;
//   * pipa 0x80880000+0x00010000 is "GPU PRR"; enuma's cfg has "XBL Log Buffer" 0x80884000+0x00010000;
//   * pipa "PIL Reserved" 0x86200000+0x0C500000 vs enuma 0x86000000+0x0D200000 (and enuma has MPSS EFS,
//     BOOT INFO, Sched Heap, DBI Dump, FV Region, ABOOT FV, SEC Heap, CPU Vectors, MMU PageTables,
//     Log Buffer and Kernel rows that pipa does not);
//   * the three rows the rest of the package depends on are identical in both: "Display Reserved"
//     0x9C000000+0x02400000, "UEFI FD" 0x9FC00000+0x00300000 (matches Resources/Configs/enuma.toml
//     [uefi_fd] and the BootShim FD_BASE) and "UEFI Stack" 0x9FF90000+0x00040000 (matches enuma.dsc's
//     PcdCPUCoresStackBase / PcdCPUCorePrimaryStackSize).
//

STATIC
EFI_MEMORY_REGION_DESCRIPTOR
gMemoryDescriptor[] = {
  // Name, Address, Length, HobOption, ResourceType, ResourceAttribute, MemoryType, ArmAttribute

  // ---- DDR ----
  {"IPC SHM",            0x805d0000,      0x20000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"MPSS EFS",           0x80600000,     0x100000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"BOOT INFO",          0x80700000,     0x100000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"AOP CMD DB",         0x80860000,      0x20000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"XBL Log Buffer",     0x80884000,      0x10000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"SMEM",               0x80900000,     0x200000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"TZApps Reserved",    0x82400000,    0x3a00000, HobOnlyNoCacheSetting, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"PIL Reserved",       0x86000000,    0xd200000, AddMem, MEM_RES, UNCACHEABLE, Reserv, UNCACHED_UNBUFFERED_XN},
  {"DXE Heap",           0x98900000,    0x3300000, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN},
  {"Sched Heap",         0x9bc00000,     0x400000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"Display Reserved",   0x9c000000,    0x2400000, AddMem, MEM_RES, SYS_MEM_CAP, Reserv, WRITE_THROUGH_XN},  // umi's hardware-proven table uses WRITE_THROUGH_XN for its display window.
  // An earlier revision mapped this UNCACHED (Device memory): the early console's writes are not
  // guaranteed aligned, device memory forbids unaligned access, and the vector table is installed only
  // after the MMU is on - so the alignment fault became a silent hang.
  {"DBI Dump",           0x9e400000,     0xf00000, NoHob, MMAP_IO, INITIALIZED, Conv, UNCACHED_UNBUFFERED_XN},
  {"FV Region",          0x9f800000,     0x200000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"ABOOT FV",           0x9fa00000,     0x200000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
  {"UEFI FD",            0x9fc00000,     0x300000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
  {"SEC Heap",           0x9ff00000,      0x8c000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"CPU Vectors",        0x9ff8c000,       0x1000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK},
  {"MMU PageTables",     0x9ff8d000,       0x3000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"UEFI Stack",         0x9ff90000,      0x40000, AddMem, SYS_MEM, SYS_MEM_CAP, BsData, WRITE_BACK_XN},
  {"Log Buffer",         0x9fff7000,       0x8000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
  {"Info Blk",           0x9ffff000,       0x1000, AddMem, SYS_MEM, SYS_MEM_CAP, RtData, WRITE_BACK_XN},
  {"Kernel",             0xa0000000,   0x10000000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},

  // ---- Other / Register ----
  {"IMEM Base",          0x14680000,      0x40000, NoHob, MMAP_IO, INITIALIZED, Conv, NS_DEVICE},
  {"IMEM Cookie Base",   0x146bf000,       0x1000, AddDev, MMAP_IO, INITIALIZED, Conv, NS_DEVICE},
  {"IPC_ROUTER_TOP",       0x400000,     0x100000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SECURITY CONTROL",     0x780000,       0x7000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QUPV3_2_GSI",          0x800000,      0xd0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QUPV3_0_GSI",          0x900000,      0xd0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QUPV3_1_GSI",          0xa00000,      0xd0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"PRNG_CFG_PRNG",        0x790000,      0x10000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"CRYPTO0 CRYPTO",      0x1dc0000,      0x40000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TCSR_TCSR_REGS",      0x1fc0000,      0x30000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"GPU_GMU_CX_BLK",      0x2c7d000,       0x2000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"GPU_CC",              0x2c90000,       0xa000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"QUPV3_SSC_GSI",       0x5a00000,      0xd0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"PERIPH_SS",           0x8800000,     0x200000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"USB30_PRIM",          0xa600000,     0x11b000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"USB_RUMI",            0xa720000,      0x10000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"USB30_SEC",           0xa800000,     0x11b000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"AOSS",                0xb000000,    0x4000000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_WEST",           0xf100000,     0x300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_SOUTH",          0xf500000,     0x300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"TLMM_NORTH",          0xf900000,     0x300000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"SMMU",               0x15000000,      0xd0000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
  {"APSS_HM",            0x17800000,    0xd981000, AddDev, MMAP_IO, UNCACHEABLE, MmIO, NS_DEVICE},
};

VOID
GetMemoryMap (
  OUT EFI_MEMORY_REGION_DESCRIPTOR **MemoryDescriptor,
  OUT UINT8                         *MemoryDescriptorCount)
{
  // Pass Data
  *MemoryDescriptor      = gMemoryDescriptor;
  *MemoryDescriptorCount = ARRAY_SIZE (gMemoryDescriptor);
}
