/** @file
  Xiaomi Pad 5 Pro 5G (enuma) PM8150 buttons via SPMI v5 observer reads.

  Board DT: EE 0, PM8150 SID 0, PON 0x800 (power / volume down), GPIO6
  0xc500 (active-low volume up). Linux reports arbiter 0x50010000.
  Register layout and command format: LineageOS SM8250 71b13e62,
  drivers/spmi/spmi-pmic-arb.c, drivers/input/misc/qpnp-power-on.c and
  drivers/pinctrl/qcom/pinctrl-spmi-gpio.c. URLs/hashes are retained in
  out/umi/input-reference/sources.json. Original ButtonsDxe independently
  polls PMIC 0 GPIO6 and PON interrupt selectors 0/1.

  Only the arbiter observer READ command is issued. No PMIC write, interrupt
  acknowledgement, reset setting, ownership or GPIO configuration changes.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/
#include <Uefi.h>
#include <Guid/EventGroup.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Configuration/BootDevices.h>
#include "EnumaKeyFilter.h"

#define ARB_CORE       0x0C440000U
#define ARB_CONFIG     0x0C40A000U
#define ARB_OBSERVER   0x0E600000U
#define ARB_CHANNELS   512U
#define QUEUE_SIZE     16U

STATIC UINTN mPonChannel;
STATIC UINTN mGpioChannel;
STATIC EFI_EVENT mTimer;
STATIC EFI_EVENT mExitEvent;
STATIC EFI_INPUT_KEY mQueue[QUEUE_SIZE];
STATIC UINTN mHead;
STATIC UINTN mTail;
STATIC UINTN mReadErrors;
STATIC UINTN mLoggedKeys;
STATIC UINTN mLoggedResyncs;
STATIC BOOLEAN mResync = TRUE;
STATIC BOOLEAN mStopped;
STATIC ENUMA_KEY_FILTER mFilter;
STATIC EFI_SIMPLE_TEXT_INPUT_PROTOCOL mInput;


//
// Bring-up: no serial port, so the entry result is painted into rows 820-879, above the strip the
// menu repaints. Whatever colour is there at the menu is the branch this driver took:
//   green    SimpleTextIn installed
//   red      SPMI arbiter version is neither v5 nor v7
//   orange   the PM8150 PON observer channel was not found
//   yellow   the GPIO (volume-up) observer channel was not found
//   magenta  the first status read failed
//   cyan     a protocol install failed
//
STATIC
VOID
EnumaOutcomeMark (
  IN UINT32  Color
  )
{
  volatile UINT32  *Fb = (volatile UINT32 *)(UINTN)0x9C000000;
  UINTN            I;

  for (I = 0; I < (1600u * 60u); I++) {
    Fb[(820u * 1600u) + I] = Color;
  }
}

STATIC EFI_STATUS
FindObserverChannel (UINT16 Ppid, UINTN *Channel)
{
  UINT32 Mapping;
  UINT32 Owner;
  UINTN Index;
  BOOLEAN Found = FALSE;

  for (Index = 0; Index < ARB_CHANNELS; ++Index) {
    Mapping = MmioRead32 (ARB_CORE + 0x900 + Index * 4);
    if ((Mapping == 0) || (((Mapping >> 8) & 0xFFF) != Ppid)) {
      continue;
    }
    Owner = MmioRead32 (ARB_CONFIG + 0x700 + Index * 4) & 7;
    // Match Linux's v5 selection: first mapping, overridden by an EE0 owner.
    if (!Found || (Owner == 0)) {
      *Channel = ARB_OBSERVER + 0x80 * Index;
      Found = TRUE;
    }
    DEBUG ((EFI_D_ERROR, "ENUMA keys: PPID=%03x APID=%u owner=%u\n",
            Ppid, (UINT32)Index, Owner));
  }
  return Found ? EFI_SUCCESS : EFI_NOT_FOUND;
}

STATIC EFI_STATUS
ReadStatusByte (UINTN Channel, UINT8 *Value)
{
  UINTN Try;
  UINT32 Status;

  // EXT_READL = 1, peripheral offset 0x10, length 1 (byte count field 0).
  // SID/peripheral are selected by APID, not by the command's address field.
  MmioWrite32 (Channel, (1U << 27) | (0x10U << 4));
  for (Try = 0; Try < 1000; ++Try) {
    Status = MmioRead32 (Channel + 0x08);
    if ((Status & BIT0) != 0) {
      if ((Status & (BIT1 | BIT2 | BIT3)) != 0) {
        return EFI_DEVICE_ERROR;
      }
      *Value = (UINT8)MmioRead32 (Channel + 0x18);
      return EFI_SUCCESS;
    }
    MicroSecondDelay (1);
  }
  return EFI_TIMEOUT;
}

STATIC EFI_STATUS
ReadKeys (UINT8 *Mask)
{
  EFI_STATUS Status;
  UINT8 Pon;
  UINT8 Gpio;

  Status = ReadStatusByte (mPonChannel, &Pon);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = ReadStatusByte (mGpioChannel, &Gpio);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *Mask = ((Gpio & BIT0) ? 0 : BIT0) |
          ((Pon & BIT1) ? BIT1 : 0) |
          ((Pon & BIT0) ? BIT2 : 0);
  return EFI_SUCCESS;
}

STATIC VOID
EFIAPI
PollKeys (EFI_EVENT Event, VOID *Context)
{
  EFI_STATUS Status;
  UINT8 Mask;
  UINT8 Press;
  UINT64 NowUs;
  UINTN Next;

  if (mStopped) {
    return;
  }
  Status = ReadKeys (&Mask);
  if (EFI_ERROR (Status)) {
    if (mReadErrors++ < 4) {
      DEBUG ((EFI_D_ERROR, "ENUMA keys: status read failed %r\n", Status));
    }
    mResync = TRUE;
    return;
  }
  NowUs = GetTimeInNanoSecond (GetPerformanceCounter ()) / 1000;
  if (mResync) {
    EnumaKeyFilterInit (&mFilter, Mask, NowUs);
    mResync = FALSE;
    if (mLoggedResyncs++ < 8) {
      DEBUG ((EFI_D_ERROR, "ENUMA keys: initial/resync mask=%02x\n", Mask));
    }
    return;
  }
  Press = EnumaKeyFilterUpdate (&mFilter, Mask, NowUs);
  if (Press == 0) {
    return;
  }
  Next = (mTail + 1) % QUEUE_SIZE;
  if (Next == mHead) {
    return;
  }
  mQueue[mTail].ScanCode = (Press == BIT0) ? SCAN_UP :
                          (Press == BIT1) ? SCAN_DOWN : SCAN_NULL;
  mQueue[mTail].UnicodeChar = (Press == BIT2) ? CHAR_CARRIAGE_RETURN : 0;
  mTail = Next;
  if (mLoggedKeys++ < 64) {
    DEBUG ((EFI_D_ERROR, "ENUMA keys: event=%a raw=%02x\n",
            (Press == BIT0) ? "volume-up" : (Press == BIT1) ? "volume-down" : "power",
            Mask));
  }
  gBS->SignalEvent (mInput.WaitForKey);
}

STATIC EFI_STATUS
EFIAPI
ResetKeys (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, BOOLEAN ExtendedVerification)
{
  EFI_TPL OldTpl;
  if (This != &mInput) {
    return EFI_INVALID_PARAMETER;
  }
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
  mHead = mTail = 0;
  mResync = TRUE;
  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS
EFIAPI
ReadKey (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This, EFI_INPUT_KEY *Key)
{
  EFI_TPL OldTpl;
  EFI_STATUS Status = EFI_NOT_READY;
  if ((This != &mInput) || (Key == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);
  if (mHead != mTail) {
    *Key = mQueue[mHead];
    mHead = (mHead + 1) % QUEUE_SIZE;
    Status = EFI_SUCCESS;
  }
  gBS->RestoreTPL (OldTpl);
  return Status;
}

STATIC VOID
EFIAPI
WaitForKey (EFI_EVENT Event, VOID *Context)
{
  if (mHead != mTail) {
    gBS->SignalEvent (Event);
  }
}

STATIC VOID
EFIAPI
StopKeys (EFI_EVENT Event, VOID *Context)
{
  mStopped = TRUE;
  gBS->SetTimer (mTimer, TimerCancel, 0);
}

EFI_STATUS
EFIAPI
EnumaButtonsEntry (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_HANDLE Handle = NULL;
  UINT32 Version;
  UINT8 Mask;

  Version = MmioRead32 (ARB_CORE);
  DEBUG ((EFI_D_ERROR, "ENUMA keys: arbiter=%08x EE=0\n", Version));
  if (((Version & 0xF0000000U) != 0x50000000U) && ((Version & 0xF0000000U) != 0x70000000U)) {
    EnumaOutcomeMark (0x00FF0000u);
    return EFI_UNSUPPORTED;
  }
  Status = FindObserverChannel (0x008, &mPonChannel);
  if (EFI_ERROR (Status)) {
    EnumaOutcomeMark (0x00FF8000u);
    return Status;
  }
  Status = FindObserverChannel (0x0C5, &mGpioChannel);
  if (EFI_ERROR (Status)) {
    EnumaOutcomeMark (0x00FFFF00u);
    return Status;
  }
  Status = ReadKeys (&Mask);
  DEBUG ((EFI_D_ERROR, "ENUMA keys: initial read %r\n", Status));
  if (EFI_ERROR (Status)) {
    EnumaOutcomeMark (0x00FF00FFu);
    return Status;
  }
  mInput.Reset = ResetKeys;
  mInput.ReadKeyStroke = ReadKey;
  Status = gBS->CreateEvent (EVT_NOTIFY_WAIT, TPL_NOTIFY, WaitForKey, NULL, &mInput.WaitForKey);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, PollKeys, NULL, &mTimer);
  if (EFI_ERROR (Status)) {
    goto Cleanup;
  }
  Status = gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_NOTIFY, StopKeys, NULL,
                              &gEfiEventExitBootServicesGuid, &mExitEvent);
  if (EFI_ERROR (Status)) {
    goto Cleanup;
  }
  Status = gBS->SetTimer (mTimer, TimerPeriodic, 100000); // 10 ms.
  if (EFI_ERROR (Status)) {
    goto Cleanup;
  }
  // Match the ConIn path already registered by PlatformBootManagerBeforeConsole.
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle, &gEfiDevicePathProtocolGuid, &KeypadDevicePath,
                  &gEfiSimpleTextInProtocolGuid, &mInput, NULL);
  if (!EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ENUMA keys: SimpleTextIn installed; volume select, short power release confirms\n"));
    EnumaOutcomeMark (0x0000FF00u);
    return EFI_SUCCESS;
  }
Cleanup:
  if (mTimer != NULL) {
    gBS->CloseEvent (mTimer);
  }
  if (mExitEvent != NULL) {
    gBS->CloseEvent (mExitEvent);
  }
  gBS->CloseEvent (mInput.WaitForKey);
  return Status;
}

