#pragma once

// #################################################################################
// Include libraries:

#include "../AUXIO_Linux/AUXIO.h"    // <- Your AUXIO input wrapper (AUXI)
#include <cstdint>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <functional>

using namespace std;

// #################################################################################

// Callback signature: (is_rising, seconds, nanoseconds)
using GpioCallback = void(*)(bool, long, long);

// #################################################################################
// Button class:

/**
* @brief Simple button wrapper on top of AUXIO (AUXI input with optional interrupts).
*
* API kept close to the original Button class, but internally uses AUXI.
* - Pull modes use the same numeric convention as before: OFF=0, DOWN=1, UP=2.
* - beginInterrupt() is added for edge-driven behavior with optional debounce.
*/
class Button
{
    public:

        std::string errorMessage;
        
        /**
        * @param pin GPIO line offset (per your platform's mapping)
        * @param pud Pull mode: 0=OFF, 1=DOWN, 2=UP
        */
        Button(uint8_t pin, uint8_t pud);

        /**
        * @brief Configure the GPIO as input with bias via AUXI. No interrupts.
        * @return true on success, false on error (check errorMessage)
        */
        bool begin(void);

        /**
        * @brief Start edge-driven interrupts (rising+falling by default).
        * @param cb C-style callback (no lambda required)
        * @param both_edges true=rising+falling, false=rising-only
        * @param debounce_us debounce time in microseconds (0 to disable)
        * @return true on success
        */
        bool beginInterrupt(GpioCallback cb, bool both_edges=true, uint32_t debounce_us=5000);

        /** Stop interrupts (if started) and release resources. */
        void clean(void);

        /** @return raw logic level from the line (0/1). */
        uint8_t value(void);

        /**
        * @brief Button pressed state considering pull mode.
        * If pull-up is used (2), pressed = level==0.
        * Otherwise, pressed = level.
        */
        uint8_t state(void);

    protected:

        // GPIO pin number
        uint8_t _pin;
        
        // Pullup/Pulldown mode. PUD_OFF:0, PUD_DOWN:1, PUD_UP:2 
        uint8_t _pud;

        // Backing AUXI input object (from AUXIO library)
        AUXI _auxi; // constructed with pin; see source for details

        // Track whether interrupt mode is active
        std::atomic<bool> _irqActive {false};

        // // Duration time that user pressed button. [us]
        // uint64_t _pressedDur;

        // uint64_t _T;
};

// ################################################################################
// ResetButton class:

/**
* @brief Specialized button that triggers reboot/shutdown after a press/hold.
* Behavior preserved from original code but implemented without bcm delays.
*/
class ResetButton : public Button
{
    public:

        using Button::Button; // inherit constructor

        /**
        * @brief If pressed, perform reboot; if still pressed after countdown, shutdown.
        * Blocks while counting down.
        */
        bool check(void);

};





