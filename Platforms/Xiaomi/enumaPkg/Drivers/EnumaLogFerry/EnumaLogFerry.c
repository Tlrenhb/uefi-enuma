/** @file
  EnumaLogFerryDxe - carry a block of kernel RAM out to a file on the logfs partition.

  Why this exists: XNU on enuma can write to physical memory (ml_io_map + a store is verified 5/5
  on this board), but nothing on this device can read RAM back: /dev/mem is not compiled into the
  stock Android kernel, /proc/kcore does not exist, pstore never surfaces the ramoops console zone,
  and there is no UART. So the only way to get a live value (the IOStorageFamily metaclass bytes the
  storage ABI audit wants, the MMU registers, a kernel log) off the device is to let the *firmware*
  pick it up on the next boot and write it somewhere an adb shell can read.

  The carrier is a fixed physical address, 0xB0400000 (inside the reserved window that was verified
  writable from XNU):

      offset 0   u32   magic 'ELOG'  (0x474f4c45)
      offset 4   u32   length         (0 when there is nothing to ferry)
      offset 8   ...   length bytes

  XNU fills that buffer and warm-resets. On the next boot this driver, at ReadyToBoot, finds the
  FAT12 volume whose GPT partition is named "logfs", writes the payload to \enuma.txt, and clears the
  magic so a stale buffer never overwrites a newer capture.

  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/EventGroup.h>

#define ENUMA_LOG_PA      0xB0400000u
#define ENUMA_LOG_MAGIC   0x474F4C45u     /* 'ELOG' */
#define ENUMA_LOG_MAX     0x4000u
#define ENUMA_LOG_FILE    L"\\enuma.txt"

STATIC EFI_EVENT  mReadyToBoot;

STATIC
EFI_STATUS
FerryToVolume (
  IN EFI_HANDLE  Handle,
  IN UINT8       *Data,
  IN UINT32       Length
  )
{
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *Fs;
  EFI_FILE_PROTOCOL                *Root;
  EFI_FILE_PROTOCOL                *File;
  EFI_FILE_PROTOCOL                *Probe;
  EFI_STATUS                        Status;
  UINTN                             Size;

  Status = gBS->HandleProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Fs->OpenVolume (Fs, &Root);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // The payload volume is the one fastboot booted; it only exists in RAM, so a file written there is
  // lost with the reset that follows. Skip it and keep looking for the on-disk FAT12 logfs partition.
  //
  Status = Root->Open (Root, &Probe, L"\EFI\UniXNU\unixnu.toml", EFI_FILE_MODE_READ, 0);
  if (!EFI_ERROR (Status)) {
    Probe->Close (Probe);
    Root->Close (Root);
    return EFI_NOT_FOUND;
  }
  Status = Root->Open (Root, &File, ENUMA_LOG_FILE,
                       EFI_FILE_MODE_CREATE | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    Root->Close (Root);
    return Status;
  }
  Size   = Length;
  Status = File->Write (File, &Size, Data);
  if (!EFI_ERROR (Status)) {
    Status = File->Flush (File);
  }
  File->Close (File);
  Root->Close (Root);
  return Status;
}

STATIC
VOID
EFIAPI
OnReadyToBoot (
  IN EFI_EVENT  Event,
  IN VOID      *Context
  )
{
  volatile UINT8  *Carrier;
  UINT8           *Data;
  UINT32           Length;
  EFI_HANDLE      *Handles;
  UINTN            Count;
  UINTN            Index;
  EFI_STATUS       Status;

  (VOID)Event;
  (VOID)Context;

  Carrier = (volatile UINT8 *)(UINTN)ENUMA_LOG_PA;
  if (*(volatile UINT32 *)Carrier != ENUMA_LOG_MAGIC) {
    return;
  }
  Length = *(volatile UINT32 *)(Carrier + 4);
  if ((Length == 0) || (Length > ENUMA_LOG_MAX)) {
    return;
  }

  Handles = NULL;
  Count   = 0;
  Status  = gBS->LocateHandleBuffer (ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &Count, &Handles);
  if (EFI_ERROR (Status)) {
    return;
  }

  Data   = (UINT8 *)(UINTN)(Carrier + 8);
  for (Index = 0; Index < Count; Index++) {
    if (!EFI_ERROR (FerryToVolume (Handles[Index], Data, Length))) {
      *(volatile UINT32 *)Carrier = 0;          /* consumed: never replay an old capture */
      gST->ConOut->OutputString (gST->ConOut, L"ENUMA-FERRY: wrote \\enuma.txt to the first writable FAT volume\r\n");
      break;
    }
  }

  gBS->FreePool (Handles);
}

EFI_STATUS
EFIAPI
EnumaLogFerryDxeEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  (VOID)ImageHandle;
  (VOID)SystemTable;

  return gBS->CreateEventEx (EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnReadyToBoot, NULL,
                             &gEfiEventReadyToBootGuid, &mReadyToBoot);
}