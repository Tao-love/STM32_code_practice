# STM32L432KC OLED 初始化梳理

本文档用于梳理当前工程中 OLED 的初始化流程。工程目标芯片为 `STM32L432KC`，OLED 为 0.96 寸 IIC OLED，分辨率 `128x64`，按 SSD1306 兼容方式初始化。

## 1. 硬件连接

| OLED 引脚 | 开发板引脚 | STM32 引脚 | 外设功能 |
| --- | --- | --- | --- |
| SCL | D1 | PA9 | I2C1_SCL |
| SDA | D0 | PA10 | I2C1_SDA |
| VCC | 3.3V / 5V | - | OLED 供电，按模块标注接 |
| GND | GND | - | 共地 |

当前 `Core/Src/i2c.c` 中已经配置：

```c
PA9  ------> I2C1_SCL
PA10 ------> I2C1_SDA
```

代码中使用的 I2C 句柄为：

```c
hi2c1
```

所以 OLED 驱动底层发送函数直接调用：

```c
HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, data, len, HAL_MAX_DELAY);
```

## 2. OLED 地址

当前 `Core/Inc/oled.h` 中定义：

```c
#define OLED_I2C_ADDR 0x78
```

这里的 `0x78` 是 OLED 的 8 位写地址。

资料里常见的 OLED 地址有两种写法：

| 写法 | 数值 | 含义 |
| --- | --- | --- |
| 7 位地址 | `0x3C` | I2C 设备真实地址 |
| 8 位写地址 | `0x78` | `0x3C << 1` 后得到的写地址 |

STM32 HAL 的 `HAL_I2C_Master_Transmit()` 需要传入左移后的地址，所以本工程使用 `0x78`。

## 3. 命令和数据控制字节

OLED 通过 I2C 收数据时，需要先发一个控制字节，告诉 OLED 后面跟的是命令还是显存数据。

当前 `Core/Src/oled.c` 中定义：

```c
#define OLED_CMD_CONTROL  0x00
#define OLED_DATA_CONTROL 0x40
```

含义如下：

| 控制字节 | 含义 |
| --- | --- |
| `0x00` | 后面发送的是 OLED 命令 |
| `0x40` | 后面发送的是 OLED 显示数据 |

发送命令的封装函数：

```c
static void OLED_SendCmd(uint8_t cmd)
{
  uint8_t sendBuffer[2] = {OLED_CMD_CONTROL, cmd};
  OLED_Send(sendBuffer, 2);
}
```

也就是每发一条命令，实际 I2C 数据为：

```text
0x00 + 命令字节
```

## 4. 当前初始化调用位置

在 `Core/Src/main.c` 中，初始化顺序为：

```c
HAL_Init();
SystemClock_Config();
MX_GPIO_Init();
MX_I2C1_Init();
OLED_Init();
```

重点是：必须先执行 `MX_I2C1_Init()`，再执行 `OLED_Init()`。

原因是 `OLED_Init()` 里面会立刻使用 `hi2c1` 发送 I2C 命令。如果 I2C1 没有先初始化，OLED 通信就不能正常工作。

## 5. OLED_Init() 总体流程

当前 `OLED_Init()` 的整体逻辑可以分成 4 步：

1. 延时等待 OLED 上电稳定。
2. 关闭显示并配置 SSD1306 的显示参数。
3. 清空软件显存，并把空显存刷新到屏幕。
4. 打开 OLED 显示。

对应代码结构：

```c
void OLED_Init(void)
{
  HAL_Delay(100);    // 等待OLED上电稳定

  // 发送一系列SSD1306初始化命令
  OLED_SendCmd(...);

  OLED_NewFrame();   // 清空软件显存OLED_GRAM
  OLED_ShowFrame();  // 把空显存写入OLED，清掉随机亮点

  OLED_SendCmd(0xAF); // 开启显示
}
```

## 6. 初始化命令逐条说明

| 命令 | 参数 | 作用 |
| --- | --- | --- |
| `0xAE` | - | 关闭 OLED 显示，初始化时先不要亮屏 |
| `0x20` | `0x02` | 设置内存寻址模式为页寻址模式 |
| `0xB0` | - | 设置页地址起始为第 0 页 |
| `0xC8` | - | 设置 COM 扫描方向，让屏幕上下方向匹配模块 |
| `0x00` | - | 设置列地址低 4 位为 0 |
| `0x10` | - | 设置列地址高 4 位为 0 |
| `0x40` | - | 设置显示起始行为第 0 行 |
| `0x81` | `0xDF` | 设置对比度 |
| `0xA1` | - | 设置段重映射，让屏幕左右方向匹配模块 |
| `0xA6` | - | 正常显示，黑底白字 |
| `0xA8` | `0x3F` | 设置多路复用率为 1/64，对应 64 行 |
| `0xA4` | - | 显示内容来自显存，而不是强制全亮 |
| `0xD3` | `0x00` | 设置显示偏移为 0 |
| `0xD5` | `0xF0` | 设置显示时钟分频和振荡频率 |
| `0xD9` | `0x22` | 设置预充电周期 |
| `0xDA` | `0x12` | 设置 COM 引脚硬件配置，适配 128x64 屏 |
| `0xDB` | `0x20` | 设置 VCOMH 电平 |
| `0x8D` | `0x14` | 开启电荷泵，OLED 内部升压工作 |
| `0xAF` | - | 开启 OLED 显示 |

## 7. 为什么初始化时要先清屏

OLED 上电后，屏幕内部显存可能不是全 0，所以有时会看到随机亮点、横线或花屏。

当前代码初始化时执行：

```c
OLED_NewFrame();
OLED_ShowFrame();
```

作用是：

| 函数 | 作用 |
| --- | --- |
| `OLED_NewFrame()` | 清空 STM32 内部的软件显存 `OLED_GRAM` |
| `OLED_ShowFrame()` | 把全 0 的显存写入 OLED |

这样 OLED 初始化完成后，屏幕会保持全黑，不会出现随机亮点。

## 8. 软件显存 OLED_GRAM 的结构

当前驱动参考课程工程写法，不是每画一个点就马上发给 OLED，而是先画到 STM32 的数组里：

```c
static uint8_t OLED_GRAM[OLED_PAGE][OLED_WIDTH];
```

其中：

```c
#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGE   8
```

OLED 一共有 64 行，每 8 行组成 1 页，所以：

```text
64 行 / 8 = 8 页
```

显存结构如下：

```text
OLED_GRAM[0][0~127]  第0页，对应 y=0~7
OLED_GRAM[1][0~127]  第1页，对应 y=8~15
OLED_GRAM[2][0~127]  第2页，对应 y=16~23
OLED_GRAM[3][0~127]  第3页，对应 y=24~31
OLED_GRAM[4][0~127]  第4页，对应 y=32~39
OLED_GRAM[5][0~127]  第5页，对应 y=40~47
OLED_GRAM[6][0~127]  第6页，对应 y=48~55
OLED_GRAM[7][0~127]  第7页，对应 y=56~63
```

每 1 个字节控制竖向 8 个像素。

例如：

```c
OLED_GRAM[y / 8][x] |= 1 << (y % 8);
```

表示把坐标 `(x, y)` 对应的像素点设置为亮。

这句代码可以拆开理解：

```c
y / 8
```

用于计算当前像素点在第几页。

因为 OLED 每 8 行组成 1 页，所以：

```text
y = 0  ~ 7   -> 第0页
y = 8  ~ 15  -> 第1页
y = 16 ~ 23  -> 第2页
...
y = 56 ~ 63  -> 第7页
```

例如 `y = 18`：

```c
18 / 8 = 2
```

说明这个点在第 2 页。

```c
y % 8
```

用于计算当前像素点在这一页的第几个 bit 位。

例如 `y = 18`：

```c
18 % 8 = 2
```

说明它对应这一页字节里的第 2 位。

```c
1 << (y % 8)
```

用于生成一个只选中目标 bit 位的掩码。

例如 `y % 8 = 2`：

```c
1 << 2 = 0b00000100
```

最后：

```c
OLED_GRAM[y / 8][x] |= 1 << (y % 8);
```

含义就是：找到 `(x, y)` 这个点所在的页和列，然后把对应字节里的某一位改成 `1`，从而点亮这个像素。

例如：

```c
x = 10;
y = 18;
```

那么：

```c
OLED_GRAM[18 / 8][10] |= 1 << (18 % 8);
```

等价于：

```c
OLED_GRAM[2][10] |= 0b00000100;
```

如果原来这个字节是：

```text
00010000
```

执行后变成：

```text
00010100
```

可以看到，它只把目标像素对应的 bit 位变成 `1`，其他 bit 位保持不变。

一句话总结：

```text
y / 8 找页，x 找列，y % 8 找这一列字节里的第几个像素位，|= 把它点亮。
```

## 9. 刷屏 OLED_ShowFrame() 的流程

`OLED_ShowFrame()` 会把 8 页显存依次写进 OLED。

每一页的刷新步骤为：

1. 设置当前页地址：`0xB0 + page`
2. 设置列地址低位：`0x00`
3. 设置列地址高位：`0x10`
4. 发送 128 字节显示数据

简化理解：

```text
第0页：发128字节
第1页：发128字节
第2页：发128字节
...
第7页：发128字节
```

总共发送：

```text
8 页 x 128 字节 = 1024 字节
```

## 10. 初始化成功后的预期现象

如果硬件连接、地址和 I2C 配置都正确，执行 `OLED_Init()` 后：

1. OLED 被正确初始化。
2. 屏幕不会乱闪。
3. 屏幕保持全黑。
4. 后续调用画点、画线、显示字符等函数，再执行 `OLED_ShowFrame()` 才会出现内容。

例如后续可以测试：

```c
OLED_NewFrame();
OLED_DrawRectangle(0, 0, 127, 63, OLED_COLOR_NORMAL);
OLED_DrawLine(0, 0, 127, 63, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

## 11. 常见问题排查

### 11.1 屏幕完全不亮

优先检查：

1. OLED 的 VCC 和 GND 是否接对。
2. SCL 是否接 D1/PA9。
3. SDA 是否接 D0/PA10。
4. `MX_I2C1_Init()` 是否在 `OLED_Init()` 前调用。
5. OLED 地址是否为 `0x78`。如果模块地址不同，可能需要改为 `0x7A`。

### 11.2 屏幕有随机亮点

检查初始化中是否执行了：

```c
OLED_NewFrame();
OLED_ShowFrame();
```

这两句用于初始化清屏。

### 11.3 内容方向反了

主要看这两条命令：

```c
OLED_SendCmd(0xC8);
OLED_SendCmd(0xA1);
```

如果屏幕上下或左右反了，通常需要调整为：

```c
OLED_SendCmd(0xC0);
OLED_SendCmd(0xA0);
```

具体是否需要调整，要看 OLED 模块的屏幕安装方向。

### 11.4 初始化正常但画图不显示

课程风格的驱动是先写显存，最后统一刷新，所以画完之后必须调用：

```c
OLED_ShowFrame();
```

只调用 `OLED_DrawLine()`、`OLED_SetPixel()`、`OLED_PrintString()` 不会立刻显示到屏幕上。

## 12. 当前工程涉及的文件

| 文件 | 作用 |
| --- | --- |
| `Core/Inc/oled.h` | OLED 参数、结构体、函数声明 |
| `Core/Src/oled.c` | OLED 初始化、I2C 发送、显存、绘图函数 |
| `Core/Src/i2c.c` | I2C1 初始化，PA9/PA10 配置 |
| `Core/Src/main.c` | 主函数中调用 `MX_I2C1_Init()` 和 `OLED_Init()` |

## 13. 最小初始化链路总结

最核心的调用关系如下：

```text
main()
  -> HAL_Init()
  -> SystemClock_Config()
  -> MX_GPIO_Init()
  -> MX_I2C1_Init()
  -> OLED_Init()
       -> HAL_Delay(100)
       -> OLED_SendCmd(...)
            -> OLED_Send(...)
                 -> HAL_I2C_Master_Transmit(&hi2c1, 0x78, ...)
       -> OLED_NewFrame()
       -> OLED_ShowFrame()
       -> OLED_SendCmd(0xAF)
```

一句话总结：

> 先初始化 STM32 的 I2C1，再通过 `hi2c1` 向地址 `0x78` 的 OLED 发送 SSD1306 初始化命令，随后清空并刷新 1024 字节显存，最后打开显示。
