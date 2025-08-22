# Button Library (AUXIO-based)

A simple C++ library for handling push-buttons on Linux SBCs (e.g., Raspberry Pi, BeagleBone) using the **AUXIO** backend (libgpiod-based).  
This library replaces older implementations that relied on **bcm2835** or **pigpio**.

---

## Features

- Configure a GPIO pin as a button input with optional pull-up/pull-down bias.
- Poll the button state (`pressed` / `not pressed`).
- Edge-driven **interrupt callbacks** with optional debounce.
- Includes a `ResetButton` class that reboots or shuts down the system after a press/hold.

---

## Requirements

- C++17 or newer
- [libgpiod](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git) (v1.6+ recommended)
- AUXIO library (your wrapper around libgpiod — provides `AUXI` class)

---

## Build

```bash
g++ -std=c++17 -O2 -lpthread -lgpiod -o button_demo \
    button_demo.cpp Button.cpp AUXIO.cpp
````

---

## Usage Example

### Polling a Button

```cpp
#include "Button.h"
#include <iostream>
#include <thread>

int main() {
    Button btn(17, 2); // GPIO17, pull-up

    if (!btn.begin()) {
        std::cerr << "Error: " << btn.errorMessage << "\n";
        return 1;
    }

    while (true) {
        if (btn.state()) {
            std::cout << "Button pressed!\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}
```

### Interrupt-driven Callback

```cpp
#include "Button.h"
#include <iostream>
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void handler(bool rising, long sec, long nsec) {
    std::cout << (rising ? "Rising" : "Falling")
              << " edge at " << sec << "." << nsec << "\n";
}

int main() {
    std::signal(SIGINT, [](int){ running = false; });

    Button btn(17, 2); // GPIO17, pull-up
    if (!btn.beginInterrupt(handler, true, 5000)) {
        std::cerr << "Error: " << btn.errorMessage << "\n";
        return 1;
    }

    std::cout << "Waiting for button events...\n";
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    btn.clean();
}
```

### ResetButton Example

```cpp
#include "Button.h"

int main() {
    ResetButton rstBtn(17, 2); // GPIO17, pull-up
    rstBtn.begin();

    while (true) {
        rstBtn.check(); // Press = reboot, Hold = shutdown
    }
}
```

---

## API Overview

### `class Button`

* `Button(uint8_t pin, uint8_t pud)`
  Create a button object. `pud`: 0=OFF, 1=DOWN, 2=UP.
* `bool begin()`
  Configure pin as input with bias.
* `bool beginInterrupt(GpioCallback cb, bool both_edges=true, uint32_t debounce_us=5000)`
  Start edge-detection with callback.
* `void clean()`
  Release resources.
* `uint8_t value()`
  Get raw logic level (0/1).
* `uint8_t state()`
  Get button state (pressed = 1, released = 0).

### `class ResetButton : public Button`

* `bool check()`
  If pressed → reboot. If held through countdown → shutdown.

---

## Notes

* Requires running with proper permissions to access `/dev/gpiochipN`.
* Shutdown/reboot functions require **sudo** privileges.
* Adjust GPIO pin numbering according to your platform.

---

## License

MIT License

