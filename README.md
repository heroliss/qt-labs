# Liss Qt Labs

三个独立的 C++ / Qt Widgets 桌面应用：可视化操作系统、Huffman 无损压缩、科学表达式计算器。它们来自早期自写 Qt 工程，保留算法脉络，以当前 C++20、Qt 6.12 和 CMake 重新整理为可运行的 Windows 软件。

[作品介绍](https://heroliss.github.io/liss-works/projects/qt-labs/) · [Windows 下载](https://github.com/heroliss/qt-labs/releases/latest)

## 三个应用

| 应用 | 程序 | 可以体验什么 |
| --- | --- | --- |
| OS Lab | `LissOsLab.exe` | 创建进程、时间片轮转、运行/就绪/阻塞状态切换、连续内存分配、内存碎片与紧凑。 |
| Huffman Lab | `LissHuffmanLab.exe` | 拖入文件，查看字节频率、前缀编码、平均码长与信息熵，压缩后再解压并核对 SHA-256。 |
| Calculator | `LissCalculator.exe` | 科学表达式、DEG/RAD、X/Y/Z 变量、记忆与 Ans、计算记录和可见的双栈求值过程。 |

解压 Windows 发布包后，直接运行其中任一 exe；三个应用共享同目录的 Qt 和 MSVC 动态运行库。不需要安装 Qt、Python、.NET，也不连接网络。请保留 DLL、`plugins` 和 `third-party` 文件夹。

## OS Lab：把状态与内存放在同一张图中

- **时间片轮转**：就绪队列按 FIFO 排列；可单步执行、自动运行、阻塞和唤醒。
- **最大空闲区分配**：保留原工程使用的 worst-fit 策略；释放后的相邻空闲区域由内存模型计算得出。
- **内存紧凑**：移动分配区并同步 PCB 首地址，同时保留进程 ID、CPU 进度和队列次序；可以手动触发或在需要时自动进行。
- **可检查的模型**：每个活跃进程恰好属于一个状态队列；PCB、内存图和空闲量保持一致。数据结构采用值语义与 RAII。

这是教学用的单核调度与连续内存模型，默认 128 块、每块标示为 4 KiB。它不会访问系统进程或真实物理内存。

## Huffman Lab：从字节统计到可信的往返恢复

- **自写编码器**：256 种字节频率、最小堆合并、前缀码与高位优先的比特打包；频率相同按最小字节值排序，输出可重复。
- **流式文件处理**：64 KiB 分块读取，64 位长度；工作线程执行，界面显示阶段和进度，可随时取消。
- **原子保存**：通过 `QSaveFile` 提交完整结果，失败或取消时不覆盖已有目标。
- **完整性校验**：压缩二次读取时核对文件未变；解压检查长度、频率、比特数、填充位与 SHA-256。
- **可解释的压缩率**：同时展示头部开销、实际文件大小、平均码长与信息熵；对随机数据或小文件，体积可能增加。

当前文件格式为 **LHF2**，扩展名 `.lhf`，一个文件自带频率表与摘要。它不读取旧版分离的 `.hkey` / `.hcode` 文件，也不是 ZIP/RAR 格式。格式说明见 [docs/lhf2.md](docs/lhf2.md)。

## Calculator：保留手写求值器的透明度

采用运算符栈将中缀表达式转为逆波兰式，再使用数值栈求值，无 `eval`、脚本引擎或第三方数学解析器。

- `+ - * / % ^`、一元正负号、科学计数法、圆括号；`[expr]` 表示绝对值。
- `^` 右结合：`2^3^2 = 512`，`-2^2 = -4`，`2^-3 = 0.125`。
- `sin/cos/tan`、反三角和双曲函数、`sqrt/abs/ln/lg/ceil/floor/exp`。
- `pi/e/phi/C/NA/G` 常量、X/Y/Z、M 与 Ans；支持 `2pi`、`2(3+4)` 等隐式乘法。
- 明确显示错误位置与原因：除零、定义域、括号配对、未知函数、溢出与输入长度边界。

采用实数浮点计算，结果限制为双精度有效范围，界面最多显示 15 位有效数字；不提供任意精度代数计算。

## 构建与验证

环境：Windows 10 1809 或更新版本（x64）、Visual Studio 2026 C++ 桌面工具、CMake 3.24+、Qt 6.12.0 MSVC 2022 x64。Qt 的 MSVC 二进制与当前 MSVC 14.51 工具链组合已实际构建验证。

已有 Qt SDK 时可直接构建。需要隔离安装 SDK 和准备分发用 Qt 源码时，可运行：

```powershell
python -m venv .toolchain/venv
./.toolchain/venv/Scripts/python.exe -m pip install -r scripts/tooling-requirements.txt
./.toolchain/venv/Scripts/python.exe scripts/install_qt.py
./.toolchain/venv/Scripts/python.exe scripts/qt_notices.py
```

这两个下载脚本仅获取固定版本官方 Qt 文件并校验摘要。随后执行：

```powershell
# QT_ROOT 指向自己的 Qt SDK；默认使用本仓库 .toolchain 下的隔离安装。
./scripts/build.ps1 -QtRoot 'C:/Qt/6.12.0/msvc2022_64'
./scripts/package.ps1 -QtRoot 'C:/Qt/6.12.0/msvc2022_64'
```

构建脚本会执行 `QtLabsChecks.exe`。验收包含 OS 的 10,000 次随机转换与内存守恒、Huffman 空文件/单字节/任意二进制/损坏/取消、计算器优先级和错误边界；完整报告写入发布包的 `core-acceptance.json`。

`--showcase` 会载入仍可继续交互的合成样例。OS 还支持 `--scenario compacted`、`--scenario finished`；计算器支持 `--scenario error`。`--capture <png>` 仅是开发用 QWidget 渲染导出；作品页正式截图另通过实际显示的窗口采集。

## 原始来源

`legacy/os`、`legacy/huffman`、`legacy/calculator` 保留原始 C++、头文件、Qt Designer UI 与 qmake 工程。`legacy/manifest.json` 记录逐文件 SHA-256，原备份目录保持只读。生成的 `ui_*.h`、构建产物、旧 exe、用户配置和私密数据未复制。

现代应用的源码位于 `src`，验证位于 `tests`；归档源码不参与构建。

## 第三方组件

Qt 6.12.0 采用动态链接；发布包保留 LGPL/GPL 及上游第三方声明，并附带相同版本未经修改的 Qt Base 完整源码。可以替换兼容的 Qt DLL；为调试 Qt 修改而进行的反向工程不受本项目限制。具体路径、上游地址和校验值见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。本仓库没有额外选择主项目开源许可证。
