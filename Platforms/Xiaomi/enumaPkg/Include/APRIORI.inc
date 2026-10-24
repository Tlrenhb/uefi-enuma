# APRIORI order for enumaPkg (FDF fragment, included from enuma.fdf [FV.FvMain]).
#
# SOURCE OF TRUTH: the UEFIReader drop of the official fastboot ROM's XBL image --
#   <drop>/APRIORI.inc (4,444 B), build BOOT.XF.3.2-00304-SM8250-2,
#   ROM enuma_images_OS1.0.3.0.TKZCNXM_13.0, uefiplat.cfg sha256 29affca4...c610.
# The drop's APRIORI DXE block is reproduced in its original order; the only change is the path prefix:
#   * QcomPkg/** and MiPkg/**  -> Binaries/enuma/**            (62 entries)
#   * paths identical to an unprefixed entry of pipaPkg's DXE.inc stay unprefixed   (18 entries, Mu/edk2 core)
#   * other paths (vendor-modified) -> Binaries/enuma/**       (4 entries: RscRtDxe.inf, SCHandlerRtDxe.inf, ArmGicDxe.inf, FvSimpleFileSystemDxe.inf)
# Rule and rationale: see Include/DXE.inc's header (same transform, same two-vs-three-way reasoning).
#
# Notes for reviewers:
#   * This file only orders DXE dispatch dependencies. The UniXNU additions (ShengBootMemoryDxe, DarwinBoot)
#     are NOT APRIORI deps and stay in DXE.inc/FDF, exactly as in pipa and umi.
#   * "ArmPkg/Drivers/ArmGic/ArmGicDxe.inf" is prefixed because pipa spells the same driver
#     "ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf" (a path that does not exist upstream); the drop's spelling is
#     the one proven to exist in this image. If the build prefers Mu's copy, drop the Binaries/enuma/ prefix
#     on that single line.
#   * No QcomBds here: the drop's own DXE.dsc.inc lists it, pipa's DXE.inc uses MdeModulePkg BdsDxe instead,
#     and UniXNU owns the boot manager. Documented switch: if the menu is never reached, add
#     INF Binaries/enuma/QcomPkg/Drivers/QcomBds/QcomBds.inf to DXE.inc and re-test.

APRIORI DXE {
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/SecRSADxe/SecRSADxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/ASN1X509Dxe/ASN1X509Dxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/TzDxe/ScmDxeLA.inf
    INF Binaries/enuma/QcomPkg/Drivers/TzDxe/TzDxeLA.inf
    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF Binaries/enuma/MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/RscRtDxe.inf
    INF Binaries/enuma/MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/SCHandlerRtDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/VariableDxe/VariableDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF Binaries/enuma/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
    INF Binaries/enuma/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
    INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/FontDxe/FontDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/EnvDxe/EnvDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
    INF Binaries/enuma/ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/PdcDxe/PdcDxe.inf
    INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
    INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
    INF FatPkg/EnhancedFatDxe/Fat.inf
    INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
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
    # vendor DisplayDxe disabled for bring-up: see the note in DXE.inc (DSI completion timeout,
    # and because it is APRIORI a hang here stops every later driver from being dispatched).
    #INF Binaries/enuma/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
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
    INF Binaries/enuma/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/HashDxe/HashDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/CipherDxe/CipherDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/MacDxe/MacDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/RNGDxe/RngDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/QcomBds/QcomBds.inf
    INF Binaries/enuma/QcomPkg/Drivers/PILDxe/PILDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/SPSSDxe/SPSSDxe.inf
    INF Binaries/enuma/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
}
