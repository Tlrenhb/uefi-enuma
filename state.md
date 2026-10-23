# enumaPkg 完成度表（task-20 版：**可构建**）

> 日期：2026-10-23（Asia/Shanghai）。作者：teammate `device-port-facts`（承接 task-15 的 `docs-claims` 版本）。
> 状态口径沿用仓库 house rule（`../UniXNU-enuma-skeleton/provenance.md`）：**V1** 云端真构建通过 ｜ **V2** 有真机（他机）证据 ｜
> **V3** 公开一手来源 ｜ **待验** 只能等本机实机。**可构建** = 仓库里不再有故意 `#error`、厂商清单不再缺。
> 主模板 = `reAAAq/uefi-umi`（SM8250，真机到 launchd + 持久 Data）；第二参考 = `ht94911/uefi-cas`；结构参考 = `xVanTuring/uefi-pipa`（未上机）。

---

## 1. 本次（task-20）填入的所有值：**全部来自 enuma 自己的原厂固件**

**唯一来源**：官方 fastboot ROM `enuma_images_OS1.0.3.0.TKZCNXM_13.0` → `xbl.elf` → UEFIReader drop
（构建号 **BOOT.XF.3.2-00304-SM8250-2**，310 个文件）。
权威配置文件 `RawFiles/uefiplat.cfg`，**sha256 = `29affca455090a26b223ebae2c02bf294547bb699f9b65a891af3c186d99c610`**（7,953 B）。

| 文件 | 填入内容 | 状态 |
| --- | --- | --- |
| `Library/MemoryMapLib/MemoryMapLib.c` | **45 行** = `[MemoryMap]` 22 + `[RegisterMap]` 23，逐行照 cfg（脚本化生成后**逐行比对 cfg 完全相同**：45/45 identical） | V3（本机一手固件） |
| `Library/ConfigurationMapLib/ConfigurationMapLib.c` | **32 个数值项** = `[ConfigParameters]` 的全部数值行（`ConfigParameterCount = 64` 含注释与 5 个字符串项）；5 个字符串项以注释保留原文，未删 | V3 |
| `Include/DXE.inc` | drop 的 **91 条 INF → 90 条 + 2 条 UniXNU**（见 §2 转换规则） | V3 |
| `Include/APRIORI.inc` | drop 的 APRIORI DXE 块，**84 条 INF**（62 vendor + 18 core + 4 vendor-modified），顺序原样 | V3 |
| `Include/RAW.inc` | drop 的 **56 个 `FILE FREEFORM`**（GUID 逐字保留，路径改 `Binaries/enuma/RawFiles/`），顺序原样；含 `uefiplat.cfg` 与两个 k81 面板 XML | V3 |
| `Resources/Configs/enuma.toml` | `[uefi_fd] base/size` 由**本机 cfg 证实**（"UEFI FD", 0x9FC00000, 0x00300000）；补记 UEFI Stack / SEC Heap / CPU Vectors / MMU PageTables / FV Region / ABOOT FV / DXE Heap / Sched Heap / Display Reserved / Kernel 十个区间 | V3 |
| `enuma.dsc` | 栈地址 `0x9FF90000/0x40000` 由 cfg 的 "UEFI Stack" 证实；两个 Lib 的"内容待填"注释改为已填 + sha256 | V3 |
| `DeviceBuild.py` | 从 pipa 逐字副本改名：`enumaPkg` / `('enuma', ...)` / `enumaPkg/enuma.dsc` / `PRODUCT_NAME=enuma`（语法已校验） | V1（结构）／待验（构建） |

**关键交叉验证**：`UEFI FD` 行同时对齐三处 —— `enuma.toml [uefi_fd]`、`MemoryMapLib.c` 的 "UEFI FD" 行、BootShim 的 `-DFD_BASE`；
`UEFI Stack` 行对齐 `enuma.dsc` 的 `PcdCPUCoresStackBase/PrimaryStackSize`；`Display Reserved 0x9C000000+0x02400000`
覆盖内核 DTS 的 `cont_splash_region 0x9c000000+0x2300000`（见 `../UniXNU-enuma-skeleton/astide-notes.md` §5）。

---

## 2. DXE.inc 是怎么合并的（drop 生成版 + UniXNU 必需项）

drop 的 `DXE.inc`（13,794 B）把 INF 与 `FILE FREEFORM` 混在一个文件里。我们**机械转换**为 umi/pipa 的三片段布局
（`enuma.fdf` 同时 include 三者，若都留在 DXE.inc 会重复）：

1. `QcomPkg/**`、`MiPkg/**` → 前缀 `Binaries/enuma/`（**67 条**）；
2. 与 pipaPkg 的 DXE.inc 中**未加前缀**条目路径完全相同的 → 保持不加前缀（**19 条**，Mu/edk2 核心模块，
   来自 `Mu_Basecore` 而不是厂商镜像）；
3. 其余路径（看着像上游但被厂商改过的，**4 条**）→ 也加前缀：`RscRtDxe.inf`、`SCHandlerRtDxe.inf`、
   `ArmGic/ArmGicDxe.inf`、`FvSimpleFileSystemDxe.inf`（这 4 条已在 drop 中逐一确认存在）；
4. **排除** `QcomPkg/XBLCore/XBLCore.inf`（1 条）：它是厂商 XBL core，本端口用 BootShim + `SiliciumPkg/Sec` 取代它，
   包含它会与 SEC 阶段冲突；
5. 56 个 `FILE FREEFORM` 拆到 `RAW.inc`；
6. **UniXNU 必需项保留**并插在核心模块之后：`shengPkg/Drivers/ShengBootMemoryDxe/ShengBootMemoryDxe.inf`、
   `DarwinBootPkg/Driver/DarwinBoot.inf`（与 umi.fdf:42-43 同）；它们**不进 APRIORI**（pipa/umi 也不进）。

**可核查性**：所有加前缀的 71 条（DXE）/ 66 条（APRIORI）路径**都在 drop 中真实存在**（脚本核对 missing = 0）。
APRIORI 里没有 `QcomBds`：drop 的 `DXE.dsc.inc` 有它，但 pipa 的 DXE.inc 用 `MdeModulePkg` 的 `BdsDxe`，且 UniXNU 自己拥有 boot manager；
已文档化的回退开关写在 `Include/APRIORI.inc` 头部。

---

## 3. 六个 DXE 补丁：现在是**可操作步骤**

见新增的 `scripts/apply-pipa-dxe-patches.md`：逐条给出「哪个模块目录 / 改什么 / 怎么验证」，以及**为什么这是手机的记录、平板必须实机确认**。
六条：`ButtonsDxe`（电源键→ENTER）、`UsbConfigDxe`（EBS 后不 deinit USB）、`TzDxeLA`（不再二次加载 TZ applet）、
`FeatureEnablerDxe`（开时钟恒返回 Success）、`UsbMsdDxe`（内部存储报成硬盘）、`DALSYSDxe`（`\_SB_.UFS0` CacheCoherency=1）。
六个目标模块在本 drop 中**全部存在**（`Binaries/enuma/QcomPkg/Drivers/<Module>/`）。

---

## 4. 仍需**实机**确认的项（构建不再阻塞）

| # | 项 | 为什么要实机 | 怎么确认 |
| --- | --- | --- | --- |
| 1 | stock ABL 是否接受 `header_version = 1` | 头字段可离线读，接受度只能试 | `fastboot boot out/enuma/boot/boot.img`；若拒绝 → 改 0（`Resources/Configs/enuma.toml` 已留说明） |
| 2 | 面板变体（`42_02_0a` / `35_02_0b`） | drop 里两个 XML **都在**（`RawFiles/Panel_K81_*.xml`），实机装的是哪一支看 vendor info | Android 侧 `cat /sys/.../display_maker` 或 `tlmm 38` 电平 |
| 3 | 六补丁的**语义**是否对平板成立 | 它们来自 pipa（小米平板 6）的 drop 记录 | 打补丁后：菜单能否用电源键导航、USB/存储是否正常（见 `scripts/apply-pipa-dxe-patches.md`） |
| 4 | SMBIOS 串（retail model / SKU / board model） | `enuma.dsc` 里仍是 `TODO` | `fastboot getvar` / 原厂 SMBIOS |
| 5 | FrameBuffer 宽高（`PcdFrameBufferWidth/Height`） | cfg 里**没有**该数据 | 阶段 1 `fastboot boot` 后读 UEFI GOP 实际 mode（几何可由面板 DTS 推：1600×2560 竖装） |
| 6 | `RamoopsLogLib.c` 的 ramoops 基址 / PON arbiter | 沿用 pipa 的 `0xB0000000`；内核树三份 dtsi 里**没有** ramoops 节点 | stock dtbo 的 `ramoops`/PON 节点 |
| 7 | ACPI 表取舍 | pipa 无 `ACPI.inc` | 决策：删 `Include/ACPI.inc` 或提供 `enuma/AcpiTables.inf` |
| 8 | `[userdata]` LBA、`[payload] dram_size` | 见设备仓库 `provenance.md`（已由机主实测填入） | 已解决，此处仅交叉引用 |

---

## 5. 下一步（Phase 1 收尾判据）

1. 构建：`UNIXNU_UEFI=<uefi-enuma checkout> ./unixnu build firmware -D ../UniXNU-enuma`
   （`Binaries/enuma/**` 需先由 `scripts/extract-vendor-dxe.sh` 从同一个 drop + `clean_up.py` 落到位）。
2. FD 尺寸检查：`0x300000` 必须容得下（cfg 的 "UEFI FD" 窗口 = 768 个 4 KiB 块）；超了就说明厂商模块选多了。
3. `unixnu bootimg … --ramdisk none` + `fastboot boot` → 屏幕出现 UniXNU 菜单、日志出现 `UNIXNU: menu: shown`
   （官方 `docs/porting/bring-up.md:25-40` 阶段 2）。屏黑而日志正常 → 按 `enuma.dsc` 里 `USE_CUSTOM_DISPLAY_DRIVER` 的说明处理。
4. 以上通过后，本文件第 4 节的 1–7 项逐条实测，并把结果写回 `../UniXNU-enuma-skeleton/provenance.md` 的表格。