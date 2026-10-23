# DXE driver list for enumaPkg (FDF fragment, included from enuma.fdf [FV.FvMain]).
#
# SOURCE OF TRUTH: the UEFIReader drop of the OFFICIAL fastboot ROM's XBL image (not a tablet dump):
#   /tmp/fw2/uefi-out/xbl.elf/BOOT.XF.3.2-00304-SM8250-2/DXE.inc   (13,794 B, build BOOT.XF.3.2-00304-SM8250-2)
#   ROM: enuma_images_OS1.0.3.0.TKZCNXM_13.0
#   uefiplat.cfg sha256 = 29affca455090a26b223ebae2c02bf294547bb699f9b65a891af3c186d99c610
#
# TRANSFORM (mechanical, reproducible; 91 INF lines in the drop -> 90 here + 2 UniXNU lines = 92):
#   1. QcomPkg/** and MiPkg/** -> prefix "Binaries/enuma/" (68 lines) -- the vendor binaries that
#      scripts/extract-vendor-dxe.sh places there;
#   2. paths IDENTICAL to an unprefixed entry of pipaPkg's DXE.inc (Project-Silicium/Mu-Silicium main)
#      stay unprefixed (19 lines) -- Mu/edk2 core modules that come from Mu_Basecore, not the image;
#   3. other paths -> also prefixed (4 lines):  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/RscRtDxe.inf, MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/SCHandlerRtDxe.inf, ArmPkg/Drivers/ArmGic/ArmGicDxe.inf, MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
#   4. QcomPkg/XBLCore/XBLCore.inf is EXCLUDED (1 line): it is the vendor XBL core, replaced in this port
#      by BootShim + SiliciumPkg/Sec; including it would collide with the SEC stage;
#   5. the 56 FILE FREEFORM blocks of the same drop file were split into Include/RAW.inc (the umi/pipa
#      three-fragment layout -- enuma.fdf includes both files, so leaving them here would duplicate them).
#
# UniXNU additions are kept after the core modules (the proven umi.fdf:42-43 carries the same two):
#   ShengBootMemoryDxe (payload ramdisk as a read-only block device) and
#   DarwinBoot (XNU loader inside the firmware volume -- verification refuses one loaded from a volume).
#

  # --- Mu/edk2 core modules: from the base tree, unprefixed (rule 2) ---
  INF MdeModulePkg/Core/Dxe/DxeMain.inf
  INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  INF FatPkg/EnhancedFatDxe/Fat.inf
  INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  # --- UniXNU additions ---
  INF shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf
  INF DarwinBootPkg/Driver/DarwinBoot.inf

  # --- vendor / vendor-modified modules: Binaries/enuma/ (rules 1 and 3) ---
  INF Binaries/enuma/QcomPkg/Drivers/SecRSADxe/SecRSADxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/ASN1X509Dxe/ASN1X509Dxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
  INF Binaries/enuma/MiPkg/Protocol/MiToken/MiToken.inf
  INF Binaries/enuma/QcomPkg/Drivers/TzDxe/ScmDxeLA.inf
  INF Binaries/enuma/QcomPkg/Drivers/TzDxe/TzDxeLA.inf
  INF Binaries/enuma/MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/RscRtDxe.inf
  INF Binaries/enuma/MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/SCHandlerRtDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/VariableDxe/VariableDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  INF Binaries/enuma/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
  INF Binaries/enuma/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/FontDxe/FontDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/EnvDxe/EnvDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
  INF Binaries/enuma/ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/PdcDxe/PdcDxe.inf
  INF Binaries/enuma/MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/ClockDxe/ClockDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/VcsDxe/VcsDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UFSDxe/UFSDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/FeatureEnablerDxe/FeatureEnablerDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/VibratorDxe/VibratorDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf
  INF Binaries/enuma/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/AdcDxe/AdcDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/TsensDxe/TsensDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/XhciDxe/XhciDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/HashDxe/HashDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/CipherDxe/CipherDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/MacDxe/MacDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/RNGDxe/RngDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/QcomBds/QcomBds.inf
  INF Binaries/enuma/QcomPkg/Drivers/PILDxe/PILDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
  INF Binaries/enuma/QcomPkg/Drivers/SPSSDxe/SPSSDxe.inf
  INF Binaries/enuma/QcomPkg/Application/QcomChargerApp/QcomChargerApp.inf
  INF Binaries/enuma/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
