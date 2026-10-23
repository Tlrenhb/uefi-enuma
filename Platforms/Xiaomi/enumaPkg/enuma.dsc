##
#  enumaPkg — Xiaomi Pad 5 Pro 5G (enuma, SM8250-AC) platform description file.
#
#  Structure and the values that carry over: **reAAAq/uefi-umi** Platforms/Xiaomi/umiPkg/umi.dsc
#  (3,980 B) — the only SM8250 port the official device table records as reaching launchd with a
#  persistent Data volume (docs/devices/index.md:23). SoC selection comes from
#  xVanTuring/uefi-pipa Platforms/Xiaomi/pipaPkg/pipa.dsc (SOC_TYPE = 2 for SM8250-AC).
#  Three-way comparison and per-value reasoning: ../TEMPLATES.md
#
#  Copyright (c) 2011 - 2022, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2020, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#  Copyright (c) Microsoft Corporation.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
##

################################################################################
# Defines Section
################################################################################
[Defines]
  PLATFORM_NAME                  = enuma
  # TODO(enuma): newly generated GUID for enumaPkg (umi uses 0AA866E8-88D2-4CCD-910E-E43689D65E5B,
  # pipa 0965E99C-81B3-4F49-9ABF-A7A39F591528). An identifier, not a hardware value.
  PLATFORM_GUID                  = 3F2A6C81-9D47-4B2E-8E15-6A0F7C93D524
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/enumaPkg
  SUPPORTED_ARCHITECTURES        = AARCH64
  # umi.dsc:13 and cas.dsc:10 both build DEBUG only; the official driver default target is DEBUG too.
  BUILD_TARGETS                  = DEBUG
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = enumaPkg/enuma.fdf
  # pipa sets 0 (SimpleFbDxe). umi sets 1 with its own InheritedFbDxe, which is what actually displayed
  # the menu on umi hardware after the vendor DisplayDxe path timed out on DSI completion.
  # TODO(enuma): start at 0. If the panel stays black (umi's symptom: DisplayDxe never returned, DSI
  # completion polling timeout status 0xD, see uefi-umi umiPkg/README.md:19-27), port umi's
  # "inherited framebuffer" GOP driver for enuma's panel and switch this to 1.
  USE_CUSTOM_DISPLAY_DRIVER      = 0

  #
  # 0 = SM8250
  # 1 = SM8250-AB
  # 2 = SM8250-AC
  #
  # enuma is SM8250-AC (Snapdragon 870). umi and cas are plain SM8250 (SOC_TYPE 0); pipa is SM8250-AC
  # and is therefore the source of this value (pipa.dsc:33).
  SOC_TYPE                       = 2

!include KonaPkg/KonaPkg.dsc.inc

################################################################################
# PACKAGES_PATH (set by the caller, NOT in this file)
#
# The unixnu build driver exports PACKAGES_PATH from the device repository's [firmware] packages list;
# official form (docs/porting/uefi-firmware.md:76-90):
#
#   {uefi}:{uefi}/Platforms/Xiaomi:{tree}/package:{tree}/drivers/DarwinBoot:{edk2}:
#   {uefi}/Common/Mu:{uefi}/Common/Mu_OEM_Sample:{uefi}/Silicon/Qualcomm:{uefi}/Silicon/Silicium:
#   {uefi}/Silicium-ACPI:{uefi}/Silicium-ACPI/Platforms/Xiaomi:{uefi}/Silicium-ACPI/Silicon/Qualcomm
#
# {uefi} = the Mu-Silicium/uefi-enuma checkout, {tree} = the assembled build tree, {edk2} = Mu_Basecore.
# Every INF this DSC references must resolve inside that list. Do not set PACKAGES_PATH here — the same
# list must serve the device repository and every other board in the checkout.
#
# umi (hardware-proven) works from the same form; pipa's board.toml carries the identical list.
################################################################################

[PcdsFixedAtBuild]
  #
  # DDR Memory — SM8250 boots with DRAM at 0x80000000 (all three templates agree)
  #
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000

  #
  # UEFI Stack — CONFIRMED by enuma's own uefiplat.cfg (task-20): [MemoryMap] "UEFI Stack",
  #   0x9FF90000, 0x00040000 (drop build BOOT.XF.3.2-00304-SM8250-2; cfg sha256 29affca4...c610).
  #   umi.dsc:28-29 and pipa.dsc:46 agree; the identical row is in MemoryMapLib.c and it does not
  #   overlap "UEFI FD" 0x9FC00000+0x300000 (pipa's check-pipa.py verifies exactly this overlap).
  #
  gArmPlatformTokenSpaceGuid.PcdCPUCoresStackBase|0x9FF90000
  gArmPlatformTokenSpaceGuid.PcdCPUCorePrimaryStackSize|0x40000

  #
  # SMBIOS — TODO(enuma): retail model / SKU / board model from the stock SMBIOS, the box or
  # `fastboot getvar`. umi: Mi 10 / umi / umi-research / umi (umi.dsc:31-35); pipa: Pad 6 / pipa /
  # 23043RP34G / M82.
  #
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemManufacturer|"Xiaomi"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemModel|"TODO(enuma): retail model name"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"enuma"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"TODO(enuma): retail SKU"
  gSiliciumPkgTokenSpaceGuid.PcdSmbiosSystemBoardModel|"TODO(enuma): board model"

  #
  # Simple Frame Buffer — TODO(enuma): enuma is an 11" tablet (Xiaomi Pad 5 Pro family), not a phone.
  # umi: 1080x2340 (umi.dsc:36-38); pipa: 1800x2880. Take enuma's from the stock DT / vendor display
  # driver, not from either template.
  #
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferWidth|0x0
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferHeight|0x0
  gSiliciumPkgTokenSpaceGuid.PcdFrameBufferColorDepth|32

  #
  # Platform PEI — all three templates set "LA"
  #
  gQcomPkgTokenSpaceGuid.PcdPlatformType|"LA"

  #
  # NV variables / capsule policy — copied from umi.dsc:41-49, the configuration that reached launchd.
  # UniXNUDxe writes standard boot variables, so their backend must stay in RAM; the capsule paths stay
  # off (cas does the same with PcdEmuVariableNvModeEnable|TRUE).
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdCapsuleInRamSupport|FALSE
  gUniXNUPkgTokenSpaceGuid.PcdUniXNUDebugLevel|0x80000000

[PcdsFeatureFlag]
  # umi.dsc:47-49 — the missing Capsule Arch is what stopped umi's DXE before BDS in an early attempt.
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportProcessCapsuleAtRuntime|FALSE

[LibraryClasses]
  #
  # Memory / configuration tables — FILLED (task-20) from this device's own firmware:
  #   uefiplat.cfg (drop build BOOT.XF.3.2-00304-SM8250-2, sha256 29affca4...c610)
  #   -> MemoryMapLib.c: 45 rows ([MemoryMap] 22 + [RegisterMap] 23), verified row-for-row against the cfg
  #   -> ConfigurationMapLib.c: 32 numeric [ConfigParameters] entries (the 5 string parameters are kept as
  #      comments; the section declares ConfigParameterCount = 64).
  # Signatures are shared by all three templates.
  #
  MemoryMapLib|enumaPkg/Library/MemoryMapLib/MemoryMapLib.inf
  ConfigurationMapLib|enumaPkg/Library/ConfigurationMapLib/ConfigurationMapLib.inf

  #
  # Serial / log. pipa's RamLogLib is already in this package (copied verbatim from uefi-pipa).
  # TODO(enuma): umi additionally redirects the EDK II console itself with
  #   SerialPortLib|umiPkg/Library/RamoopsSerialPortLib/RamoopsSerialPortLib.inf
  # which is how its ramoops log was recovered after an ASSERT. Port that library for enuma if the
  # console is needed (different purpose from RamLogLib, which serves the debug-mask log path).
  #
  RamLogLib|enumaPkg/Library/RamoopsLogLib/RamoopsLogLib.inf

  #
  # Null platform libraries — copied from umi.dsc:55-62. UniXNU owns BDS/ACPI/RamManager here, and the
  # null implementations are what the hardware-proven configuration uses.
  #
  DeviceBootManagerLib|SiliciumNullPkg/Library/PlatformBootManagerLibNull/PlatformBootManagerLibNull.inf
  PlatformBootOptionsLib|SiliciumNullPkg/Library/PlatformBootOptionsLibNull/PlatformBootOptionsLibNull.inf
  AcpiTableUpdateLib|SiliciumNullPkg/Library/AcpiTableUpdateLibNull/AcpiTableUpdateLibNull.inf
  RamManagerLib|SiliciumNullPkg/Library/RamManagerLibNull/RamManagerLibNull.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf

  #
  # UniXNU libraries
  #
  UniXNULib|UniXNUPkg/Library/UniXNULib/UniXNULib.inf
  FdtLib|MdePkg/Library/BaseFdtLib/BaseFdtLib.inf

  #
  # TODO(enuma): umi also provides SecureBootHandlerLib via its own
  # NullSecureBootHandlerLib and ResetSystemLib/HwResetSystemLib via UmiResetSystemLib, plus
  # UmiSpmiLib/UmiRpmhLib for PMIC and RPMH access. Those are board-specific implementations; enuma
  # needs its own or must fall back to an upstream library. Decide when the button/reset path is wired
  # (the stock volume keys must reach the UniXNU menu).

[Components]
  #
  # UniXNU: the boot manager, its registrar, the ramdisk block device and the XNU loader.
  # ShengBootMemoryDxe and DarwinBoot come from the uefi-pipa fork; the memory driver reads the device
  # tree the previous stage passes and is board-independent.
  #
  UniXNUPkg/Application/UniXNU/UniXNU.inf
  UniXNUPkg/Driver/UniXNUDxe/UniXNUDxe.inf
  shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf
  DarwinBootPkg/Driver/DarwinBoot.inf

  #
  # Display: with USE_CUSTOM_DISPLAY_DRIVER = 0 the FDF selects SiliciumPkg/Drivers/SimpleFbDxe
  # (pipa's choice). If enuma needs the inherited-framebuffer route, add an enuma-specific GOP driver
  # here, mirroring umiPkg/Drivers/InheritedFbDxe/InheritedFbDxe.inf.
  #

  #
  # TODO(enuma): ACPI. pipaPkg references `pipa/AcpiTables.inf`; the tables in enuma's case come with
  # the vendor inputs. Add `enuma/AcpiTables.inf` only when the dump provides them.
  # enuma/AcpiTables.inf