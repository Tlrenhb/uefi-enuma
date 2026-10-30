# uefi-enuma-skeleton — enumaPkg（Mu-Silicium 平台包）骨架

> 任务：task-13 起，**task-15 换模板重写**。作者：teammate `docs-claims`。日期：2026-10-23（Asia/Shanghai）。
> 目标设备：小米平板 5 Pro 5G（**enuma**，SM8250-AC / 骁龙 870，Kona）。
> **主模板**：[reAAAq/uefi-umi](https://github.com/reAAAq/uefi-umi)（小米 10，SM8250）—— 官方设备表里唯一记录到
> "UEFI 菜单 + payload 卷 + DarwinBoot + UFS 根卷 + launchd + 持久 Data" 的 SM8250 端口（`docs/devices/index.md:23`）。
> 第二参考：[ht94911/uefi-cas](https://github.com/ht94911/uefi-cas)（小米 10 Ultra，真机到菜单，header v2 + 自有 SEC/显示库）。
> 第三参考：[xVanTuring/uefi-pipa](https://github.com/xVanTuring/uefi-pipa)（平板 6，**从未上机**；仅用于 SM8250-AC 的 `SOC_TYPE = 2` 与结构对照）。
> 三模板逐项对照与每个选择的理由：**[TEMPLATES.md](TEMPLATES.md)**。

> **当前状态（2026-10-31）：enumaPkg 已在实机上把 UniXNU 菜单跑起来了。**
> 设备全程只用 `fastboot boot` 热启动，不写任何分区。当前屏幕上：wordmark、4 个条目、选中高亮、
> 倒计时与状态行全部正常；剩 `Connect ConIn Failed`（没有输入设备）、`no boot entry found`（没有 payload）、
> `drivers 0 / kits 0 / acpi 0`（UniXNU 侧注入链未启动）三件事。
> 设备侧仓库与上机记录：[Tlrenhb/UniXNU-enuma](https://github.com/Tlrenhb/UniXNU-enuma)。

## 0. 让它跑起来的两条平台级修正（2026-10-30）

### 0.1 `Library/MemoryMapLib/MemoryMapLib.c` —— 补上 ramoops 窗口

```
{"Ramoops", 0xb0000000, 0x400000, AddMem, SYS_MEM, SYS_MEM_CAP, Reserv, WRITE_BACK_XN},
```

原因：`SiliciumPkg/Library/FrameBufferSerialPortLib` 的 `SerialPortWrite()` 第一句就是
`RamLogWrite (Buffer, NumberOfBytes)`，而 `RamoopsLogLib` 写的是 `RAMOOPS_CONSOLE_BASE = 0xB0000000`。
原表最后一行 `{"Kernel", 0xa0000000, 0x10000000}` **正好结束在 0xB0000000**，于是 ramoops 基址一个字节
都没被映射；`ArmEnableMmu()` 一开，第一次写控制台就 data abort，固件永远停在 `MU:09 enabling mmu`。

> 别和 `{"AOSS", 0xb000000, 0x4000000}` 搞混：那是 **0x0B000000**（7 位），AOSS 寄存器块，不是 ramoops。

### 0.2 `enuma.dsc` —— ASSERT 只打印，不死循环

```
gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0E
```

默认 `0x0F` 下 `ASSERT_EFI_ERROR` 失败会 `CpuDeadLoop()`，屏幕上一个字都不增，且随后的电源键复位会被
`RamoopsLogLib` 的 PON 设置变成温重启直接回 fastboot。`0x0E` 保留打印、去掉死循环。

### 0.3 控制台库的额外加固（由构建流水线注入，不改本仓库）

`FrameBufferSerialPortLib.c` 在流水线里被补三处：控制台写路径不再依赖 `RamLogWrite`；显存区
（`LocateMemoryRegionByName ("Display Reserved")`）与几何（`LoadGeometry` 里的 hand-off）只在第一次调用时解析并缓存；
光标从 `FbBase + FbLength - 8`（区尾部）移进库静态。这样控制台在任何 MMU 配置下都不会因为摸到区尾部而踩空。

---

**当前状态：只差 enuma 自己的 dump 产物即可构建。** 结构、库签名、UniXNU 集成、NV/capsule 策略、SoC 选择都已按
**真机验证过的 umi 配置**落地；仍缺的只有"只能从 enuma 固件里取"的东西（见 §3），并且两个设备相关的 `.c` 表仍带
**故意 `#error`** —— 不填就不出固件，这是刻意的。

---

## 1. 这是什么（放进哪个仓库）

不是独立仓库，而是 **`uefi-enuma` 的 delta**。最省事的落地方式：把 `uefi-umi`（或 `uefi-pipa`）fork 成 `uefi-enuma`，
再把本目录的文件按路径覆盖进去。

```sh
git clone https://github.com/reAAAq/uefi-umi uefi-enuma        # 主模板：真机到桌面的那份
cd uefi-enuma
# 覆盖（路径与 checkout 一一对应）：
cp -r <skeleton>/Resources/Configs/enuma.toml      Resources/Configs/
cp -r <skeleton>/Platforms/Xiaomi/enumaPkg         Platforms/Xiaomi/
cp    <skeleton>/scripts/extract-vendor-dxe.sh     scripts/
```

> 为什么是 umi：它的包结构（`Include/{APRIORI,DXE,RAW}.inc` + `Library/{MemoryMapLib,ConfigurationMapLib}`）与本骨架
> 同构，改动面小；它的 `[uefi_fd]` 值有出处（"Matched to the uefiplat.cfg extracted from this phone's xbl_5"）且与 pipa
> 一致（两台独立 SM8250）；它的 NV/capsule/null-lib 组合就是走到 launchd 的那套。
> 不用 cas 做底：它的 header v2、自有 `Sec/`、`CasDisplayLib`/`CasInputLib` 是它自己的重活，复制过来等于另起一套。
> 不用 pipa 做底：pipa 自述 "not run on hardware"（`docs/devices/pipa.md:7`）。

## 2. 从 umi 学到的、enuma 大概率会遇到的坑

| 现象（umi 真机） | 出处 | enuma 的对策 |
| --- | --- | --- |
| vendor `DisplayDxe` 进入后不返回、DSI completion polling 超时（status 0xD）、屏幕全黑 | `umiPkg/README.md:19-24` | `USE_CUSTOM_DISPLAY_DRIVER = 0`（SimpleFbDxe）先试；失败则按 umi 的 **InheritedFbDxe 模式**（只写 ABL 留下的保留帧缓冲）为 enuma 写一个 GOP 驱动 |
| HALIOMMU 让 DisplayDxe 卡死（受控实验：临时移除后成功） | `:9-13` | 面板不通时，把 IOMMU/SMMU 作为可疑项做同样的受控实验 |
| 缺 Capsule Arch 导致 DXE 停在 BDS 之前 | `:14-16` | 已照抄 umi：`DxeCapsuleLibNull` + 两个 capsule PCD 关闭（见 `enuma.dsc`） |
| 菜单导航需要电源键映射为 ENTER | `:28` | 六个必须的 DXE 补丁之一（见 §4） |
| "Generic upstream build instructions alone do not [work]" | `README.md:30-33` | 用配套设备仓库的构建器（`UniXNU-enuma-skeleton/scripts/`），不要只照上游通用指令 |

## 3. 已经填好的（真机/多源验证）与仍等 dump 的

**已填（来源写在文件注释里）**

| 项 | 值 | 依据 |
| --- | --- | --- |
| `[uefi_fd] base/size` | `0x9fc00000` / `0x300000` | umi（按本机 uefiplat.cfg 匹配过）与 pipa 逐字节相同 |
| `[boot_image] header_version` | `1` | umi.toml + 官方 umi 设备页 "Header v1, 4096-byte pages" + pipa |
| `kernel_compression` / `append_dtb` | `gzip` / `true` | umi + pipa + sheng 三者一致 |
| `base` / `kernel_offset` / `tags_offset` / `pagesize` | `0` / `0x8000` / `0x1e00000` / `4096` | cas 显式声明 + pipa 的"mkbootimg 默认值"说明 + 官方 umi 页 |
| `SOC_TYPE` | `2`（SM8250-AC） | enuma 是骁龙 870；三模板里只有 pipa 声明 -AC |
| `PcdSystemMemoryBase` | `0x80000000` | umi/cas/pipa 三者一致 |
| `PcdCPUCoresStackBase` / 栈大小 | `0x9FF90000` / `0x40000` | umi + pipa 一致 |
| NV / capsule 策略 | `PcdEmuVariableNvModeEnable TRUE` 等 4 项 + 2 个 feature flag | umi（真机到桌面） |
| 空实现库组合 | `SiliciumNullPkg` 的 BootManager/BootOptions/AcpiTableUpdate/RamManager + `DxeCapsuleLibNull` | umi |
| `KonaPkg` SoC 层 | `!include KonaPkg/KonaPkg.dsc.inc` | umi + pipa |
| UniXNU 集成 | `UniXNUPkg/{Application,Driver}` + `ShengBootMemoryDxe` + `DarwinBoot` | uefi-pipa fork（与 umi.fdf:42-43 一致） |
| `[kernel] archives` / PACKAGES_PATH 形态 | 见设备仓库 `board.toml` 与 `../UniXNU-enuma-skeleton/board.toml` | 官方 `docs/porting/uefi-firmware.md:76-90` |

**等 dump（每项都带获取方法，在文件里是 `# TODO(enuma)`）**

1. **`uefi_fd` base/size 的实机确认** —— 读 enuma 自己的 `uefiplat.cfg`（MemoryMap 的 "UEFI FD"），必须与 `MemoryMapLib.c` 的 "UEFI FD" 行和 BootShim 的 `-DFD_BASE` 自洽。
2. **`header_version` 的实机确认** —— 读 stock boot.img 的 `ANDROID!` 头，或用 v1 冒烟启动。
3. **`PcdCPUCoresStackBase`** —— uefiplat.cfg 的 "UEFI Stack"。
4. **`Library/MemoryMapLib/MemoryMapLib.c`** —— uefiplat.cfg 的 MemoryMap（DDR / Other / Register 三组）。带 `#error`。
5. **`Library/ConfigurationMapLib/ConfigurationMapLib.c`** —— uefiplat.cfg 的 `[ConfigParameters]`（含 fuse/IMEM 地址）。带 `#error`。
6. **`Include/{APRIORI,DXE,RAW}.inc`** —— UEFIReader 的产物：厂商模块名与顺序（UEFITool）、每个 RAW 文件的 stock FFS GUID。
7. **SMBIOS 串 / FrameBuffer 尺寸** —— 实机（`fastboot getvar`、stock SMBIOS、stock DT、`wm size`）。
8. **`RamoopsLogLib` 的 ramoops 基址 `0xB0000000` 与 PON arbiter** —— 目前是从 pipa 逐字复制，需按 enuma 的 stock DT 核实。
9. **是否移植 umi 的 `RamoopsSerialPortLib` / `UmiResetSystemLib` / `UmiSpmiLib` / `UmiRpmhLib`** —— 前者是 EDK II 串口重定向（真机排障回路），后三者是板级 PMIC/RPMH/复位实现。
10. **`DeviceBuild.py`** —— 逐字复制自 pipa，需把 `PackagesSupported`/`Scopes` 改成 enumaPkg/enuma。
11. **ACPI 表** —— 是否/如何提供（`Include/ACPI.inc` 里写了两种处理方式）。
12. **vendor 二进制从哪来** —— `Project-Silicium/Device-Binaries` 里**没有 enuma/elish 目录**（76 个机型目录，有 pipa/nabu），必须自造 drop。

## 4. 六个必须的 DXE 补丁（真机经验，缺一不可）

| 模块 | 作用 | 来源 |
| --- | --- | --- |
| `ButtonsDxe` | 电源键映射为 **ENTER**，否则菜单无法导航 | `Device-Binaries/pipa/README.md:9-13` |
| `UsbConfigDxe` | `ExitBootServices` 后**不** deinit USB | `:15-19` |
| `TzDxeLA` | 不再二次通知/加载 TZ applet（已加载过） | `:21-25` |
| `FeatureEnablerDxe` | 开时钟函数恒返回 Success（重复开时钟会**丢帧缓冲**） | `:27-31` |
| **`UsbMsdDxe`** | 内部存储按**硬盘**而非可移除 U 盘上报 | `:33-37` |
| **`DALSYSDxe`** | `\_SB_.UFS0` 的 `CacheCoherency = 1`（内部存储稳定性） | `:39-43` |

脚本 `scripts/extract-vendor-dxe.sh` 第 5 步会核对这六个（`PATCH_NAMES` 已含全部六项）；README/state 与脚本保持一致。

## 5. 构建与迭代

```sh
export UNIXNU_UEFI=$PWD/uefi-enuma
UNIXNU_UEFI=$PWD/uefi-enuma ./unixnu build firmware -D ../UniXNU-enuma
UNIXNU_UEFI=$PWD/uefi-enuma ./unixnu bootimg  -D ../UniXNU-enuma \
    --dtb <enuma.dtb> --ramdisk none -o out/enuma/boot/boot-menu-round.img
fastboot boot out/enuma/boot/boot-menu-round.img
```

官方验收关键字：屏幕出现 UniXNU 菜单 + 日志出现 `UNIXNU: menu: shown`（`docs/guide/debugging.md:98`、
`docs/porting/bring-up.md:31-34`）。逐步落地与失败排查见 `README.md` §2/§3 与 `TEMPLATES.md` §4。

## 6. 许可

沿用上游 BSD-2-Clause-Patent。逐字复制的文件（`Library/RamoopsLogLib/*`、`DeviceBuild.py`）来源与大小记录在
`state.md` §2，并已在文件头加了 TODO 标注。