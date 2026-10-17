# CLion 编辑，Keil 编译调试

本目录的 `CMakeLists.txt` 仅提供代码索引：补全、跳转、头文件识别和静态分析。
`Project.uvprojx` 仍是固件工程入口；编译、链接、烧录和调试继续在 Keil 中操作。

## 打开与重新加载

1. 在 CLion 打开本目录；如果已经打开，在根目录的 `CMakeLists.txt` 上选择“加载 CMake 项目”。
2. 使用 CLion 自带的 MinGW 工具链即可；不需要安装 ARM GCC。
3. 已加载 CMake 后，可通过“工具 → CMake → 重新加载 CMake 项目”刷新配置。
4. 等待加载、索引完成，再编辑 `User/main.c`、`Hardware/MyCAN.c` 等文件。

索引目标名为 `CLionIndex`，芯片宏为 `STM32F10X_MD`，启用标准外设库。
包含路径与 Keil 工程一致：`Start`、`Library`、`User`、`System`、`Hardware`。

## 日常工作

- CLion 和 Keil 直接编辑/使用同一目录中的源码。切到 Keil 编译前，在 CLion 保存文件（Ctrl+S）。
- 新增 `User`、`Hardware`、`System`、`Library` 下的 C/H 文件后，重新加载 CMake 即可加入 CLion 索引。
- 新增的 C 文件还需要自行加入 Keil 工程组，才会参与 Keil 编译；CMake 不会修改 Keil 的文件列表。
- 当前 `MyCAN.c/.h` 是空文件，已加入 CLion 索引，但尚未列在 `Project.uvprojx` 中。需要编译 CAN 实现时，请在 Keil 的 Hardware 组添加 `MyCAN.c`。
- CMake 默认构建不编译这些嵌入式文件，也不生成固件；不要把 `CLionIndex` 当成 PC 程序运行。
- `Start/core_cm3.c` 和启动汇编仅供浏览，其 ARM 指令应由 Keil 处理。

本次配置不改动已有 C/H/汇编文件或 Keil 工程配置，也不实现 CAN 功能。
