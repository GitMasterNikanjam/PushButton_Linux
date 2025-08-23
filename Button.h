/**
 * @file Button.h
 * @brief High-level push-button wrapper built on AUXIO::AUXI (libgpiod v1.x).
 *
 * This wrapper provides:
 *  - Simple configuration of a GPIO input line via AUXI
 *  - Polarity control (mode: active-high/active-low)
 *  - Bias control (bias: off / pull-down / pull-up)
 *  - Polling & cached reads:
 *      - value() -> RAW line level (0/1/-1)
 *      - read()  -> LOGICAL (polarity-applied) boolean
 *      - get()   -> cached LOGICAL boolean (no hardware access)
 *  - Event-driven interrupts with software debounce and a C-style callback
 *
 * A specialized ResetButton is also provided that triggers reboot or shutdown
 * depending on how long the button is held.
 */

// ################################################################################

#pragma once

// #################################################################################
// Include libraries:

#include "../AUXIO_Linux/AUXIO.h"      // uses AUXI from your latest AUXIO library
#include <cstdint>
#include <string>
#include <thread>
#include <chrono>

// #################################################################################

/**
 * @brief GPIO edge event callback signature used by Button::beginInterrupt().
 *
 * @param is_rising True for rising edge; false for falling edge.
 * @param sec       Kernel timestamp seconds component.
 * @param nsec      Kernel timestamp nanoseconds component.
 */
using GpioCallback = void(*)(bool is_rising, long sec, long nsec);

// #################################################################################
// Button class:

/**
 * @class Button
 * @brief Wrapper for a push-button input line (AUXI under the hood).
 *
 * **Polarity (`mode`)**
 * - 1 = active-high  (raw=1 → logical true/pressed)
 * - 0 = active-low   (raw=0 → logical true/pressed)
 *
 * **Bias (`bias`)**
 * - 0 = bias off
 * - 1 = pull-down
 * - 2 = pull-up
 *
 * The logical state exposed by read()/get() already applies polarity.
 * `value()` returns the RAW (non-inverted) digital level from the line.
 */
class Button
{
    public:

        /**
         * @brief Stores last error message (set if an operation fails).
         */
        std::string errorMessage;
        
        /**
         * @brief Construct a button on a given chip/line with chosen polarity and bias.
         *
         * @param gpiodChip_path Path to GPIO chip device (e.g., "/dev/gpiochip0").
         * @param pin            GPIO line offset on that chip.
         * @param mode           Polarity: 1=active-high (default), 0=active-low.
         * @param bias           Bias: 0=off (default), 1=pull-down, 2=pull-up.
         */
        Button(const char* gpiodChip_path, unsigned int pin, uint8_t mode = 1, uint8_t bias = 0);

        /**
         * @brief Request the line as input with configured bias/polarity.
         * @return true on success, false on failure (see @ref errorMessage).
         */
        bool begin(void);

        /**
         * @brief Start edge-driven interrupts (numeric edge selector).
         *
         * Requests kernel edge events and launches AUXI’s internal poll thread
         * with optional software debounce. The callback is invoked from that thread.
         *
         * @param edge         0=Both edges, 1=Rising only, 2=Falling only.
         * @param debounce_us  Debounce window in microseconds (default 5000).
         * @param cb           C-style callback pointer (must not be null).
         * @return true on success, false on error (see errorMessage).
         */
        bool beginInterrupt(uint8_t edge = 0, uint32_t debounce_us = 5000, GpioCallback cb = nullptr);

        /**
         * @brief Start edge-driven interrupts (type-safe overload).
         *
         * @param edge         Edge selection (AUXI::Edge::Both/Rising/Falling).
         * @param debounce_us  Debounce window in microseconds.
         * @param cb           C-style callback pointer (must not be null).
         * @return true on success, false on error (see errorMessage).
         */
        bool beginInterrupt(AUXI::Edge edge, uint32_t debounce_us, GpioCallback cb);

        /**
         * @brief Stop AUXI’s internal event thread (no-op if not running).
         */
        void stopInterrupt();

        /**
         * @brief Release the GPIO line/chip resources (safe to call multiple times).
         *
         * Calls @ref stopInterrupt() and then AUXI::clean().
         */
        void clean(void);

        /**
         * @brief Read current RAW digital level from the line.
         * @return 1 if high, 0 if low, or -1 on error.
         * @note For inputs this is the electrical level; use read()/get() for logical state.
         */
        int value(void);

        /**
         * @brief Read current LOGICAL state (hardware read + polarity applied).
         * @return true if pressed (logically active), false otherwise.
         */
        bool read(void);

        /**
         * @brief Get the last cached LOGICAL state (no hardware access).
         * @return true if last known state was pressed, false otherwise.
         */
        bool get(void);

    protected:

        AUXI        _auxi;          ///< Underlying AUXI instance (configured via constructor)
};

// ################################################################################
// ResetButton class:

/**
 * @class ResetButton
 * @brief Button with reboot/shutdown behavior based on hold time.
 *
 * Behavior:
 *  - If pressed: start a 3..2..1 countdown.
 *  - If still pressed after countdown: shutdown system.
 *  - If released before countdown ends: reboot system.
 */
class ResetButton : public Button
{
    public:

        using Button::Button; // inherit constructors

        /**
         * @brief Execute reboot/shutdown behavior.
         *
         * If pressed, prints a countdown (3..2..1). If still pressed
         * after countdown, performs shutdown; otherwise, performs reboot.
         *
         * @return true if button was initially pressed, false otherwise.
         * @note This call blocks while counting down and executing system calls.
         */
        bool check(void);

};





