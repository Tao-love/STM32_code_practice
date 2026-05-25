# motor 模块在当前项目中的调用说明

## 1. 当前模块里有哪些函数
你现在已经写好的函数分两层。

### 1.1 单轮函数
- `Motor_Init()`
- `Motor_Forward(uint8_t motor_id)`
- `Motor_Reverse(uint8_t motor_id)`
- `Motor_SlowStop(uint8_t motor_id)`
- `Motor_FastStop(uint8_t motor_id)`
- `Motor_SetSpeed(uint8_t motor_id, uint16_t speed)`

### 1.2 整车函数
- `Car_Forward(void)`
- `Car_Reverse(void)`
- `Car_SlowStop(void)`
- `Car_FastStop(void)`

---

## 2. 宏定义是什么意思
头文件里现在有这些宏：

```c
#define MOTOR1 1
#define MOTOR2 2
#define speed_L_init 50
#define speed_R_init 50
```

含义是：
- `MOTOR1`：左轮
- `MOTOR2`：右轮
- `speed_L_init`：左轮初始化默认速度
- `speed_R_init`：右轮初始化默认速度

你以后要改默认速度，就直接改这两个宏。

---

## 3. `Motor_Init()` 是什么时候调用的
这个函数建议在 `main.c` 里所有外设初始化完成后调用。

位置通常放在：
- `MX_GPIO_Init()`
- `MX_TIM2_Init()`

这些完成之后。

作用是：
- 启动左右 PWM
- 给左右轮设一个初始速度
- 默认先把车停住

示例：

```c
MX_GPIO_Init();
MX_TIM2_Init();

Motor_Init();
```

---

## 4. `Motor_SetSpeed()` 现在怎么理解
这个函数写的是：

```c
Motor_SetSpeed(MOTOR1, 50);
Motor_SetSpeed(MOTOR2, 50);
```

虽然你把参数名叫 `speed`，但它现在本质上还是 PWM 比较值。

如果当前 `TIM2` 的 `ARR` 是 `99`，那大致可以这样理解：
- `0`：约 0%
- `25`：约 25%
- `50`：约 50%
- `99`：接近 100%

也就是说，你现在调“速度”，实际上是在调占空比大小。

---

## 5. 单轮函数怎么用

### 5.1 左轮正转
```c
Motor_Forward(MOTOR1);
```

### 5.2 左轮反转
```c
Motor_Reverse(MOTOR1);
```

### 5.3 左轮缓停
```c
Motor_SlowStop(MOTOR1);
```

### 5.4 左轮急停
```c
Motor_FastStop(MOTOR1);
```

右轮同理，把 `MOTOR1` 换成 `MOTOR2`。

---

## 6. 整车函数怎么用

### 6.1 整车前进
```c
Car_Forward();
```

它内部会做：
- 左轮前进
- 右轮前进

### 6.2 整车后退
```c
Car_Reverse();
```

它内部会做：
- 左轮后退
- 右轮后退

### 6.3 整车缓停
```c
Car_SlowStop();
```

它内部会做：
- 左轮缓停
- 右轮缓停

### 6.4 整车急停
```c
Car_FastStop();
```

它内部会做：
- 左轮急停
- 右轮急停

---

## 7. 当前项目里最常见的调用顺序
如果只是做最基本的跑车测试，通常可以这样写：

```c
Motor_Init();

Motor_SetSpeed(MOTOR1, 25);
Motor_SetSpeed(MOTOR2, 25);
Car_Forward();
```

如果要停车：

```c
Car_SlowStop();
```

如果要紧急停车：

```c
Car_FastStop();
```

如果要后退：

```c
Car_Reverse();
```

---

## 8. 后面接超声波时怎么用
你前面的计划是按距离分三档。

### 8.1 距离大于等于 30cm
建议：

```c
Motor_SetSpeed(MOTOR1, 25);
Motor_SetSpeed(MOTOR2, 25);
Car_Forward();
```

### 8.2 距离在 15cm 到 30cm 之间
建议：

```c
Motor_SetSpeed(MOTOR1, 15);
Motor_SetSpeed(MOTOR2, 15);
Car_Forward();
```

### 8.3 距离小于 15cm
建议：

```c
Car_FastStop();
```

或者如果你觉得不需要那么硬的刹车，也可以：

```c
Car_SlowStop();
```

---

## 9. 当前这个模块还没做什么
目前这个模块还没有做这些内容：
- 左转
- 右转
- 原地旋转
- 自动避障转向
- 速度百分比换算接口

但这不影响你现在先完成“前进、减速、停车”主逻辑。

---

## 10. 你后面最该注意的点

### 10.1 先设速度，再给动作
建议顺序：
1. 先 `Motor_SetSpeed()`
2. 再 `Car_Forward()` 或 `Car_Reverse()`

这样逻辑最清楚。

### 10.2 如果轮子方向反了，不一定是代码错
有两种可能：
- 电机线接反了
- 你定义的“前进方向”和实际安装方向相反

这种情况你可以选择：
- 改电机线
- 或者改 `Motor_Forward()` / `Motor_Reverse()` 里的高低电平组合

### 10.3 急停不要默认到处乱用
`Car_FastStop()` 虽然停得快，但频繁急停对机械和供电冲击会更大一些。

所以通常：
- 普通停车用 `Car_SlowStop()`
- 快速避障用 `Car_FastStop()`

---

## 11. 当前阶段推荐做法
按你现在项目进度，最合理的下一步是：
1. 在 `main.c` 里包含 `motor.h`
2. 初始化后调用 `Motor_Init()`
3. 先做一次整车前进/后退/停车测试
4. 再把超声波距离判断接进来

这样推进最稳。

---

## 12. 一句话总结
这套调用方式可以简单记成：
- `Motor_Init()` 负责准备好电机控制
- `Motor_SetSpeed()` 负责设轮速
- `Car_Forward()` / `Car_Reverse()` 负责运动方向
- `Car_SlowStop()` / `Car_FastStop()` 负责停车方式
