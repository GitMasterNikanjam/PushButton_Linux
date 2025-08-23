# Button_Linux — Push-Button Wrapper (AUXIO backend)

A small C++ library for handling push-buttons on Linux SBCs (Raspberry Pi, BeagleBone, x86)  
built on top of the **AUXIO** GPIO library (libgpiod v1.x).

This library lets you:
- Configure a GPIO line as a button input with **bias** (pull-up, pull-down, or none).
- Handle **polarity** automatically (active-high or active-low).
- Read the button state by polling or cached value.
- Use **interrupts** (rising, falling, or both edges) with optional debounce.
- Use the included `ResetButton` class to reboot or shut down the system when pressed/held.

---

## ✨ Features

- Simple wrapper over `AUXI` (input) from AUXIO.
- Supports bias: `0=off`, `1=pull-down`, `2=pull-up`.
- Supports polarity: `1=active-high`, `0=active-low`.
- Polling methods:
  - `value()` → raw electrical level (0/1/−1)
  - `read()` → logical pressed state (bool)
  - `get()`  → cached logical state (bool)
- Interrupt methods:
  - Rising, falling, or both edges
  - Software debounce (µs resolution)
  - C-style callback with kernel timestamp
- `ResetButton` utility:
  - Press → reboot
  - Hold through countdown → shutdown

---

## 📦 Requirements

- Linux with `/dev/gpiochipN` character devices
- **libgpiod v1.x** (≥1.6 recommended)  
  ```bash
  sudo apt install libgpiod-dev gpiod
  ```
- C++17 compiler
- AUXIO library (`AUXIO.h`, `AUXIO.cpp`)

---

## 🔧 Build

```bash
g++ -std=c++17 -O2 -lpthread -lgpiod     -o button_demo Button.cpp AUXIO.cpp button_demo.cpp
```

Run with root privileges or after configuring udev rules for GPIO.

---

## 🚀 Usage Examples

### Polling

```cpp
#include "Button.h"
#include <iostream>
#include <thread>

int main() {
    // active-low with pull-up
    Button btn("/dev/gpiochip0", 17, /*mode=*/0, /*bias=*/2);

    if (!btn.begin()) {
        std::cerr << "Error: " << btn.errorMessage << "\n";
        return 1;
    }

    while (true) {
        if (btn.read()) {
            std::cout << "Pressed!\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
```

### Interrupts

```cpp
#include "Button.h"
#include <atomic>
#include <csignal>
#include <iostream>

std::atomic<bool> running{true};
void on_sigint(int){ running.store(false); }

// Edge callback
void my_cb(bool rising, long sec, long nsec) {
    std::cout << (rising ? "Rising" : "Falling")
              << " at " << sec << "." << nsec << "\n";
}

int main() {
    Button btn("/dev/gpiochip0", 17, /*mode=*/0, /*bias=*/2);

    if (!btn.beginInterrupt(0, 5000, my_cb)) { // 0=Both edges
        std::cerr << "Error: " << btn.errorMessage << "\n";
        return 1;
    }

    std::signal(SIGINT, on_sigint);
    while (running.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    btn.clean();
}
```

### ResetButton

```cpp
#include "Button.h"

int main() {
    ResetButton rst("/dev/gpiochip0", 17, /*mode=*/0, /*bias=*/2);
    rst.begin();

    while (true) {
        rst.check(); // Press = reboot, Hold = shutdown
    }
}
```

---

## 🔍 API Overview

### `class Button`
- `Button(const char* chip, unsigned pin, uint8_t mode=1, uint8_t bias=0)`
- `bool begin()`
- `bool beginInterrupt(uint8_t edge=0, uint32_t debounce_us=5000, GpioCallback cb=nullptr)`
- `void stopInterrupt()`
- `void clean()`
- `int value()` → raw line value (0/1/−1)
- `bool read()` → logical pressed (polarity applied)
- `bool get()` → cached logical pressed

### `class ResetButton : public Button`
- `bool check()` — reboot or shutdown depending on hold duration

---

## ⚠️ Notes

- `value()` is raw (no polarity). Use `read()`/`get()` for logical “pressed”.
- Shutdown/reboot requires appropriate privileges.
- Ensure correct GPIO numbering (`gpioinfo` shows offsets).
- For libgpiod v2.x you will need to port this library.

---

## 📜 License

MIT License
