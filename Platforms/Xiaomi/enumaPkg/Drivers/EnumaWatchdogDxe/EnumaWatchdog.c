/** @file
  APSS watchdog: make a hung boot recover by itself.

  Why: every hard stop in this bring-up so far (the AFF1/cluster assert, a bad page table) ends the
  same way - the panel keeps the last frame and nothing ever moves again, so a person has to hold the
  power button for fifteen seconds before the next attempt. With the SoC watchdog armed, the same
  hang resets the tablet on its own and the next `fastboot boot` can follow immediately.

  Hardware facts, all from the board device tree (out/astide/enuma.dtb, sha256 296c7e91...):

    qcom,wdt@17c10000 {
        compatible = "qcom,msm-watchdog";
        reg = <0x17c10000 0x1000>;
        qcom,bark-time = <0x4e20>;   // 20000 ms - first stage (interrupt), the stock kernel pets here
        qcom,pet-time  = <0x3a98>;   // 15000 ms - how often the stock kernel pets it
    };

  Register layout and the millisecond-to-tick rule follow drivers/watchdog/qcom-wdt.c: WDT_RST 0x04
  (bit 0 = reset enable), WDT_EN 0x08 (bit 0 = enable), WDT_BARK_TIME 0x10, WDT_BITE_TIME 0x14, and
  ticks = ms * 32764 / 1000 for "qcom,msm-watchdog" (the sleep clock).

  This driver arms the watchdog once and never pets it: the bite stage therefore fires unless the
  boot finishes and something else takes over the counter. That is intentional - a boot that reaches
  the panic screen has already produced its evidence, and a boot that hangs gets recycled.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#define WDT_BASE          0x17C10000U
#define WDT_RST           (WDT_BASE + 0x04)
#define WDT_EN            (WDT_BASE + 0x08)
#define WDT_BARK_TIME     (WDT_BASE + 0x10)
#define WDT_BITE_TIME     (WDT_BASE + 0x14)
#define WDT_RESET_ENABLE  0x1U
#define WDT_ENABLE        0x1U

// 32764 Hz: the sleep clock the "qcom,msm-watchdog" driver assumes (drivers/watchdog/qcom-wdt.c).
#define WDT_TICK_HZ       32764U

// Deliberately generous: photographing a panic screen takes seconds, not minutes, and a longer
// first attempt is cheaper than a reset in the middle of one.
#define WDT_BARK_MS       100000U
#define WDT_BITE_MS       120000U

STATIC
UINT32
MsToTicks (
  IN UINT32  Milliseconds
  )
{
  return (UINT32)(((UINT64)Milliseconds * WDT_TICK_HZ) / 1000U);
}

EFI_STATUS
EFIAPI
EnumaWatchdogEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  UINT32  Bark = MsToTicks (WDT_BARK_MS);
  UINT32  Bite = MsToTicks (WDT_BITE_MS);

  // Status first, so the log says whether firmware left it running already.
  DEBUG ((
    DEBUG_ERROR,
    "ENUMA-WDT: base 0x%lx status 0x%x (arming bark %u ms = %u ticks, bite %u ms = %u ticks)\n",
    (UINT64)WDT_BASE,
    MmioRead32 (WDT_BASE + 0x0C),
    WDT_BARK_MS,
    Bark,
    WDT_BITE_MS,
    Bite
    ));

  // Order matters: the reset enable and the two thresholds, then the counter itself.
  MmioWrite32 (WDT_RST, WDT_RESET_ENABLE);
  MmioWrite32 (WDT_BARK_TIME, Bark);
  MmioWrite32 (WDT_BITE_TIME, Bite);
  MmioWrite32 (WDT_EN, WDT_ENABLE);

  DEBUG ((
    DEBUG_ERROR,
    "ENUMA-WDT: armed, enabled=0x%x status 0x%x - a hung boot now resets after %u ms\n",
    MmioRead32 (WDT_EN),
    MmioRead32 (WDT_BASE + 0x0C),
    WDT_BITE_MS
    ));

  return EFI_SUCCESS;
}