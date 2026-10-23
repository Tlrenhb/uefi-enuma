# 六条 pipa DXE 补丁 → 对 enuma 这份 drop 的可操作步骤

> task-20 交付物。原始说明：`Project-Silicium/Device-Binaries/pipa/README.md`（**手机的** drop 记录，
> 设备 Xiaomi Pad 6、版本 `OS2.0.12.0.UMZMIXM` / **`BOOT.XF.3.2-00304-SM8250-2`**）。
> 本文把它转写成对 **enuma 自己的 drop** 的可操作步骤：
> drop 根 = `/tmp/fw2/uefi-out/xbl.elf/BOOT.XF.3.2-00304-SM8250-2/`（构建号**与 pipa 的完全相同**），
> ROM = `enuma_images_OS1.0.3.0.TKZCNXM_13.0`，`uefiplat.cfg` sha256 `29affca4…c610`。
>
> **必须记住的前提**：这 6 条是**手机的记录**。平板是否出现同样症状、同样的二进制改动是否成立，**只有实机才能确认**。
> 因此第一步不是"照着改"，而是"先构建未打补丁的固件，看症状"（见 §3 的推荐顺序）。

---

## 1. 先验事实：Device-Binaries/pipa **并没有**提供打好补丁的二进制

这一点决定了整条工作流。2026-10-23 对三对文件做了 sha256 比对：

| 模块（.efi） | enuma drop | Device-Binaries/pipa | 结果 |
| --- | --- | --- | --- |
| `ButtonsDxe.efi`（40,960 B） | `36212d2694b078c0…` | `36212d2694b078c0…` | **完全相同** |
| `UsbMsdDxe.efi`（53,248 B） | `4a95145ce5356027…` | `4a95145ce5356027…` | **完全相同** |
| `FeatureEnablerDxe.efi`（32,768 B） | `0bad5a385b43c48e…` | `0bad5a385b43c48e…` | **完全相同** |

（其余三个模块 `UsbConfigDxe` / `TzDxeLA` / `DALSYSDxe` 未做比对，见 §4 的 TODO。）

**结论**：`Device-Binaries/pipa` 里这几个模块就是**原厂字节**，它的 README 是"**改动记录**"，不是"现成补丁包"——
仓库里没有 `.patch` 文件，也没有打过补丁的 `.efi`。所以：
- **不存在"直接抄过来就完事"的路径**；
- 每条补丁都要在**本 drop 的 .efi 上**定位并修改，再记录 before/after 哈希；
- 好消息是两边构建号相同（`BOOT.XF.3.2-00304-SM8250-2`），同一个 SoC 家族里这些**通用模块**应该是同一份代码，
  所以 pipa 记录的经验（症状与改法）在 enuma 上大概率成立——**但"大概率"不是"已验证"**。

---

## 2. 六个目标模块在本 drop 里的确切位置与文件形态

目录：`/tmp/fw2/uefi-out/xbl.elf/BOOT.XF.3.2-00304-SM8250-2/QcomPkg/Drivers/<Module>/`
（落到构建树后前缀变成 `Binaries/enuma/QcomPkg/Drivers/<Module>/`，见 `Include/DXE.inc`。）

| 模块 | 目录内文件 | .efi 大小 | 补丁目的（原文） | 原作者 |
| --- | --- | --- | --- | --- |
| `ButtonsDxe` | `.inf` + `.efi` + `.depex`(72 B) | 40,960 B | 电源键从 SUSPEND 改映射为 **ENTER**，才能用电源键导航 UEFI 菜单 | Gustave Monce |
| `UsbConfigDxe` | `.inf` + `.efi` + `.depex`(72 B) | 94,208 B | `ExitBootServices` 例程不再 deinit USB（Windows/Linux/UEFI 里 USB 才能用） | Gustave Monce |
| `TzDxeLA` | 在 `Drivers/TzDxe/`：`TzDxeLA.{inf,efi,depex}`（同目录还有 `ScmDxeLA.*`） | 61,440 B | 全局变量改为 `0x1`，使"通知 TZ 并加载 applet"的函数不再被调用（Common/Keymaster applet 已加载） | N1kroks |
| `FeatureEnablerDxe` | `.inf` + `.efi`（无 `.depex`） | 32,768 B | 打开时钟的函数改为**恒返回 Success**（再开一次会丢 framebuffer） | N1kroks |
| `UsbMsdDxe` | `.inf` + `.efi` + `.depex`(2 B) | 53,248 B | Removable 状态改为 **Non-Removable**：内部存储按硬盘上报 | N1kroks |
| `DALSYSDxe` | `.inf` + `.efi` + `.depex`(18 B) | 344,064 B | `\_SB_.UFS0` 节点的 **CacheCoherency = 1**（存储稳定性） | N1kroks |

六个模块**都在本 drop 中存在**（已逐一 `ls` 核对），所以 DXE.inc 的厂商清单不会因为缺模块而失败。

---

## 3. 推荐顺序：先看症状，再按症状打补丁

1. **第一轮：不打任何补丁**（`Binaries/enuma/` 直接用 drop 的原厂字节），构建 + `fastboot boot`。
2. 对照下表判断到底命中哪一条，只打**命中的那几条**：

| 症状（实机可观测） | 对应补丁 | 验收 |
| --- | --- | --- |
| UniXNU 菜单出现但**电源键不能选中/确认** | `ButtonsDxe` | 电源键能当 ENTER 用（音量键上下移动） |
| `ExitBootServices` 之后 **USB 掉线**（键盘/存储/USB 网卡消失） | `UsbConfigDxe` | 交给 OS 后 USB 仍在 |
| 卡在 TZ/TzDxe 阶段、日志显示 applet 重复加载/超时 | `TzDxeLA` | 不再二次加载 applet，顺利进 BDS |
| 固件启动过程中**屏幕黑掉（背光还在）** | `FeatureEnablerDxe` | 时钟不被二次使能，framebuffer 保住 |
| 内部存储被 OS 当成**可移动 U 盘** | `UsbMsdDxe` | 报成固定硬盘 |
| 存储读写不稳定 / 报 cache 一致性错误 | `DALSYSDxe` | `\_SB_.UFS0` 的 CacheCoherency = 1 |

3. **第二轮起**：打补丁 → 记录 before/after sha256 → 重测同一条验收。
   **不要**一次性全打：六个改动互相掩盖症状，出问题时无法定位。

---

## 4. 单条补丁的操作模板（以 `ButtonsDxe` 为例，其余同构）

> README 只给了**语义**（"电源键映射为 ENTER 而不是 SUSPEND"），**没有给文件偏移**。
> 因此定位只能靠反汇编 + 与参考源码对照；下表如实写明"已知"与"未知"。

1. **备份 + 记录基线**
   ```sh
   M=QcomPkg/Drivers/ButtonsDxe
   cp -a "$M/ButtonsDxe.efi" "$M/ButtonsDxe.efi.orig"
   sha256sum "$M/ButtonsDxe.efi.orig" | tee "$M/ButtonsDxe.efi.sha256"
   # 基线（本 drop，已与 Device-Binaries/pipa 相同）：
   #   ButtonsDxe.efi     36212d2694b078c0…  (40,960 B)
   #   UsbMsdDxe.efi      4a95145ce5356027…  (53,248 B)
   #   FeatureEnablerDxe.efi 0bad5a385b43c48e… (32,768 B)
   ```
2. **定位改动点**：先看 `.inf` 拿到模块的源码路径/库依赖，再用反汇编工具
   （`llvm-objdump -d --arch-name=aarch64 ButtonsDxe.efi`，或 Ghidra/IDA）找目标函数：
   - `ButtonsDxe`：找按键码表/`TranslateKey`-类逻辑里 SUSPEND 与 ENTER 两个常量的比较分支；
   - `UsbConfigDxe`：找 `ExitBootServices` 事件回调里调用 deinit 的那个 `BL`；
   - `TzDxeLA`：找那个全局标志（初始化为 0 的那处），改成 1；
   - `FeatureEnablerDxe`：找 enable-clock 函数的返回路径，改为无条件 `mov w0,#0; ret`；
   - `UsbMsdDxe`：找 Removable/Non-Removable 的布尔常量或表项；
   - `DALSYSDxe`：找 `CacheCoherency` 属性名附近的取值（UFS0 节点由该驱动发布）。
   **本文件不含任何未经核对的偏移值**——写下未经反汇编证实的偏移，等于伪造证据。
3. **改字节**：只改必要的最小字节数；改完 `sha256sum` 记录新值，与 `.orig` 一起留在 `Binaries/enuma/` 里（可审计）。
4. **静态验证**：重新反汇编，确认目标分支/常量按预期变化，且没有破坏 PE 校验相关的结构
   （不要改 PE header/checksum 之外的区域；`.efi` 通常不校验自身 checksum，但**不要动 header**）。
5. **实机验证**：只按 §3 表格里对应的那一行验收；通过后把结论写回
   `../UniXNU-enuma-skeleton/provenance.md`（该文件是仓库的数值/验证等级台账）。

---

## 5. 尚未核实、需要补的三件事（如实列出，不猜）

1. `UsbConfigDxe.efi`、`TzDxeLA.efi`、`DALSYSDxe.efi` 与 `Device-Binaries/pipa` 的 sha256 比对**未做**
   （本文只比了 3 个）。若它们也完全相同，则进一步支持"该仓库不含打过补丁的二进制"这一结论。
2. 六条补丁的**具体字节偏移**：公开材料里没有，必须在本地反汇编得到——任何声称有现成偏移的说法都需要证据。
3. 平板（enuma）与手机（pipa/umi）的差异面：**按键**（平板只有电源+音量，无 Home）、**存储**（UFS 单实例 vs 手机多实例）、
   **USB**（USB 3.0 主/从角色与平板的 Type-C 配置可能不同）——这三处正是这六个补丁涉及的功能域，
   所以"pipa 需要"不等于"enuma 需要"。这也是 §3 推荐"先不打补丁"的原因。