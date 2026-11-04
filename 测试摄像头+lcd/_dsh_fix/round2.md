
---

# 第二轮：红屏（SCCB 读不到 ID）的继续排查

> 现象：把 SCCB 宏改成实际接线 PB10/PB11 后，仍然是"白屏一闪 → 立刻红屏 → 停住"。

红屏本身是自检在起作用：它表示 **SCCB 读不到 PID=0x76 / VER=0x73**，问题被限定在"MCCB 通信"这一层，
与屏幕、与图像数据无关。

## 8.1 第二个根因：整个工程从来没有调用过 `Delay_Init()`

`System\Delay.c`：

```c
static uint32_t fac_us = 0;          // 静态初值 0
void Delay_Init(uint8_t SYSCLK)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us = SYSCLK/8;
    ...
}
```

全工程搜索 `Delay_Init`，只有 `Delay.c` 里的这一处定义，**没有任何地方调用它**。
于是 `fac_us` 永远是 0，`Delay_us()` 里：

```c
uint32_t total_ticks = nus * fac_us;   // = 0
while (total_ticks > SYSTICK_LOAD_MAX) {...}   // 不进入
if (total_ticks > 0) {...}                      // 不进入 → 直接返回
```

**所有 `Delay_us()/Delay_ms()` 都是空操作。**

后果：SCCB 每一位之间没有任何间隔，SCL 周期只由函数调用开销决定（工程是 `-O0`，一次
`GPIO_WriteBit`+`Delay_us` 大约 0.4~0.6 µs）→ SCL 跑到 0.8~1.2 MHz，
**是 OV7670 SCCB 上限 400 kHz 的两三倍**，从机根本不应答，读回来就是 0xFF，于是红屏。

这也解释了为什么"单跑 LCD 一直没事"：LCD 是纯输出、软件 SPI 没有最小时序要求，
延时失效看不出来；而 SCCB 是双向握手协议，延时一失效立刻失败。顺带地，
`OV7670_Rest()` 的 100 ms 复位脉宽、`LCD_Init()` 的 120 ms 唤醒时间此前也全都没有生效。

**修复：**

1. `main()` 第一行显式调用 `Delay_Init((uint8_t)(SystemCoreClock / 1000000));`
2. `Delay_us()` / `Delay_ms()` 里加自愈保护：`if (fac_us == 0) { Delay_Init(...); }`，以后忘了调用也不会再退化；
3. SCCB 位延时保持 10 µs（约 33 kHz），稳稳低于 400 kHz 上限。

## 8.2 红屏不再是"猜"，改成把实测值打到屏上

`main.c` 里自检失败时不再单纯刷红，而是显示：

```
SCCB ERROR
PID=xx
VER=xx
LINE=x
SCL=PB10
SDA=PB11
```

其中 `LINE` 来自新增的 `SCCB_TestLine()`（先释放 SDA 读一次，再主动拉低读一次）：

| 屏幕显示 | 含义 | 下一步 |
|---|---|---|
| `PID=76 VER=73 LINE=0` | 通信正常 | 正常，会继续往下跑出图像 |
| `PID=FF VER=FF LINE=0` | SDA 空闲为高、能拉低，但摄像头不应答 | 时序/地址问题：把 `SCCB_W_SCL/SDA` 里的 `Delay_us(10)` 加大到 50 试试；仍不行则查模块供电（3.3V/2.8V）与 PWDN |
| `LINE=1` | SDA 释放后仍是低 | 没有上拉电阻 / SDA 对地短路 / 摄像头模块没供电 |
| `LINE=2` | SDA 主动拉低后仍为高 | 引脚接错、焊点没通、摄像头与板子没共地 |
| `PID=00 VER=00` | SDA 被拉死在低 | 同 `LINE=1` |

## 8.3 顺手加的：颜色字节序开关

`lcd.c` 顶部新增

```c
#define OV7670_SWAP_BYTES   0     // 颜色明显不对（偏紫/偏绿/红蓝颠倒）时改成 1
```

图像出来之后如果颜色不对，只改这一个数字即可，不用再改别的地方。

## 8.4 第二轮修改清单

| 文件 | 改动 |
|---|---|
| [`System\Delay.c`](System/Delay.c) | `Delay_us()` / `Delay_ms()` 增加 `fac_us==0` 自初始化保护 |
| [`User\main.c`](User/main.c) | 开头调用 `Delay_Init(SystemCoreClock/1000000)`；SCCB 自检失败时把 PID/VER/LINE 显示到屏上 |
| [`Hardware\SCCB.c`](Hardware/SCCB.c) | 新增 `SCCB_TestLine()` SDA 连线自检；引脚保持你实测的 PB10/PB11 |
| [`Hardware\SCCB.h`](Hardware/SCCB.h) | 增加 `SCCB_TestLine()` 声明，删掉从未实现的 `SCCB_WaitEvent()` |
| [`Hardware\lcd.c`](Hardware/lcd.c) | 新增 `OV7670_SWAP_BYTES` 颜色字节序开关 |

## 8.5 验证

```
compiling Delay.c...
compiling SCCB.c...
compiling main.c...
compiling lcd.c...
".\Objects\Project.axf" - 0 Error(s), 0 Warning(s).
Program Size: Code=6298  RO-data=14654  RW-data=36  ZI-data=78436
```

原始文件备份在 `_dsh_fix\backup2\`。

## 8.6 下一步

直接烧录 `Objects\Project.axf`（或 Keil 里 F8），把屏幕上 `PID / VER / LINE` 三个值告诉我：

- 三个值正常 → 应该能看到画面，接下来只可能剩"颜色字节序"和"裁剪"两个小问题；
- 还是 `ERROR` → 按 8.2 的表对照，能直接定位到是时序、上拉还是接线。
