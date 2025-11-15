# i2c-pca9555-extension

ESP-IDF driver for PCA9555/TCA9555 I2C 16-bit I/O expander (multi-device support & interrupt callback)

This project provides a simple 16-channel digital I/O expander driver for PCA9555/TCA9555-compatible chips and includes example code.

Features

- Support multiple devices on a single I2C bus (up to 8 devices)
- Optional INT pin handling with callback
- Read/write single IO pin and read/write 16-bit registers
- Only supports digital IO (no PWM/ADC/DAC)

Overview

- Add a device using `i2c9555_add_device()`
- Configure IO directions using `i2c9555_ioconfig()` (1 = input, 0 = output)
- Read/write single pins using `i2c9555pin_read()` / `i2c9555pin_write()`
- If INT pin configured, driver invokes a callback when inputs change

Hardware

- ESP32 (ESP-IDF build environment)
- PCA9555 or compatible TCA9555 I2C I/O expander
- I2C lines: SDA / SCL
- Optional: INT pin to ESP32 GPIO (the driver uses falling edge interrupt by default)

Wiring example

- VCC -> 3.3V
- GND -> GND
- SDA -> ESP32 SDA (e.g. GPIO21)
- SCL -> ESP32 SCL (e.g. GPIO22)
- INT -> ESP32 interrupt GPIO (optional)

Make sure the address pins (A0/A1/A2 or A2/A1/A0 depending on the RI chip) are configured according to your hardware.

Address (summary)

TCA9555 address (7-bit write address variants):

| A2 | A1 | A0 | Address (Write) |
|----|----|----|-----------------|
| GND| GND| GND| 0x40            |
| GND| GND| VCC| 0x42            |
| GND| VCC| GND| 0x44            |
| ...|    |    | ...             |

PCA9555 address (7-bit):

| A2 | A1 | A0 | Address |
|----|----|----|---------|
| L  | L  | L  | 0x20    |
| L  | L  | H  | 0x21    |
| ...|    |    | ...     |

Quick start

1. Add a device

```c
int device = i2c9555_add_device(GPIO_NUM_21, GPIO_NUM_22, 0x20, GPIO_NUM_10, i2c_io_callback);
```

- Parameters:

- sda, scl: ESP32 I2C pins
- addr: device address (see table above)
- int_pin: INT pin (GPIO_NUM_NC to disable)
- callback: optional input-change callback

Returns device id (0-7) or -1 on failure.

1. Configure IO direction

```c
// 16-bit configuration: high bits map to port1 (GPIO8..15), low bits to port0 (GPIO0..7)
// 1 = input, 0 = output
i2c9555_ioconfig(device, 0x0000); // all outputs
i2c9555_ioconfig(device, 0xFFFF); // all inputs
```

1. Read/write a single pin

```c
// Write
i2c9555pin_write(device, EXT_IO0, HIGH);
// Read
int state = i2c9555pin_read(device, EXT_IO5);
```

1. Interrupt callback

The driver calls your callback when an input change is detected:

```c
void i2c_io_callback(uint16_t pin, int level) {
    // pin is a bitmask (EXT_IO0, EXT_IO1, ...), level is 0/1
}
```

API overview

- int i2c9555_add_device(gpio_num_t sda, gpio_num_t scl, uint16_t addr, gpio_num_t int_pin, i2c9555_input_cb_t f);
- esp_err_t i2c9555_ioconfig(uint8_t device_id, uint16_t config);
- int i2c9555pin_read(uint8_t device_id, uint16_t pin);
- esp_err_t i2c9555pin_write(uint8_t device_id, uint16_t pin, int level);
- esp_err_t i2c9555_write_word(uint8_t device_id, uint8_t reg, uint16_t data);
- esp_err_t i2c9555_read_word(uint8_t device_id, uint8_t reg, uint16_t *data);

Macros:

- EXT_IO0 ... EXT_IO17 (16 IOs in header, some are marked NULL in the header file)
- HIGH/LOW

Example (from `main.c`)

```c
device = i2c9555_add_device(GPIO_NUM_9, GPIO_NUM_8, 0x20, GPIO_NUM_10, i2c_io_callback);
vTaskDelay(pdMS_TO_TICKS(10));
i2c9555_ioconfig(device, 0b0011111111111111); // configure upper 13 bits as inputs

// blink task
xTaskCreatePinnedToCore(blink_test, "blink_test", 4096, NULL, 2, NULL, 0);
```

Building and flashing (ESP-IDF)

On Windows PowerShell, after sourcing ESP-IDF environment (e.g., using `export.ps1`) do:

```pwsh
idf.py build
idf.py -p COM3 flash monitor
```

Replace `COM3` with your board port. Use `idf.py -p <PORT> erase-flash` to clear flash if needed.

Limitations and notes

- Driver only supports basic digital IO.
- INT pin triggers on falling edge (`GPIO_INTR_NEGEDGE`)—external or internal pull-up may be needed depending on your wiring.
- PCA9555/TCA9555 addresses differ by chip model and address pin wiring—refer to your chip datasheet.

Contributing & License

Contributions via issues and pull requests are welcome. The repository currently does not have a LICENSE file—please add a suitable open source license (e.g., MIT or Apache-2.0) before publishing.
