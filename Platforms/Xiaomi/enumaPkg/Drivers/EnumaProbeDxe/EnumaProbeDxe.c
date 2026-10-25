/** @file
  Bring-up probe for enuma (Xiaomi Pad 5 Pro 5G, SM8250-AC).

  The early framebuffer console stops producing output the moment ArmConfigureMmu() runs, so the
  SEC stage paints coloured bands straight into the display window instead (see ci/instrument_mmu.py).
  This driver continues that idea inside DXE:

    row 104  cyan    this driver's entry point ran  -> DXE dispatch is alive
    row 112  cyan    the periodic timer was armed
    row 120+ magenta/green, walking down 8 rows    -> the DXE event loop is alive (heartbeat)

  Nothing here depends on the console, the GOP or any vendor driver.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>

#define ENUMA_FB_BASE     0x9C000000u
#define ENUMA_FB_WIDTH    1600u
#define ENUMA_BAND_LINES  6u

STATIC EFI_EVENT  mHeartbeat;
STATIC UINT32     mTick;

STATIC
VOID
EnumaPaint (
  IN UINTN   Row,
  IN UINT32  Color
  )
{
  volatile UINT32  *Fb;
  UINTN             Index;

  Fb = (volatile UINT32 *)(UINTN)ENUMA_FB_BASE;
  for (Index = 0; Index < (ENUMA_FB_WIDTH * ENUMA_BAND_LINES); Index++) {
    Fb[(Row * ENUMA_FB_WIDTH) + Index] = Color;
  }
}

STATIC
VOID
EFIAPI
EnumaHeartbeat (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  mTick++;
  EnumaPaint (
    120 + ((mTick % 8) * (ENUMA_BAND_LINES + 2)),
    ((mTick & 1) != 0) ? 0x0000FF00u : 0x00FF00FFu
    );
}

EFI_STATUS
EFIAPI
EnumaProbeDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EnumaPaint (104, 0x0000FFFFu);

  mTick = 0;
  if (gBS != NULL) {
    if (!EFI_ERROR (
          gBS->CreateEvent (
                 EVT_TIMER | EVT_NOTIFY_SIGNAL,
                 TPL_CALLBACK,
                 EnumaHeartbeat,
                 NULL,
                 &mHeartbeat
                 )
          ))
    {
      if (!EFI_ERROR (gBS->SetTimer (mHeartbeat, TimerPeriodic, 5000000u)   // 500 ms in 100 ns units)) {
        EnumaPaint (112, 0x0000FFFFu);
      }
    }
  }

  return EFI_SUCCESS;
}
