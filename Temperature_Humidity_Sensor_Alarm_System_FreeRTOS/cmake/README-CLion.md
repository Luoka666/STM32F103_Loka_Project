# CLion 编辑、Keil 构建

打开本项目根目录，使用普通 CMake 工程。CMake 3.20 及以上即可；现有
CLion MinGW 工具链可用于加载和索引，无需配置固件链接、烧录或调试。

1. 在 `Settings > Build, Execution, Deployment > CMake` 中启用当前 Profile。
2. 执行 `Tools > CMake > Reset Cache and Reload Project`，等待 CMake 加载和索引完成。
3. 查看 `CLionIndex` 的代码上下文，检查 `main.c` 和六个 `Tasks/task_*.c`。
   应能跳转到 `GPIO_InitTypeDef`、`RCC_APB2PeriphClockCmd`、`xTaskCreate`、
   `QueueHandle_t`、`SemaphoreHandle_t` 的定义。
4. 如果加载成功后仍显示旧诊断，先确认文件使用 `CLionIndex` 上下文，再执行
   `File > Invalidate Caches` 并重启。此操作不是常规加载的必要步骤。

`CLionIndex` 是 `OBJECT EXCLUDE_FROM_ALL` 目标，用于提供 CMake 文件模型和
`compile_commands.json`，默认构建不会编译它，也没有固件链接目标。
仍通过 `Project.uvprojx` 在 Keil 中编译、烧录和调试。不要将索引目标或其
编译命令用作固件构建配置。

索引设置使用 C99，包含全部六个任务、主程序、驱动、标准库和 FreeRTOS 内核。
Keil 当前用户宏为 `USE_STDPERIPH_DRIVER`；索引另外明确设置芯片所需的
`STM32F10X_MD`（现有 Keil 依赖记录也包含该设备宏）。Keil 工具还定义了
`__UVISION_VERSION=524`，但当前源码没有使用它，因此不加入索引配置。
不会伪造 `__CC_ARM` / `__ARMCC_VERSION` 等编译器内建宏。
64 位 MinGW 下仅给索引目标添加 `-m32`，让指针宽度匹配 STM32，避免标准库和
内核的指针转换误报；该设置不需要链接 32 位运行库，也不影响 Keil。

当前项目只有 RVDS/ARM_CM3 移植层。`CLionIndex.cmake` 每次配置从原始
`portmacro.h` 生成构建目录下的 `clion-index-include/portmacro.h`：保留真实类型和
API 宏，把五个 Keil 汇编内联函数变成声明，并补充三个 Keil intrinsic 声明。
它不实现调度器，不用于 Keil，也不修改原始 FreeRTOS 文件。临界区 API 的跳转
会落到生成的声明；需要阅读实现时打开原始 RVDS 文件。

`port.c`、`core_cm3.c` 和 MD 启动文件只附加供阅读，不交给宿主编译器。
这些底层文件的 Keil 汇编仍可能显示编辑器诊断；本配置不保证其汇编静态分析。
`stm32f10x_it_old.c` 包含重复的异常处理函数，且不在当前 Keil 文件清单中，
因此不作为活动索引源；`warning_system.c` 同样不在当前 Keil 清单中。
文件全部保留。`stack_macros.h` 和 `StackMacros.h` 是两个不同的实际文件。

日后在同一目录编辑同一套源码。增删 C 文件时同步 Keil 文件清单与根目录的
`index_sources`；头文件由限定目录的 glob 自动发现。Keil 宏、include 路径或
FreeRTOS 移植层变更后，也应同步检查 CMake 配置并重新加载。
使用宿主 MinGW 时，指令集及编译器特有行为与 ARM 不完全相同；ABI、
汇编、链接、实际时序和硬件正确性以 Keil 构建及硬件测试为准。
