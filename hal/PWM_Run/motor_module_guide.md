# motor 模块代码与思路说明

## 1. 文件说明
这份文档记录当前小车电机控制模块 `motor` 的代码、设计思路和控制原理。

当前源码位置：
- `Core/Inc/motor.h`
- `Core/Src/motor.c`

当前模块的目标很明确：
- 用 `MOTOR1` 和 `MOTOR2` 区分左右两个电机
- 用 `IN1~IN4` 控制方向与刹车
- 用 `TIM2_CH1` 和 `TIM2_CH2` 的 PWM 控制左右轮速度
- 在此基础上再封装整车前进、后退、停车动作

---

## 2. 当前 motor.h

```c
/*
 * motor.h
 *
 *  Created on: May 25, 2026
 *      Author: ZhuanZ（无密码）
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"
#include "tim.h"

#define MOTOR1 1
#define MOTOR2 2
#define speed_L_init 50
#define speed_R_init 50

void Motor_Init(void);
void Motor_Forward(uint8_t motor_id);
void Motor_Reverse(uint8_t motor_id);
void Motor_SlowStop(uint8_t motor_id);
void Motor_FastStop(uint8_t motor_id);
void Motor_SetSpeed(uint8_t motor_id, uint16_t speed);
void Car_Forward(void);
void Car_Reverse(void);
void Car_SlowStop(void);
void Car_FastStop(void);

#endif /* INC_MOTOR_H_ */
```

---

## 3. 当前 motor.c

```c
/*
 * motor.c
 *
 *  Created on: May 25, 2026
 *      Author: ZhuanZ（无密码）
 */

#include "motor.h"

void Motor_Init(void)
{
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  Motor_SetSpeed(MOTOR1, speed_L_init);
  Motor_SetSpeed(MOTOR2, speed_R_init);

  Motor_SlowStop(MOTOR1);
  Motor_SlowStop(MOTOR2);
}

void Motor_SetSpeed(uint8_t motor_id, uint16_t speed)
{
  uint32_t max_pwm = __HAL_TIM_GET_AUTORELOAD(&htim2);

  if (motor_id == MOTOR1)
  {
    if (speed > max_pwm)
    {
      speed = max_pwm;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, speed);
    return;
  }

  if (motor_id == MOTOR2)
  {
    if (speed > max_pwm)
    {
      speed = max_pwm;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, speed);
  }
}

void Motor_Forward(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
  }
}

void Motor_Reverse(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
  }
}

void Motor_SlowStop(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
  }
}

void Motor_FastStop(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
  }
}

void Car_Forward(void)
{
  Motor_Forward(MOTOR1);
  Motor_Forward(MOTOR2);
}

void Car_Reverse(void)
{
  Motor_Reverse(MOTOR1);
  Motor_Reverse(MOTOR2);
}

void Car_SlowStop(void)
{
  Motor_SlowStop(MOTOR1);
  Motor_SlowStop(MOTOR2);
}

void Car_FastStop(void)
{
  Motor_FastStop(MOTOR1);
  Motor_FastStop(MOTOR2);
}
```

---

## 4. 这个模块是怎么想出来的

这个模块不是按“抽象最漂亮”的方向写的，而是按“你自己后面最好读、最好改”的方向写的。

核心思路有 4 条：

### 4.1 先分单轮，再分整车
先把单个电机控制清楚：
- 正转
- 反转
- 缓停
- 急停
- 调速

然后再组合成整车动作：
- 前进
- 后退
- 缓停
- 急停

这样后面如果你要加：
- 左转
- 右转
- 原地旋转
- 左右轮不同速

都会很方便，因为底层单轮函数已经准备好了。

### 4.2 不做你不喜欢的“引脚抽象层”
之前没有用什么 `in_a_pin`、`in_b_pin` 这种写法，原因很简单：
- 看代码时要跳来跳去
- 不如直接看到 `IN1`、`IN2`、`IN3`、`IN4` 明白
- 你自己维护时更直接

所以现在的写法是：
- 如果传入 `MOTOR1`，就直接操作 `IN1/IN2`
- 如果传入 `MOTOR2`，就直接操作 `IN3/IN4`

这不是最“高级”的写法，但对当前项目是最实用的写法。

### 4.3 速度和方向分开控制
这个模块里把“速度”和“方向”拆开了：
- `Motor_SetSpeed()` 只管 PWM 比较值
- `Motor_Forward()` / `Motor_Reverse()` / `Motor_SlowStop()` / `Motor_FastStop()` 只管方向引脚

这样做的好处是：
- 不会在一个函数里同时混进太多动作
- 你以后可以先设速度，再决定前进还是后退
- 逻辑更稳定，不容易改乱

### 4.4 PWM 启动只放在 Init 里
你前面已经明确过：
- 不要在 `Motor_SetSpeed()` 里频繁启动和关闭 PWM
- PWM 启动统一放在初始化函数里

所以现在 `Motor_Init()` 只做三件事：
1. 启动左右两个 PWM 通道
2. 设置左右初始速度
3. 把左右电机先置为缓停状态

这个顺序是合理的，因为上电后：
- PWM 已经准备好
- 比较值已经有默认值
- 但方向脚是停的，不会一上电就乱跑

---

## 5. 这个模块的控制原理

## 5.1 L293N 的基本控制逻辑
一个电机通常需要 3 类控制信号：
- 1 个使能端：`ENA` 或 `ENB`
- 2 个方向端：比如 `IN1`、`IN2`

在当前项目里：
- `ENA` 对应左轮 PWM
- `ENB` 对应右轮 PWM
- `IN1/IN2` 控制左轮方向
- `IN3/IN4` 控制右轮方向

本质上是：
- `ENA/ENB` 决定电机“给多少力”
- `IN1~IN4` 决定电机“朝哪边转”或者“怎么停”

---

## 5.2 为什么 PWM 能控速度
PWM 不是直接输出一个连续变化的模拟电压，而是输出一个“高低电平快速切换”的数字波形。

例如：
- 频率固定为 `1kHz`
- 如果占空比是 `25%`
- 那么每个周期里有 `25%` 时间是高电平，`75%` 时间是低电平

电机看到的是一个平均效果：
- 高电平越多，平均电压越高，速度通常越快
- 高电平越少，平均电压越低，速度通常越慢

所以：
- 频率一般先固定
- 通过改占空比去改速度

在这个项目里，PWM 是由 `TIM2` 输出到：
- `TIM2_CH1 -> PA0 -> ENA`
- `TIM2_CH2 -> PA1 -> ENB`

---

## 5.3 `Motor_SetSpeed()` 本质在干什么
`Motor_SetSpeed()` 并不是直接写“速度百分比”，它现在写进去的是定时器比较寄存器的值。

函数里的关键逻辑：

```c
uint32_t max_pwm = __HAL_TIM_GET_AUTORELOAD(&htim2);
```

这句拿到的是自动重装值，也就是当前 PWM 计数上限。

后面：

```c
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, speed);
```

或者：

```c
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, speed);
```

就是把比较值写进去。

如果当前 `ARR = 99`，那大致关系就是：
- `speed = 0`，占空比约 `0%`
- `speed = 25`，占空比约 `25%`
- `speed = 50`，占空比约 `50%`
- `speed = 99`，占空比接近 `100%`

所以你现在传进去的 `speed`，更准确地说是“PWM 比较值”，只是你为了后续自己读着顺手，把它叫做 `speed`。

这在当前项目里是可以接受的。

---

## 5.4 正转、反转、缓停、急停的原理
以一路电机为例，方向控制可以理解成下面这样：

| 状态 | INa | INb | 效果 |
| --- | --- | --- | --- |
| 正转 | 1 | 0 | 电机正向转 |
| 反转 | 0 | 1 | 电机反向转 |
| 缓停 | 0 | 0 | 电机断开驱动，靠惯性慢慢停 |
| 急停 | 1 | 1 | 两端同电平，形成电子刹车，停得更快 |

对应到当前代码：

左轮：
- 正转：`IN1=1, IN2=0`
- 反转：`IN1=0, IN2=1`
- 缓停：`IN1=0, IN2=0`
- 急停：`IN1=1, IN2=1`

右轮：
- 正转：`IN3=1, IN4=0`
- 反转：`IN3=0, IN4=1`
- 缓停：`IN3=0, IN4=0`
- 急停：`IN3=1, IN4=1`

这就是为什么代码里要分 `Motor_SlowStop()` 和 `Motor_FastStop()`。

---

## 5.5 为什么初始化后默认先停
`Motor_Init()` 中最后调用了：

```c
Motor_SlowStop(MOTOR1);
Motor_SlowStop(MOTOR2);
```

这样做的原因是：
- 定时器 PWM 虽然已经启动了
- 速度默认值也已经写进去了
- 但方向脚先不让电机真正输出转动方向

这样上电时更安全，不容易出现：
- 一下载程序就跑车
- 上电瞬间方向状态不确定
- 引脚默认态导致电机误动作

---

## 6. 当前模块已经能做什么

现在这个 `motor` 模块已经能完成：
- 左轮单独正转、反转、缓停、急停、调速
- 右轮单独正转、反转、缓停、急停、调速
- 整车前进
- 整车后退
- 整车缓停
- 整车急停

换句话说，当前小车运动控制的基础层已经够用了。

---

## 7. 这个模块后面怎么继续扩展

如果后面继续往下做，最自然的扩展方向是：

### 7.1 增加整车转向函数
比如：
- `Car_TurnLeft()`
- `Car_TurnRight()`
- `Car_SpinLeft()`
- `Car_SpinRight()`

这些都可以直接在当前单轮函数基础上组合出来。

### 7.2 把速度做成更好理解的百分比接口
现在 `Motor_SetSpeed()` 传的是比较值。
以后如果你愿意，可以再封装一层：
- 输入 `0~100`
- 内部换算成 `CCR` 值

这样主逻辑会更像“设置百分比速度”，更容易读。

### 7.3 接入超声波避障主逻辑
后面主循环可以按距离做判断：
- 远：前进
- 中：减速前进
- 近：停车

这一步不需要再改 `motor` 底层太多，只需要调用已经写好的函数。

---

## 8. 一句话总结
这个 `motor` 模块目前的特点是：
- 结构简单
- 引脚对应直接
- 方便你自己维护
- 已经具备小车运动控制的基础能力

它不是为了做成“通用电机框架”，而是为了让这个 STM32 小车项目先稳定跑起来。
