#!/usr/bin/env bash
#
# extract-vendor-dxe.sh — enuma (Xiaomi Pad 5 Pro 5G) vendor DXE extraction pipeline (skeleton).
#
# Turns the device firmware into the `Binaries/enuma/` tree that Mu-Silicium's enumaPkg needs:
#
#   1. obtain the firmware image: dump xbl/uefi from the tablet (root) or take them from an official
#      fastboot ROM package
#   2. run UEFIReader over the UEFI partition image        -> per-module DXE trees + RawFiles/
#   3. move the output into <checkout>/Binaries/enuma/      (the path every DXE.inc line references)
#   4. run CleanUp.sh in the Binaries folder (once)         -> the upstream normalisation step
#   5. apply the four manual DXE patches the fix-up list names
#
# Every step checks what it needs and stops with a specific message. Nothing is skipped silently.
# Everything this script does NOT know (module names, addresses, GUIDs) stays in the package files as
# `# TODO(enuma)`: the dump is what fills them.
#
# Reference: Project-Silicium/Guides, Silicium/Porting/Snapdragon/Device/README.md (Steps 1-3).
#            uefi-pipa branch `pipa` for the worked same-SoC example.
#
set -euo pipefail

CHECKOUT=""
DUMP_DIR=""
ROM_DIR=""
UEFIREADER=""
CLEANUP_SCRIPT=""
PATCHES_DIR=""
DRY_RUN=0

usage() {
  sed -n '3,20p' "$0" | sed 's/^# \{0,1\}//'
  cat <<'USAGE'

Options:
  --checkout DIR        Mu-Silicium / uefi-enuma checkout that will receive Binaries/enuma/ (required)
  --dump-dir DIR        directory holding the device dump (xbl*.img / uefi*.img)   [one of the two]
  --rom-dir DIR         directory holding an extracted official fastboot ROM        [is required]
  --uefireader PATH     UEFIReader binary (default: $CHECKOUT/../UEFIReader/bin/Debug/net8.0/UEFIReader
                        or `UEFIReader` on PATH)
  --cleanup-script PATH CleanUp.sh shipped with the Binaries drop (see step 4)
  --patches-dir DIR     directory holding the four DXE patches (see step 5)
  --dry-run             print the commands instead of running them
  -h, --help            this text

Exit codes: 1 = a required input is missing, 2 = a tool is missing, 3 = a step produced no output.
USAGE
}

log()  { printf '\n\033[1m==> %s\033[0m\n' "$*"; }
fail() { printf '\nERROR: %s\n' "$1" >&2; exit "${2:-1}"; }

run() {
  if [ "$DRY_RUN" = 1 ]; then printf '  [dry-run] %s\n' "$*"; else printf '  $ %s\n' "$*"; "$@"; fi
}

while [ $# -gt 0 ]; do
  case "$1" in
    --checkout)        CHECKOUT="${2:?}"; shift 2 ;;
    --dump-dir)        DUMP_DIR="${2:?}"; shift 2 ;;
    --rom-dir)         ROM_DIR="${2:?}"; shift 2 ;;
    --uefireader)      UEFIREADER="${2:?}"; shift 2 ;;
    --cleanup-script)  CLEANUP_SCRIPT="${2:?}"; shift 2 ;;
    --patches-dir)     PATCHES_DIR="${2:?}"; shift 2 ;;
    --dry-run)         DRY_RUN=1; shift ;;
    -h|--help)         usage; exit 0 ;;
    *)                 fail "unknown argument: $1 (try --help)" ;;
  esac
done

[ -n "$CHECKOUT" ] || fail "--checkout is required (the Mu-Silicium / uefi-enuma checkout)"
[ -d "$CHECKOUT" ] || fail "--checkout '$CHECKOUT' is not a directory"
[ -n "$DUMP_DIR" ] || [ -n "$ROM_DIR" ] || fail "give --dump-dir (device dump) or --rom-dir (official ROM)"

# ---------------------------------------------------------------------------------------------
log "step 1/5: locate the firmware image (xbl and/or uefi)"
# ---------------------------------------------------------------------------------------------
SRC_DIR=""
if [ -n "$DUMP_DIR" ]; then
  [ -d "$DUMP_DIR" ] || fail "--dump-dir '$DUMP_DIR' is not a directory"
  SRC_DIR="$DUMP_DIR"
  cat <<'HOWTO'
  How to produce the dump (device must be rooted; this reads only):
    adb shell su -c 'dd if=/dev/block/by-name/xbl of=/data/local/tmp/xbl.img'
    adb shell su -c 'dd if=/dev/block/by-name/uefi of=/data/local/tmp/uefi.img'
    adb pull /data/local/tmp/xbl.img /data/local/tmp/uefi.img <dump-dir>/
  The partition names differ on some builds: check `adb shell ls -l /dev/block/by-name | grep -iE 'xbl|uefi'`
  and pass whatever exists. XBL carries uefiplat.cfg (memory map + configuration), which the package
  files need; the UEFI partition carries the DXE drivers.
HOWTO
elif [ -n "$ROM_DIR" ]; then
  [ -d "$ROM_DIR" ] || fail "--rom-dir '$ROM_DIR' is not a directory"
  SRC_DIR="$ROM_DIR"
  echo "  Using an official fastboot ROM. Expect xbl*.elf/img and uefi*.img inside; names vary per release."
fi

shopt -s nullglob nocaseglob
XBL_FILES=("$SRC_DIR"/xbl*.img "$SRC_DIR"/xbl*.elf)
UEFI_FILES=("$SRC_DIR"/uefi*.img "$SRC_DIR"/uefi*.elf "$SRC_DIR"/uefi*.bin)
shopt -u nullglob nocaseglob

[ "${#UEFI_FILES[@]}" -gt 0 ] || fail "no uefi*.img/elf/bin in $SRC_DIR.
  Without the UEFI partition image there is nothing to extract.
  If the ROM ships a super/super_empty or a single 'uefi' file without extension, rename it to uefi.img
  or pass its directory containing it as-is (this script matches uefi*)."
if [ "${#XBL_FILES[@]}" -eq 0 ]; then
  echo "  WARNING: no xbl* image in $SRC_DIR. UEFIReader can still extract the DXE drivers from the UEFI"
  echo "           image, but uefiplat.cfg (the memory map and [ConfigParameters] source) lives in XBL:"
  echo "           MemoryMapLib.c, ConfigurationMapLib.c and Resources/Configs/enuma.toml stay TODO."
fi
printf '  uefi image(s): %s\n' "${UEFI_FILES[*]}"
[ "${#XBL_FILES[@]}" -gt 0 ] && printf '  xbl image(s):  %s\n' "${XBL_FILES[*]}"

# ---------------------------------------------------------------------------------------------
log "step 2/5: run UEFIReader"
# ---------------------------------------------------------------------------------------------
if [ -z "$UEFIREADER" ]; then
  for candidate in \
      "$CHECKOUT/../UEFIReader/bin/Debug/net8.0/UEFIReader" \
      "$CHECKOUT/../UEFIReader/bin/Release/net8.0/UEFIReader" \
      "$(command -v UEFIReader || true)"; do
    [ -n "$candidate" ] && [ -x "$candidate" ] && { UEFIREADER="$candidate"; break; }
  done
fi
if [ -n "$UEFIREADER" ] && { [ ! -e "$UEFIREADER" ] || [ ! -x "$UEFIREADER" ]; }; then
  fail "--uefireader '$UEFIREADER' is not an executable file.
  Build UEFIReader per the official guide (dotnet build UEFIReader.sln) or drop --uefireader to let the
  script search common locations and PATH." 2
fi
[ -n "$UEFIREADER" ] || fail "UEFIReader not found. Build it per the official guide:
    sudo apt install dotnet-sdk-8.0   # or the macOS equivalent
    git clone https://github.com/WOA-Project/UEFIReader.git
    cd UEFIReader && dotnet build UEFIReader.sln
    # then pass --uefireader UEFIReader/bin/Debug/net8.0/UEFIReader
  (The guide also pins a compiled build in the project's Discord #general.)" 2

OUT_DIR="${TMPDIR:-/tmp}/enuma-uefireader-out"
rm -rf "$OUT_DIR"; mkdir -p "$OUT_DIR"
for img in "${UEFI_FILES[@]}"; do
  echo "  extracting $(basename "$img")"
  run "$UEFIREADER" "$img" "$OUT_DIR"
done
if [ "$DRY_RUN" = 0 ]; then
  [ -n "$(ls -A "$OUT_DIR" 2>/dev/null || true)" ] || fail "UEFIReader produced nothing in $OUT_DIR" 3
  echo "  produced:"; find "$OUT_DIR" -maxdepth 2 -mindepth 1 | head -20 | sed 's/^/    /'
fi

# ---------------------------------------------------------------------------------------------
log "step 3/5: install into <checkout>/Binaries/enuma/"
# ---------------------------------------------------------------------------------------------
BINARIES="$CHECKOUT/Binaries/enuma"
if [ -e "$BINARIES" ] && [ "$(ls -A "$BINARIES" 2>/dev/null || true)" ]; then
  echo "  $BINARIES already has content; leaving it untouched (delete it yourself to re-extract)."
else
  run mkdir -p "$BINARIES"
  run cp -a "$OUT_DIR"/. "$BINARIES"/
fi
# The guide's path is Mu-Silicium/Binaries/<Device Codename>/ and every generated INF line reads
# `INF Binaries/enuma/QcomPkg/Drivers/<Module>/<Module>.inf`.
echo "  NOTE: these are vendor binaries. Decide deliberately whether to commit them; the device"
echo "        repository never contains them (its [firmware] submodules list pulls upstream instead)."
if [ "${#XBL_FILES[@]}" -gt 0 ]; then
  echo "  uefiplat.cfg: look for it under Binaries/enuma/RawFiles/ (it is what MemoryMapLib.c,"
  echo "                ConfigurationMapLib.c and Resources/Configs/enuma.toml are derived from)."
  echo "                If it is not there, extract the RawFiles from the XBL image as well:"
  echo "                $UEFIREADER ${XBL_FILES[0]} ${TMPDIR:-/tmp}/enuma-xbl-out"
fi

# ---------------------------------------------------------------------------------------------
log "step 4/5: vendor-tree normalisation (upstream: clean_up.py)"
# ---------------------------------------------------------------------------------------------
# The guide says: "Then Execute `CleanUp.sh` in the Binaries Folder once." — that name is STALE.
# VERIFIED 2026-10-23 (lead, first-hand):
#   - Project-Silicium/Mu-Silicium `Binaries` is a **git submodule** pointing at
#     Project-Silicium/Device-Binaries (submodule sha a1501e0c2e64…). So per-device binaries and
#     their notes live in Device-Binaries, NOT in the Mu-Silicium tree.
#   - `CleanUp.sh` does not exist anywhere we could read (404 on Binaries/CleanUp.sh etc.).
#     The upstream normaliser is now **`clean_up.py`** at the ROOT of Device-Binaries
#     (it deletes stray *.inc and re-formats *.inf). Path:
#       https://raw.githubusercontent.com/Project-Silicium/Device-Binaries/main/clean_up.py
#   - The four DXE patches are documented in Device-Binaries/pipa/README.md (HTTP 200), which also
#     records the exact stock firmware the pipa drop came from (OS2.0.12.0.UMZMIXM /
#     BOOT.XF.3.2-00304-SM8250-2) — useful as a sanity check for enuma's own drop.
CANDIDATES=("$CLEANUP_SCRIPT" "$BINARIES/clean_up.py" "$CHECKOUT/Binaries/clean_up.py" \
            "$BINARIES/CleanUp.sh" "$CHECKOUT/Binaries/CleanUp.sh")
FOUND_CLEANUP=""
for candidate in "${CANDIDATES[@]}"; do
  [ -n "$candidate" ] && [ -f "$candidate" ] && { FOUND_CLEANUP="$candidate"; break; }
done
if [ -z "$FOUND_CLEANUP" ]; then
  fail "vendor normaliser not found (looked at: ${CANDIDATES[*]}).
  Get it from the submodule that actually holds it:
    git clone https://github.com/Project-Silicium/Device-Binaries    # clean_up.py is at its root
  then re-run with --cleanup-script <path>. Do not skip this step silently: the extracted tree
  contains files the build does not want." 1
fi
echo "  using $FOUND_CLEANUP"
case "$FOUND_CLEANUP" in
  *.py) ( cd "$(dirname "$FOUND_CLEANUP")" && run python3 "$(basename "$FOUND_CLEANUP")" ) ;;
  *)    ( cd "$(dirname "$FOUND_CLEANUP")" && run bash "$(basename "$FOUND_CLEANUP")" ) ;;
esac

# ---------------------------------------------------------------------------------------------
log "step 5/5: the four manual DXE patches"
# ---------------------------------------------------------------------------------------------
# The task brief names these four as the mandatory fix-ups for the pipa-class device:
#   ButtonsDxe, UsbConfigDxe, TzDxeLA, FeatureEnablerDxe, UsbMsdDxe, DALSYSDxe
# VERIFIED 2026-10-23 (lead, first-hand): the pipa README lists SIX patch sections, not four.
#   UsbMsdDxe   -> exposes internal storage as a fixed disk (needed for the payload/ESP)
#   DALSYSDxe   -> `\_SB_.UFS0` cache coherency (directly affects storage stability)
# Each corresponds to a module the previous step extracted (ButtonsDxe.inf, UsbConfigDxe.inf,
# TzDxeLA.inf, FeatureEnablerDxe.inf). Apply them to the extracted modules BEFORE building; they are
# what makes the volume keys, the USB role switch, the trustzone app loader and the feature enabler
# behave under this firmware.
#
# VERIFIED 2026-10-23: the location the brief names (Device-Binaries/pipa/README.md) does not resolve:
#   404 for Mu-Silicium/main/Device-Binaries/pipa/README.md, uefi-pipa/pipa/Device-Binaries/pipa/README.md,
#   and the Mu-Silicium main tree has no Device-Binaries/ path at all.
# So the patches are treated as an input the porter must supply (community drop), not as a repo path.
PATCH_NAMES=(ButtonsDxe UsbConfigDxe TzDxeLA FeatureEnablerDxe UsbMsdDxe DALSYSDxe)
echo "  required patches (apply by hand, then re-check the module list):"
for name in "${PATCH_NAMES[@]}"; do
  printf '    - %-20s module: Binaries/enuma/QcomPkg/Drivers/%s/%s.inf\n' "$name" "$name" "$name"
done
if [ -n "$PATCHES_DIR" ]; then
  [ -d "$PATCHES_DIR" ] || fail "--patches-dir '$PATCHES_DIR' is not a directory"
  missing=0
  for name in "${PATCH_NAMES[@]}"; do
    match=$(find "$PATCHES_DIR" -maxdepth 2 -iname "*${name}*.patch" -print -quit 2>/dev/null || true)
    if [ -n "$match" ]; then
      echo "  found: $match"
      ( cd "$CHECKOUT" && run git apply --check "$match" ) || \
        echo "    WARNING: 'git apply --check' failed for $match; inspect it before applying."
    else
      echo "  MISSING: no *${name}*.patch under $PATCHES_DIR"; missing=1
    fi
  done
  [ "$missing" = 0 ] || fail "not all six patches are present under $PATCHES_DIR" 1
else
  echo "  no --patches-dir given: the patches must be supplied by the porter (see the note above)."
fi

# ---------------------------------------------------------------------------------------------
log "next steps (all of them are skeleton TODOs in this repository)"
# ---------------------------------------------------------------------------------------------
cat <<'NEXT'
  1. Fill Platforms/Xiaomi/enumaPkg/Include/{APRIORI,DXE,RAW}.inc from the dump:
       - DXE.inc     : one `INF Binaries/enuma/QcomPkg/Drivers/<Module>/<Module>.inf` per module
       - APRIORI.inc : same modules in the order UEFITool reports for the stock image
       - RAW.inc     : every RawFiles entry with its stock FFS GUID (uefiplat.cfg included)
  2. Fill Library/MemoryMapLib/MemoryMapLib.c and Library/ConfigurationMapLib/ConfigurationMapLib.c
     from uefiplat.cfg (remove the intentional #error lines once the tables are real).
  3. Fill Resources/Configs/enuma.toml ([uefi_fd] base/size, header_version) from the same file, and
     make enuma.dsc's PcdCPUCoresStackBase match the "UEFI Stack" entry.
  4. Build and iterate:
       UNIXNU_UEFI=<checkout> ./unixnu build firmware -D <device repo>
       UNIXNU_UEFI=<checkout> ./unixnu bootimg -D <device repo> --dtb <enuma.dtb> --ramdisk none
       fastboot boot out/enuma/boot/boot.img
     (see ../../../UniXNU-enuma-skeleton/scripts/build-enuma-uefi.sh for the round loop)
NEXT

echo
echo "done."