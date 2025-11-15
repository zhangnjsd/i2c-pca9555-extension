# i2c-pca9555-extension

[English version](README_EN.md)

基于 ESP-IDF 的 PCA9555/TCA9555 I2C I/O 扩展驱动（支持多设备 & 中断回调）

该项目实现了一个简单的 16 路数字 I/O 扩展驱动（PCA9555/TCA9555 系列），并包含了示例代码：

- 支持多个设备（最多 8 个）
- 支持中断（INT）引脚与回调处理
- 支持读写单个引脚和整字读取/写入
- 仅支持普通数字 I/O（不支持 PWM/ADC/DAC 等高级功能）

---

## 特性概述

- 通过 `i2c9555_add_device()` 添加设备到 I2C 总线
- 通过 `i2c9555_ioconfig()` 配置 16 路 IO 的方向（1 = 输入，0 = 输出）
- 使用 `i2c9555pin_read()` / `i2c9555pin_write()` 读写单个 IO
- 如果配置 INT 引脚，可在输入变化时由驱动通过回调通知

---

## 硬件要求

- ESP32（使用 ESP-IDF 构建）
- PCA9555 或兼容的 TCA9555 I2C I/O 扩展芯片
- I2C 总线：SDA/SCL
- 可选：INT 引脚连接到 ESP32 的 GPIO（建议外部接上拉阻或启用内部上拉）

注意：部分芯片的 I2C 地址与 `TCA`/`PCA` 不同，下面有地址表

---

## 接线示例

- VCC -> 3.3V
- GND -> GND
- SDA -> ESP32 的 SDA（例：GPIO21）
- SCL -> ESP32 的 SCL（例：GPIO22）
- INT -> ESP32 的中断 GPIO（可选，驱动默认使用下降沿触发）

确保地址引脚（A0/A1/A2 或 A2/A1/A0）按硬件配置设定。

---

## 地址表（节选）

TCA9555 地址（7-bit 写地址常见表示方式变体）

| A2 | A1 | A0 | 地址 (写) |
|----|----|----|----------|
| GND| GND| GND| 0x40     |
| GND| GND| VCC| 0x42     |
| GND| VCC| GND| 0x44     |
| ...|    |    | ...      |

PCA9555 地址（7-bit）

| A2 | A1 | A0 | 地址 |
|----|----|----|------|
| L  | L  | L  | 0x20 |
| L  | L  | H  | 0x21 |
| ...|    |    | ...  |

---

## 快速上手

1. 添加设备

```c
int device = i2c9555_add_device(GPIO_NUM_21, GPIO_NUM_22, 0x20, GPIO_NUM_10, i2c_io_callback);
```

- 参数说明：

- SDA, SCL: ESP32 的 I2C 引脚
- addr: 设备地址（见上表）
- int_pin: INT 引脚（如果不使用中断，可传 `GPIO_NUM_NC`）
- callback: 输入变化时回调函数（如果提供）

函数返回设备 id（0~7），失败返回 -1。

1. 配置 IO 方向

```c
// 16 位，上位代表端口1（GPIO8~15），下位代表端口0（GPIO0~7）
// 1 = 输入，0 = 输出
i2c9555_ioconfig(device, 0x0000); // 全输出
i2c9555_ioconfig(device, 0xFFFF); // 全输入
```

1. 读写单个引脚

```c
// 写
i2c9555pin_write(device, EXT_IO0, HIGH);
// 读
int state = i2c9555pin_read(device, EXT_IO5);
```

1. IRQ 回调

驱动会在检测到输入状态变化时，调用用户提供的回调函数：

```c
void i2c_io_callback(uint16_t pin, int level) {
    // pin 是一个位掩码（例如 EXT_IO0, EXT_IO1），level 0/1
}
```

回调中可根据 `pin` 判断是哪个引脚触发。

---

## API 概览

- int i2c9555_add_device(gpio_num_t sda, gpio_num_t scl, uint16_t addr, gpio_num_t int_pin, i2c9555_input_cb_t f);
- esp_err_t i2c9555_ioconfig(uint8_t device_id, uint16_t config);
- int i2c9555pin_read(uint8_t device_id, uint16_t pin);
- esp_err_t i2c9555pin_write(uint8_t device_id, uint16_t pin, int level);
- esp_err_t i2c9555_write_word(uint8_t device_id, uint8_t reg, uint16_t data);
- esp_err_t i2c9555_read_word(uint8_t device_id, uint8_t reg, uint16_t *data);

常用宏定义：

- EXT_IO0 ... EXT_IO17（代表 16 个 IO, 部分宏在头文件中注释为 NULL）
- HIGH/LOW

---

## 示例 (摘自 `main.c`)

```c
device = i2c9555_add_device(GPIO_NUM_9, GPIO_NUM_8, 0x20, GPIO_NUM_10, i2c_io_callback);
vTaskDelay(pdMS_TO_TICKS(10));
i2c9555_ioconfig(device, 0b0011111111111111); // 配置高 13 位为输入

// blink test
xTaskCreatePinnedToCore(blink_test, "blink_test", 4096, NULL, 2, NULL, 0);
```

---

## 编译与刷写（ESP-IDF）

在 Windows PowerShell 环境下，先初始化 ESP-IDF 环境（例如使用 `export.ps1`）后，运行：

```pwsh
idf.py build
idf.py -p COM3 flash monitor
```

根据你的设备端口修改 `COM3`。也可以使用 `idf.py -p <PORT> erase-flash` 清除 flash。

---

## 限制与注意

- 驱动仅用于普通数字 IO，未实现 PWM、ADC、DAC 等高级功能。
- INT 引脚使用下降沿触发 (`GPIO_INTR_NEGEDGE`)；在某些硬件上需要额外的上拉电阻或配置。
- PCA9555/TCA9555 地址随芯片型号与引脚配置不同，请以实际硬件为准。

---

## 贡献 & 版权

欢迎提交 issue 或 PR。当前仓库未明确声明许可证（LICENSE 文件），请在合并前添加合适的开源许可证。
