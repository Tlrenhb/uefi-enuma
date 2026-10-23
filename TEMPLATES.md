# TEMPLATES.md — 三个同 SoC 模板对照，以及 enuma 每个选择的理由

> 任务：task-15。作者：teammate `docs-claims`。日期：2026-10-23。
> 只读来源（raw + GitHub API，未克隆）：`reAAAq/uefi-umi`（default `main`，pushed 2026-10-23）、`ht94911/uefi-cas`（default `cas`，pushed 2026-10-22）、`xVanTuring/uefi-pipa`（default `pipa`）、上游 `Project-Silicium/Mu-Silicium`。
> 真机证据来自官方设备表：`docs/devices/index.md:23`（umi："iPadOS 17.7.11 Hello/Setup with touch; CPU display, **UFS and persistent Data**"）、`:35`（cas："UEFI and UniXNU menu verified; DarwinBoot and XNU not tested"）、`:44` + `docs/devices/pipa.md:7`（pipa："not run on the tablet"）。

## 1. 结论：以 **umi** 为主模板

| 判据 | umi | cas | pipa |
| --- | --- | --- | --- |
| 官方记录的真机进度 | **UEFI 菜单 + payload 卷 + DarwinBoot + UFS 根卷 + launchd + 持久 Data**（`docs/devices/index.md:23`；`uefi-umi` 根 README:14） | UEFI + 菜单 + 分区完整性校验（`docs/devices/index.md:35`） | 从未上机（`docs/devices/pipa.md:7`） |
| 包的形态 | 52 文件：`Include/{APRIORI,DXE,RAW}.inc` + `Library/{MemoryMapLib,ConfigurationMapLib}` + 板级驱动/应用 | 48 文件：**自带 `Sec/`、`SerialPortLib`、`CasDisplayLib`、`CasInputLib`**、`Include/{Components,Libraries}.dsc.inc` | 11 文件：与上游 pipaPkg 相同的最小形态 |
| 复用成本 | 结构与我们已写的骨架一一对应（同目录/同类名），移植面小 | 需要接受它的自有 SEC 与显示/输入库，等于另起一套 | 结构最小，但无真机证据 |
| toml 形态 | 与官方键位表**完全一致**且最小 | 扩展了 cas 专属键（`[boot_image_dtb]`、自有 BootShim 三件套、header v2） | 与 umi 同形态 |

**理由**：umi 是唯一在 SM8250 上把整条链（含 UFS 与持久 Data）跑到 launchd 的端口；它的包结构与我们在 task-13 写好的骨架同构（`Include/*.inc` + `Library/{MemoryMapLib,ConfigurationMapLib}`），可以直接"照着改名字 + 换值"；它的 `uefi_fd` 值有明确出处（"Matched to the uefiplat.cfg extracted from this phone's xbl_5"），且与 pipa 的值逐字节相同 —— 两台独立 SM8250 一致，是我们能拿到的最强先验。
cas 作为**第二参考**保留：它的 `cas.toml` 是本组里唯一把 Android boot image 参数（`base/kernel_offset/tags_offset/pagesize/ramdisk_offset/dtb_offset`）显式写出来的，可以在没有 umi 对应键时当交叉证据；它的自有 SEC/显示库则是"如果 enuma 的 ABL/BDS 走不通"的备选重活。
pipa 降为**第三参考**（仅用于 SM8250-AC 的 SoC 选择与结构对照）。

## 2. 逐项对照表

| 项 | umi（真机：到桌面） | cas（真机：到菜单） | pipa（未上机） | **enuma 的选择 + 理由** |
| --- | --- | --- | --- | --- |
| `[uefi_fd] base` | `0x9fc00000`（注释：按本机 `xbl_5` 的 uefiplat.cfg 匹配） | `0x9f000000` | `0x9FC00000` | **`0x9fc00000`**（umi+pipa 一致；enuma 仍需按自己的 uefiplat.cfg 复核） |
| `[uefi_fd] size` | `0x300000`（3 MiB） | `0x800000`（8 MiB） | `0x300000` | **`0x300000`**（同上；cas 的 8 MiB 是它自带 SEC+显示库的体积需求） |
| `[boot_image] header_version` | **1** | 2 | 1 | **1**（两台一致；cas 的 v2 与它的 DTB2/ABL 安排绑定） |
| `pagesize` | 未写（默认） | `4096` 显式 | 未写（"mkbootimg 默认"） | **4096**（cas 显式 + 官方 umi 设备页 "4096-byte pages"） |
| `kernel_offset` | 未写 | `0x8000` 显式 | 未写（默认） | **0x8000**（cas 显式 + pipa 注释说明是默认值） |
| `tags_offset` | 未写 | `0x1e00000` 显式 | 未写（默认） | **0x1e00000**（同上） |
| `base` | 未写 | `0` 显式 | 未写（默认） | **0**（同上） |
| `[boot_image_kernel]` | `gzip` + `append_dtb = true` | `gzip` + **`append_dtb = false`**（DTB2 是数据、ABL 自己套 DTBO） | `gzip` + `append_dtb = true` | **gzip + true**（umi/pipa 一致；cas 的 false 是它的 ABL 特例） |
| `[boot_shim]` | 仅 `requires_kernel_header = true` | 另有 `text_offset` / `source` / `validator` / `linker`（自有 BootShim） | 仅 `requires_kernel_header = true` | **仅 `requires_kernel_header = true`**（用上游 BootShim；cas 的独立 BootShim 是它的自研） |
| `SOC_TYPE` | **0**（SM8250） | **0**（SM8250） | **2**（SM8250-AC） | **2**（enuma 是 SM8250-AC / 骁龙 870；三个模板里只有 pipa 声明了 -AC，故此项以 pipa 为准） |
| `!include KonaPkg` | 有（`umi.dsc:19`） | 无（cas 自带 Sec 与库，未走 KonaPkg.dsc.inc） | 有（`pipa.dsc:35` 附近） | **有**（Kona SoC 层由上游提供，umi/pipa 都靠它） |
| `PcdSystemMemoryBase` | `0x80000000` | `0x80000000` | `0x80000000` | **0x80000000**（三者一致 = SM8250 级事实） |
| `PcdCPUCoresStackBase` | `0x9FF90000` | `0x9f900000` | `0x9FF90000` | **0x9FF90000**（umi+pipa 一致）→ 仍需按 enuma 的 uefiplat.cfg "UEFI Stack" 复核 |
| `PcdCPUCorePrimaryStackSize` | `0x40000` | `0x40000` | `0x40000` | **0x40000**（三者一致） |
| `BUILD_TARGETS` | `DEBUG` | `DEBUG` | `RELEASE\|DEBUG` | **DEBUG**（两个真机端口都只建 DEBUG；官方驱动默认 target 也是 DEBUG） |
| 显示路径 | `USE_CUSTOM_DISPLAY_DRIVER = 1` + 自研 **`InheritedFbDxe`**（只写 ABL 留下的保留帧缓冲；真机上菜单由此显示） | 自研 `CasGopDxe` + `CasDisplayLib` | `USE_CUSTOM_DISPLAY_DRIVER = 0`（SimpleFbDxe；无真机证据） | **先用 0（SimpleFbDxe）**，把 umi 的"InheritedFbDxe 模式"写成备选 TODO：它是唯一在 SM8250 真机上让菜单出画的路径，但代码是 umi 专属（1080×2340），enuma 需要按自己面板（双 CPhY）改写 |
| 串口/日志库 | `SerialPortLib` → **`RamoopsSerialPortLib`**（EDK II 串口输出直接进 stock kernel 的 ramoops 控制台） | 自带 `SerialPortLib` | `RamLogLib` → `RamoopsLogLib` | 骨架已逐字复制 pipa 的 `RamoopsLogLib`（`RamLogLib`）；**TODO(enuma)**：按 umi 追加 `SerialPortLib|.../RamoopsSerialPortLib`（真机验证过的排障回路），二者用途不同（一个重定向串口、一个供调试掩码写日志） |
| NV / Capsule | `PcdEmuVariableNvModeEnable TRUE`、`PcdResetOnMemoryTypeInformationChange FALSE`、`PcdCapsuleInRamSupport FALSE`、`[PcdsFeatureFlag]` 两个 capsule 开关 FALSE、`PcdUniXNUDebugLevel 0x80000000` | `PcdEmuVariableNvModeEnable TRUE`、`PcdFirmwareVendor` 自设 | 无 | **采用 umi 的整套**（真机到桌面；这是 EDK II/UniXNUDxe 运行环境，与具体机型无关） |
| 空实现库 | `DeviceBootManagerLib`/`PlatformBootOptionsLib`/`SecureBootHandlerLib`/`AcpiTableUpdateLib`/`RamManagerLib` 全部用 **`SiliciumNullPkg/...Null`**；`CapsuleLib` 用 `DxeCapsuleLibNull` | 自有 Components/Libraries.dsc.inc | 无 | **采用 umi 的 null 库组合**（把 BDS/ACPI/RamManager 交给 UniXNU，是 umi 走到桌面的配置） |
| 额外自研库 | `UmiResetSystemLib`、`UmiSpmiLib`、`UmiRpmhLib`、`NullSecureBootHandlerLib` | `CasResetLib`、`CasSpmi`（在 CasInputLib 内）、`CasDisplayLib` | 无 | **不复制**：这些是各自的板级 PMIC/RPMH/复位实现；enuma 需要自己的（或先不启用）。**TODO(enuma)**：确定侧键/复位路径是否必须自研（否则退回上游 `ResetSystemLib`） |
| FDF 布局 | FD → FVMAIN_COMPACT → Sec + `SiliciumPkg/Common.fdf.inc`；`[FV.FvMain]` 含 `!include Include/{APRIORI,DXE,RAW}.inc` + UniXNU app/driver INF + `BootManagerMenuApp` + `MsBootPolicy` + BootLogo/NoBootOS FREEFORM | FD → FVMAIN_COMPACT（file GUID `51ae0a65-…`）+ 自有 Sec；`[FV.FvMain]` 只 `!include APRIORI.inc, DXE.inc`（无 RAW.inc） | FD → FVMAIN_COMPACT（file GUID `9E21FD93-…`）+ Sec；`[FV.FvMain]` 含三个 include + UniXNU INF | **采用 umi 的 FDF 形态**（三 include 齐全）；`FILE FV_IMAGE` 的 GUID 沿用上游 `9E21FD93-…`（cas 自改了 GUID，无必要）。`BootManagerMenuApp`/`MsBootPolicy` 保留并标注来源 umi，若 enuma 的 BDS 策略不同可按 pipa 去掉 |
| vendor 二进制引用 | `Binaries/<codename>/…`（本仓库树里没有 `Binaries/`，靠提取产物落地） | 同 | 同 | 三者一致：`Binaries/enuma/QcomPkg/Drivers/<Module>/<Module>.inf`，模块清单等 dump |
| preflight | 仓库内无 | 仓库内无 | 仓库内无（设备仓库侧有 `check-pipa.py`） | **UEFI 仓库不放 preflight**：官方 `[firmware] preflight` 读的是设备仓库/构建树里的脚本（我们的 `UniXNU-enuma-skeleton/scripts/check-enuma.py`），此处保持无 |
| 包内 `PACKAGES_PATH` 形态 | 由设备仓库/构建器传入：`{uefi}:{uefi}/Platforms/Xiaomi:{tree}/package:{tree}/drivers/DarwinBoot:{edk2}:{uefi}/Common/Mu:…`（官方 `docs/porting/uefi-firmware.md:76-90`；pipa board.toml 的 `[firmware] packages` 是同一形态） | 同 | 同（`UniXNU-pipa/board.toml` 的 `[firmware] packages` 列表） | **沿用官方形态**，具体列表见设备仓库 `board.toml`；UEFI 侧 `enuma.dsc` 只声明 Components 与 LibraryClasses，**不要**自己设 PACKAGES_PATH |

## 3. 每个 enumaPkg 文件的来源出处

| enumaPkg 文件 | 结构来源 | 值/内容来源 | 仍需 dump 的部分 |
| --- | --- | --- | --- |
| `Resources/Configs/enuma.toml` | umi.toml（+ 官方键位表） | `uefi_fd` + `header_version` + `gzip/append_dtb` 取 umi；`base/kernel_offset/tags_offset/pagesize` 取 cas（显式声明）并标注 pipa 的默认值说明；`requires_kernel_header` 三者一致 | FD base/size、header_version 的实机确认 |
| `enuma.dsc` | umi.dsc（Defines/PcdsFixedAtBuild/LibraryClasses/Components 的分段顺序） | `SOC_TYPE = 2` 取 pipa；栈地址取 umi+pipa；NV/capsule PCD 取 umi；null 库组合取 umi；`MemoryMapLib`/`ConfigurationMapLib` 取自有骨架 | MemoryMap 表、ConfigurationMap 表（uefiplat.cfg）、SMBIOS 串、FrameBuffer 尺寸 |
| `enuma.fdf` | umi.fdf（三 include + UniXNU INF） | `FILE FV_IMAGE` GUID 沿用上游 `9E21FD93-…`；BootManagerMenuApp/MsBootPolicy/Logo 标注为 umi 来源 | `Include/{APRIORI,DXE,RAW}.inc` 的厂商条目 |
| `Include/DXE.inc` | umi/pipa 的 A 段（EDK II 通用）+ 已写好的 UniXNU 段 | UniXNU 四行取自 uefi-pipa fork（真机无关）；厂商段留 TODO，附 pipa 的模块名作**参考** | 全部厂商模块名与顺序 |
| `Include/APRIORI.inc` | umi 的 `Include/APRIORI-order.json` 佐证"顺序来自 stock 镜像" | 通用项取自 pipa/umi；厂商顺序待 UEFITool | 厂商顺序 |
| `Include/RAW.inc` | pipa/umi（cas 无此文件） | `uefiplat.cfg` 的 RAW 条目形态取自官方指南 | 每个 RAW 文件的 stock FFS GUID |
| `Include/ACPI.inc` | 无模板（三者都没有 ACPI.inc；pipa 用 `pipa/AcpiTables.inf`） | 占位 + 说明 | 是否/如何提供 ACPI 表 |
| `Library/MemoryMapLib`、`Library/ConfigurationMapLib` | 三者同构（umi/cas/pipa 都有同名库） | 签名/结构照抄，表全部 TODO + `#error` | uefiplat.cfg 的 MemoryMap 与 `[ConfigParameters]` |
| `Library/RamoopsLogLib` | pipa（逐字复制） | 同 uefi-pipa 分支 `pipa`（12,611 B `.c` + 451 B `.inf`） | ramoops 基址 `0xB0000000` 与 PON arbiter 需按 enuma 核实；另见 umi 的 `RamoopsSerialPortLib` |
| `DeviceBuild.py` | pipa（逐字复制） | 同 uefi-pipa | `PackagesSupported`/`Scopes`（改为 enumaPkg/enuma）与 `GetRequiredSubmodules()` 的路径 |

## 4. umi 在真机上踩过、enuma 大概率会遇到的坑（摘自 umiPkg/README.md）

1. **显示是最大的坑**：vendor `DisplayDxe` 在 umi 上走到 DSI completion polling timeout（status 0xD）、屏幕全黑（README.md:19-24）；最终靠**自研 InheritedFbDxe**（只写 ABL 留下的保留帧缓冲）让菜单出画（:24-27）。→ enuma 是 11 吋双 CPhY 面板，建议把这条"继承帧缓冲"路线作为计划内备选，而不是意外。
2. **HALIOMMU 会让 DisplayDxe 卡死**：umi 的 attempt 04/05 走了"临时移除 HALIOMMU"的受控实验（:9-13）。→ 若是 SMMU/IOMMU 相关问题，这是已知的排查手法（注意 enuma 有自己的 SMMU 配置）。
3. **Capsule Arch 缺失会让 DXE 停在 BDS 之前**（:14-16）→ 这也是 umi 加 `DxeCapsuleLibNull` + 两个 capsule PCD 关闭的原因；enuma 已照抄这套 PCD。
4. **BDS 菜单**：umi 用 `BootManagerMenuApp` + `MsBootPolicy` 保证没有 OS 时有菜单（`umi.fdf:44-45`）；UniXNU 自己的菜单是另一层。
5. **构建不可只用上游通用指令**：umi 根 README:30-33 明确"Generic upstream build instructions alone do not [work]"——要用配套设备仓库的构建器；enuma 同理，见设备仓库 `scripts/`。

## 5. 来源清单（本次实取）

| 文件 | 来源 URL | 大小 |
| --- | --- | --- |
| umi.toml | `https://raw.githubusercontent.com/reAAAq/uefi-umi/main/Resources/Configs/umi.toml` | 278 B |
| umi.dsc / umi.fdf | `.../main/Platforms/Xiaomi/umiPkg/umi.dsc` / `umi.fdf` | 3,980 B / 2,294 B |
| umi DXE.inc | `.../main/Platforms/Xiaomi/umiPkg/Include/DXE.inc` | 1,930 B |
| umi README（包） | `.../main/Platforms/Xiaomi/umiPkg/README.md` | 11,690 B |
| umi README（根） | `.../main/README.md` | 2,528 B |
| cas.toml | `https://raw.githubusercontent.com/ht94911/uefi-cas/cas/Resources/Configs/cas.toml` | 903 B |
| cas.dsc / cas.fdf | `.../cas/Platforms/Xiaomi/casPkg/cas.dsc` / `cas.fdf` | 3,660 B / 1,380 B |
| pipa.toml / pipa.dsc / pipa.fdf / DXE.inc | `https://raw.githubusercontent.com/Project-Silicium/Mu-Silicium/main/Platforms/Xiaomi/pipaPkg/...` 与 `https://raw.githubusercontent.com/xVanTuring/uefi-pipa/pipa/...` | 314 B / 2,331 B / 4,465 B / 5,107 B（fork 版 3,333/4,570/5,572 B） |
| 官方设备表 / 官方键位表 | `docs/devices/index.md:23,35,44`、`docs/porting/uefi-firmware.md:37-70` | — |