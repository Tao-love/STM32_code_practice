# OLED 可调用操作函数梳理

本文档对应当前工程中的 OLED 驱动文件：

- 头文件：`Core/Inc/oled.h`
- 源文件：`Core/Src/oled.c`
- 芯片：`STM32L432KC`
- OLED：0.96 寸 IIC OLED，128x64，SSD1306 兼容
- 接线：`SCL = D1 = PA9 = I2C1_SCL`，`SDA = D0 = PA10 = I2C1_SDA`

当前 OLED 驱动的核心思想是：

1. 先把点、线、矩形、文字、图片画到单片机 RAM 里的 `OLED_GRAM` 显存。
2. 再调用 `OLED_ShowFrame()`，把整帧显存一次性刷新到 OLED 屏幕。

也就是说，大部分绘图函数本身不会立刻让屏幕变化，只有刷新后才会真正显示出来。

## 1. 基本参数

```c
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGE 8
#define OLED_I2C_ADDR 0x78
```

含义：

- `OLED_WIDTH`：屏幕宽度，128 列。
- `OLED_HEIGHT`：屏幕高度，64 行。
- `OLED_PAGE`：页数，64 行被分成 8 页，每页 8 行。
- `OLED_I2C_ADDR`：OLED 的 I2C 写地址，`0x78`。它对应 7 位地址 `0x3C` 左移 1 位。

## 2. 颜色模式

```c
typedef enum {
  OLED_COLOR_NORMAL = 0,
  OLED_COLOR_REVERSED
} OLED_ColorMode;
```

可选值：

- `OLED_COLOR_NORMAL`：正常模式。画点时点亮像素，常理解为黑底白字。
- `OLED_COLOR_REVERSED`：反色模式。画点时清除像素，常理解为白底黑字或擦除效果。

在绘图函数里，最后一个参数通常都是 `OLED_ColorMode color`。

## 3. 初始化和显示控制函数

### OLED_Init

```c
void OLED_Init(void);
```

作用：

- 等待 OLED 上电稳定。
- 向 OLED 发送初始化命令。
- 清空显存。
- 把空显存刷新到屏幕，避免刚上电时出现随机亮点。
- 打开 OLED 显示。

典型调用位置：

```c
MX_GPIO_Init();
MX_I2C1_Init();
OLED_Init();
```

注意：

- 必须在 `MX_I2C1_Init()` 之后调用。
- 因为 OLED 通过 `hi2c1` 通信，如果 I2C1 没有初始化，OLED 无法收到命令。

### OLED_DisPlay_On

```c
void OLED_DisPlay_On(void);
```

作用：

- 打开 OLED 显示。
- 同时开启电荷泵。

适用场景：

- 屏幕被关闭后重新点亮。
- 低功耗休眠后恢复显示。

### OLED_DisPlay_Off

```c
void OLED_DisPlay_Off(void);
```

作用：

- 关闭 OLED 显示。
- 同时关闭电荷泵。

注意：

- 关闭显示不等于清空显存。
- 重新调用 `OLED_DisPlay_On()` 后，原来显存里的内容仍然可能显示出来。

### OLED_SetColorMode

```c
void OLED_SetColorMode(OLED_ColorMode mode);
```

作用：

- 设置整个屏幕的显示模式。

示例：

```c
OLED_SetColorMode(OLED_COLOR_NORMAL);   // 正常显示
OLED_SetColorMode(OLED_COLOR_REVERSED); // 整屏反色显示
```

注意：

- 这个函数是发送 OLED 控制命令，影响的是屏幕整体显示效果。
- 它和绘图函数里的 `color` 参数不是同一层含义。

## 4. 显存操作函数

### OLED_NewFrame

```c
void OLED_NewFrame(void);
```

作用：

- 清空单片机 RAM 里的 OLED 显存 `OLED_GRAM`。
- 通常在开始绘制新画面前调用。

示例：

```c
OLED_NewFrame();
OLED_DrawLine(0, 0, 127, 63, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

注意：

- `OLED_NewFrame()` 只清空内存中的显存，不会马上清空屏幕。
- 如果想让屏幕也变黑，需要再调用 `OLED_ShowFrame()`。

### OLED_ShowFrame

```c
void OLED_ShowFrame(void);
```

作用：

- 把 `OLED_GRAM` 中的 8 页 x 128 列数据发送到 OLED。
- 这是让绘制内容真正显示出来的关键函数。

典型流程：

```c
OLED_NewFrame();                                      // 清空显存
OLED_DrawRectangle(0, 0, 127, 63, OLED_COLOR_NORMAL); // 画边框
OLED_ShowFrame();                                     // 刷新到屏幕
```

注意：

- 点、线、矩形、文字、图片等函数一般只是修改显存。
- 修改显存后必须调用 `OLED_ShowFrame()`，屏幕才会变化。

### OLED_SetPixel

```c
void OLED_SetPixel(uint8_t x, uint8_t y, OLED_ColorMode color);
```

作用：

- 设置一个像素点。

参数：

- `x`：横坐标，范围 `0~127`。
- `y`：纵坐标，范围 `0~63`。
- `color`：颜色模式。

示例：

```c
OLED_NewFrame();
OLED_SetPixel(10, 20, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

内部核心语句：

```c
OLED_GRAM[y / 8][x] |= (uint8_t)(1 << (y % 8));
```

解释：

- `y / 8`：判断这个点属于第几页。
- `x`：判断这个点属于第几列。
- `y % 8`：判断这个点在当前页的第几位。
- `1 << (y % 8)`：生成一个只点亮对应位的二进制掩码。
- `|=`：在不影响同一字节里其他像素的情况下，把目标像素置 1。

## 5. 图形绘制函数

### OLED_DrawLine

```c
void OLED_DrawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, OLED_ColorMode color);
```

作用：

- 绘制一条线段。

参数：

- `(x1, y1)`：线段起点。
- `(x2, y2)`：线段终点。
- `color`：颜色模式。

示例：

```c
OLED_DrawLine(0, 0, 127, 63, OLED_COLOR_NORMAL);
```

### OLED_DrawRectangle

```c
void OLED_DrawRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
```

作用：

- 绘制空心矩形。

参数：

- `(x, y)`：矩形左上角坐标。
- `w`：矩形宽度参数。
- `h`：矩形高度参数。
- `color`：颜色模式。

示例：

```c
OLED_DrawRectangle(0, 0, 127, 63, OLED_COLOR_NORMAL);
```

注意：

- 当前代码内部画的是从 `x` 到 `x + w`、从 `y` 到 `y + h` 的边线。
- 如果想画满 128x64 的外框，可以使用 `w = 127`、`h = 63`。

### OLED_DrawFilledRectangle

```c
void OLED_DrawFilledRectangle(uint8_t x, uint8_t y, uint8_t w, uint8_t h, OLED_ColorMode color);
```

作用：

- 绘制实心矩形。

示例：

```c
OLED_DrawFilledRectangle(10, 10, 30, 20, OLED_COLOR_NORMAL);
```

### OLED_DrawTriangle

```c
void OLED_DrawTriangle(uint8_t x1, uint8_t y1,
                       uint8_t x2, uint8_t y2,
                       uint8_t x3, uint8_t y3,
                       OLED_ColorMode color);
```

作用：

- 绘制空心三角形。

示例：

```c
OLED_DrawTriangle(64, 5, 10, 55, 118, 55, OLED_COLOR_NORMAL);
```

### OLED_DrawFilledTriangle

```c
void OLED_DrawFilledTriangle(uint8_t x1, uint8_t y1,
                             uint8_t x2, uint8_t y2,
                             uint8_t x3, uint8_t y3,
                             OLED_ColorMode color);
```

作用：

- 绘制实心三角形。

示例：

```c
OLED_DrawFilledTriangle(64, 5, 10, 55, 118, 55, OLED_COLOR_NORMAL);
```

### OLED_DrawCircle

```c
void OLED_DrawCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
```

作用：

- 绘制空心圆。

参数：

- `(x, y)`：圆心坐标。
- `r`：半径。
- `color`：颜色模式。

示例：

```c
OLED_DrawCircle(64, 32, 20, OLED_COLOR_NORMAL);
```

### OLED_DrawFilledCircle

```c
void OLED_DrawFilledCircle(uint8_t x, uint8_t y, uint8_t r, OLED_ColorMode color);
```

作用：

- 绘制实心圆。

示例：

```c
OLED_DrawFilledCircle(64, 32, 10, OLED_COLOR_NORMAL);
```

### OLED_DrawEllipse

```c
void OLED_DrawEllipse(uint8_t x, uint8_t y, uint8_t a, uint8_t b, OLED_ColorMode color);
```

作用：

- 绘制空心椭圆。

参数：

- `(x, y)`：椭圆中心。
- `a`：横向半轴长度。
- `b`：纵向半轴长度。
- `color`：颜色模式。

示例：

```c
OLED_DrawEllipse(64, 32, 40, 20, OLED_COLOR_NORMAL);
```

## 6. 图片显示函数

### Image 结构体

```c
typedef struct {
  uint8_t w;
  uint8_t h;
  const uint8_t *data;
} Image;
```

含义：

- `w`：图片宽度。
- `h`：图片高度。
- `data`：图片取模数据。

### OLED_DrawImage

```c
void OLED_DrawImage(uint8_t x, uint8_t y, const Image *img, OLED_ColorMode color);
```

作用：

- 在指定位置绘制图片。

参数：

- `(x, y)`：图片左上角坐标。
- `img`：图片结构体指针。
- `color`：颜色模式。

示例：

```c
OLED_DrawImage(0, 0, &MyImage, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

### OLED_DrawImage 显示图片的原理

OLED 本身不会认识“图片”这个概念，它只认识一堆像素点。所谓显示图片，本质上就是把图片取模后得到的字节数组，按照 OLED 的显存格式写入 `OLED_GRAM`，最后再刷新到屏幕。

当前工程里的图片显示链路是：

```c
OLED_DrawImage()
    -> OLED_SetBlock()
        -> OLED_SetByte()
            -> OLED_GRAM[page][column]
    -> OLED_ShowFrame()
        -> 通过 I2C 发送到 OLED
```

也就是说：

- `OLED_DrawImage()` 负责接收图片对象。
- `OLED_SetBlock()` 负责把图片数组搬到显存对应位置。
- `OLED_ShowFrame()` 负责把显存真正发送到 OLED 屏幕。

只调用 `OLED_DrawImage()` 时，屏幕不会立刻变化；必须再调用 `OLED_ShowFrame()`。

### Image 结构体怎样描述一张图片

例如当前工程里的图片写法类似：

```c
const uint8_t GenshinData[] = {
  /* 图片取模数据 */
};

const Image GenshinImg = {72, 64, GenshinData};
```

这句话的意思是：

- `72`：图片宽度是 72 个像素。
- `64`：图片高度是 64 个像素。
- `GenshinData`：真正保存图片像素的数组。

所以调用时要传图片结构体：

```c
OLED_DrawImage(43, 0, &GenshinImg, OLED_COLOR_NORMAL);
```

不要传 `&GenshinData`，因为 `GenshinData` 只是原始字节数组，不包含宽度和高度信息。`OLED_DrawImage()` 需要知道图片有多宽、多高，才能正确搬运数据。

### 图片数组长度怎么算

OLED 是分页存储的，64 行会被分成 8 页：

```text
第0页：y = 0  ~ 7
第1页：y = 8  ~ 15
第2页：y = 16 ~ 23
...
第7页：y = 56 ~ 63
```

每一页里，一个字节控制竖直方向 8 个像素。

因此一张图片需要的字节数是：

```c
图片宽度 * 图片高度 / 8
```

例如 `GenshinImg` 是 `72 x 64`：

```c
72 * 64 / 8 = 576
```

所以 `GenshinData[]` 里应该有 576 个字节。

### OLED_SetBlock 怎样搬运图片数据

当前代码里 `OLED_DrawImage()` 的核心只有一句：

```c
OLED_SetBlock(x, y, img->data, img->w, img->h, color);
```

它把图片的起始坐标、图片数据、宽度、高度、颜色模式传给 `OLED_SetBlock()`。

`OLED_SetBlock()` 里面的核心逻辑是：

```c
uint8_t pages = (h + 7) / 8;

for (uint8_t page = 0; page < pages; page++)
{
  for (uint8_t column = 0; column < w; column++)
  {
    uint8_t targetPage = (y / 8) + page;
    uint8_t targetColumn = x + column;
    OLED_SetByte(targetPage, targetColumn, data[page * w + column], color);
  }
}
```

这段代码可以拆开理解：

- `pages = (h + 7) / 8`：算出图片高度占多少页。
- 外层 `page`：一页一页处理图片。
- 内层 `column`：在当前页里一列一列处理图片。
- `targetPage = (y / 8) + page`：算出图片数据要写到 OLED 显存的第几页。
- `targetColumn = x + column`：算出图片数据要写到 OLED 显存的第几列。
- `data[page * w + column]`：从图片数组里取出当前页、当前列的 1 个字节。

如果图片宽度是 `72`，那么：

```c
data[0 * 72 + 0]  // 第0页第0列
data[0 * 72 + 1]  // 第0页第1列
data[0 * 72 + 71] // 第0页第71列

data[1 * 72 + 0]  // 第1页第0列
data[1 * 72 + 1]  // 第1页第1列
```

所以图片数据的排列方式是：

```text
第0页的所有列 -> 第1页的所有列 -> 第2页的所有列 -> ...
```

这就是常说的“按页排列”或“列行式取模”。

### 一个字节如何表示 8 个像素

假设图片数组里的某个字节是：

```c
0x05
```

它的二进制是：

```text
0000 0101
```

如果按 OLED 当前显存习惯理解：

```text
bit0 = 当前页第0行
bit1 = 当前页第1行
bit2 = 当前页第2行
...
bit7 = 当前页第7行
```

那么 `0x05` 的 `bit0` 和 `bit2` 是 1，表示这一列的第 0 行和第 2 行像素点亮。

这也是为什么图片取模软件里的“高位在前 / 低位在前”很重要：

- 如果软件生成的是 `bit0` 对应上面的像素，和当前代码匹配。
- 如果软件生成的是 `bit7` 对应上面的像素，那么每 8 行内部会上下颠倒。
- 斜线、斜笔画最容易看出这个问题，因为跨页时会出现断开或错位。

### 为什么 y 最好是 8 的倍数

当前 `OLED_SetBlock()` 是按整页搬运图片字节的：

```c
uint8_t targetPage = (y / 8) + page;
```

这意味着图片的 `y` 坐标最好是：

```c
0, 8, 16, 24, 32, 40, 48, 56
```

如果 `y = 0`，图片正好从第 0 页开始。

如果 `y = 8`，图片正好从第 1 页开始。

如果 `y = 5`，图片理论上应该从一页中间开始，这就需要把图片字节拆开，再跨两页拼接。当前这个简化版 `OLED_SetBlock()` 没有做这种跨页位移处理，所以图片可能会显示错位。

因此显示图片时推荐：

```c
OLED_DrawImage(43, 0, &GenshinImg, OLED_COLOR_NORMAL);
```

其中 `y = 0` 是安全的。

注意：

- 当前图片数据按“列行式”排列，和课程代码、常见 OLED 取模方式一致。
- 当前底层 `OLED_SetBlock()` 更适合 `y` 是 8 的倍数的图片显示，例如 `y = 0`、`8`、`16`。

## 7. ASCII 字符显示函数

### ASCIIFont 结构体

```c
typedef struct {
  uint8_t h;
  uint8_t w;
  uint8_t *chars;
} ASCIIFont;
```

含义：

- `h`：字符高度。
- `w`：字符宽度。
- `chars`：ASCII 字模数据。

### OLED_PrintASCIIChar

```c
void OLED_PrintASCIIChar(uint8_t x, uint8_t y, char ch, const ASCIIFont *font, OLED_ColorMode color);
```

作用：

- 显示一个 ASCII 字符。

参数：

- `(x, y)`：字符左上角坐标。
- `ch`：要显示的字符。
- `font`：ASCII 字库。
- `color`：颜色模式。

示例：

```c
OLED_PrintASCIIChar(0, 0, 'A', &Font_ASCII_8x16, OLED_COLOR_NORMAL);
```

注意：

- 只支持 `' '` 到 `'~'` 范围内的可显示 ASCII 字符。
- 使用前需要工程里已经有对应的 ASCII 字库变量。

### OLED_PrintASCIIString

```c
void OLED_PrintASCIIString(uint8_t x, uint8_t y, char *str, const ASCIIFont *font, OLED_ColorMode color);
```

作用：

- 显示一串 ASCII 字符串。

示例：

```c
OLED_PrintASCIIString(0, 0, "HELLO", &Font_ASCII_8x16, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

注意：

- 每显示一个字符，`x` 会自动增加一个字符宽度。
- 当前函数不会自动换行，超出屏幕右边后就显示不到了。

## 8. 中文和混合字符串显示函数

### Font 结构体

```c
typedef struct {
  uint8_t h;
  uint8_t w;
  const uint8_t *chars;
  uint8_t len;
  const ASCIIFont *ascii;
} Font;
```

含义：

- `h`：字高。
- `w`：字宽。
- `chars`：中文字模数据。
- `len`：中文字模数量。
- `ascii`：配套 ASCII 字库。

### OLED_PrintString

```c
void OLED_PrintString(uint8_t x, uint8_t y, char *str, const Font *font, OLED_ColorMode color);
```

作用：

- 显示字符串。
- 支持课程字库格式的中文和英文混合显示。

示例：

```c
OLED_PrintString(0, 0, "你好OLED", &Font_Chinese_16x16, OLED_COLOR_NORMAL);
OLED_ShowFrame();
```

注意：

- 中文字符需要在字库里有对应字模。
- 如果中文没有对应字模，当前代码会用空格代替。
- ASCII 字符会使用 `font->ascii` 指向的 ASCII 字库显示。
- 当前函数不会自动换行。

## 9. 推荐调用流程

### 显示一帧完整画面

```c
OLED_NewFrame();

OLED_DrawRectangle(0, 0, 127, 63, OLED_COLOR_NORMAL);
OLED_DrawLine(0, 0, 127, 63, OLED_COLOR_NORMAL);
OLED_PrintASCIIString(8, 8, "OLED OK", &Font_ASCII_8x16, OLED_COLOR_NORMAL);

OLED_ShowFrame();
```

流程含义：

1. `OLED_NewFrame()`：清空旧画面。
2. 各种绘图函数：往显存里画内容。
3. `OLED_ShowFrame()`：把显存刷新到屏幕。

### 只清屏

```c
OLED_NewFrame();
OLED_ShowFrame();
```

### 关闭和重新打开屏幕

```c
OLED_DisPlay_Off();
HAL_Delay(1000);
OLED_DisPlay_On();
```

## 10. 当前不能直接调用的内部函数

下面这些函数在 `oled.c` 里是 `static`，只给驱动内部使用，不能在 `main.c` 里直接调用：

- `OLED_Send()`
- `OLED_SendCmd()`
- `OLED_SetByte()`
- `OLED_SetBlock()`
- `OLED_GetUTF8Len()`

原因：

- 它们属于底层辅助函数。
- 用户代码一般只需要调用 `oled.h` 里声明的函数。

判断一个函数能不能在 `main.c` 里调用，最简单的方法是看它有没有出现在 `Core/Inc/oled.h` 里。
