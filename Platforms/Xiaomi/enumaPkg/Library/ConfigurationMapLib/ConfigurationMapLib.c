#include <Library/ConfigurationMapLib.h>

//
// enumaPkg configuration map: every numeric entry of the [ConfigParameters] section of THIS DEVICE's own
// uefiplat.cfg, in file order. Same source and sha256 as MemoryMapLib.c:
//   <drop>/RawFiles/uefiplat.cfg, sha256 29affca455090a26b223ebae2c02bf294547bb699f9b65a891af3c186d99c610
//   the section declares ConfigParameterCount = 64; this table carries the 32 entries that are numeric
//   (the file also has 5 string parameters, listed below as comments -- EFI_CONFIGURATION_ENTRY_DESCRIPTOR
//   holds a UINT32, so strings cannot be expressed and are NOT dropped silently).
//
// pipaPkg divergence to be aware of (do not copy it): pipa writes {"EnableUfsIOC", 1} while enuma's cfg
// says EnableUfsIOC = 0. The cfg wins here.
//
// String parameters of the same section, kept verbatim for the record (unused by this table):
//   PlatConfigFileName = "uefiplatLA.cfg"
//   OsTypeString = "LA"
//   SpecialLogPartition = "LOGFS:\"
//   DefaultChargerApp = "QcomChargerApp"
//   DefaultBDSBootApp = "LinuxLoader"
//
STATIC
EFI_CONFIGURATION_ENTRY_DESCRIPTOR
gConfigurationDescriptor[] = {
  // Name, Value
  {"ConfigParameterCount", 0x40},
  {"NumCpusFuseAddr", 0x5c04c},
  {"EnableShell", 0x1},
  {"EUDEnableAddr", 0x88e2000},
  {"SecPagePoolCount", 0x800},
  {"SharedIMEMBaseAddr", 0x146bf000},
  {"DloadCookieAddr", 0x1fd3000},
  {"DloadCookieValue", 0x10},
  {"NumCpus", 0x8},
  {"NumActiveCores", 0x8},
  {"MaxLogFileSize", 0x400000},
  {"UefiMemUseThreshold", 0x77},
  {"USBHS1_Config", 0x0},
  {"UsbFnIoRevNum", 0x10001},
  {"PwrBtnShutdownFlag", 0x0},
  {"Sdc1GpioConfigOn", 0x1e92},
  {"Sdc2GpioConfigOn", 0x1e92},
  {"Sdc1GpioConfigOff", 0xa00},
  {"Sdc2GpioConfigOff", 0xa00},
  {"EnableSDHCSwitch", 0x1},
  {"EnableUfsIOC", 0x0},
  {"UfsSmmuConfigForOtherBootDev", 0x1},
  {"SecurityFlag", 0xc4},
  {"EnableLogFsSyncInRetail", 0x1},
  {"ShmBridgememSize", 0xa00000},
  {"EnableMultiThreading", 0x1},
  {"MaxCoreCnt", 0x8},
  {"EarlyInitCoreCnt", 0x1},
  {"EnableUefiSecAppDebugLogDump", 0x0},
  {"AllowNonPersistentVarsInRetail", 0x1},
  {"EnableDisplayThread", 0x1},
  {"EnableDisplayImageFv", 0x0},
};

VOID
GetConfigurationMap (
  OUT EFI_CONFIGURATION_ENTRY_DESCRIPTOR **ConfigurationDescriptor,
  OUT UINT8                               *ConfigurationDescriptorCount)
{
  // Pass Data
  *ConfigurationDescriptor      = gConfigurationDescriptor;
  *ConfigurationDescriptorCount = ARRAY_SIZE (gConfigurationDescriptor);
}
