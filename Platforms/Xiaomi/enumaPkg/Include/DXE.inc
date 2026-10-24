# DXE module list for enumaPkg.
#
# SHAPE: the minimal, source-built list that the only hardware-proven SM8250 port (uefi-umi,
# reAAAq/uefi-umi, Mi 10) actually boots with -- 31 modules, NONE of them from Binaries/<device>.
# We shipped pipa's full vendor list first (91 lines, 65 of them vendor DXE from Binaries/enuma) and the
# tablet stopped dead right after SEC's "MMU: HOBs built, enabling": the vendor drivers expect a complete
# stock ABL environment (TZ applets, DAL, IMEM) and any one of them blocking on hardware stalls the
# single-threaded DXE dispatcher, after which BDS never runs. pipa never reached hardware, so that list
# was never validated.
#
# The vendor drivers are all still in Binaries/enuma/ (and in Include/RAW.inc as data); re-add them one at
# a time, with a reason and a test, only when a later stage needs them (storage, USB, buttons, charging).
#
# Display: the panel is lit by ABL and the early console draws into the inherited "Display Reserved"
# window; SimpleFbDxe publishes exactly that window as the GOP (SimpleFb.c:100-121) with the measured
# geometry from enuma.dsc (1600x2560). The vendor DisplayDxe is deliberately NOT here: per uefi-umi's
# README it re-initialises DSI and times out on panel completion (poll status 0xD).

  # --- Mu/edk2 core (paths and spellings follow uefi-umi's DXE.inc, which builds against the same Mu_Basecore) ---
  INF MdeModulePkg/Core/Dxe/DxeMain.inf
  INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
  INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  INF MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  INF MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  INF ArmPkg/Drivers/ArmGicDxe/ArmGicDxe.inf
  INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

  # --- display: the inherited framebuffer as the GOP (umi uses UmiInheritedFbDxe for the same job) ---
  INF SiliciumPkg/Drivers/SimpleFbDxe/SimpleFbDxe.inf

  # --- storage and filesystem: needed by the payload volume on later stages, harmless now ---
  INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  INF FatPkg/EnhancedFatDxe/Fat.inf

  # --- UniXNU: the payload block device, the XNU loader, the boot manager registrar (umi.fdf:42-43) ---
  INF shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf
  INF DarwinBootPkg/Driver/DarwinBoot.inf
  INF UniXNUPkg/Driver/UniXNUDxe/UniXNUDxe.inf
