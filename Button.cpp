
// #######################################################################
// Inclide Libraries:

#include "Button.h"
#include <cstdlib>

// #######################################################################
// Button class:

// Map legacy PUD values (0=OFF,1=DOWN,2=UP) to AUXI bias enum.
// Adjust names if your AUXIO exposes different identifiers.
static AUXI::Bias toAuxiBias(uint8_t pud) 
{
    switch (pud) 
    {
        case 1: return AUXI::Bias::PullDown;
        case 2: return AUXI::Bias::PullUp;
        default: return AUXI::Bias::Disabled;
    }
}

Button::Button(uint8_t pin, uint8_t pud)
: _pin(pin), _pud(pud), _auxi(pin) {}

bool Button::begin(void)
{
    if(_pin > 30)
    {
        errorMessage = "Error Button object: pin configuration is in wrong range.";
        return false;
    }

    // Configure as input with bias via AUXI
    if (!_auxi.beginInput(toAuxiBias(_pud))) { // <-- implement in AUXIO
    errorMessage = "AUXI beginInput() failed.";
    return false;
    }
    _irqActive.store(false);

    return true;
}

bool Button::beginInterrupt(GpioCallback cb, bool both_edges, uint32_t debounce_us) 
{
    if (!cb) 
    {
        errorMessage = "Callback is null.";
        return false;
    }

    if (!_auxi.beginInput(toAuxiBias(_pud))) 
    { // ensure configured
        errorMessage = "AUXI beginInput() failed.";
        return false;
    }

    AUXI::Edge edge = both_edges ? AUXI::Edge::Both : AUXI::Edge::Rising;
    if (!_auxi.beginInterrupt(edge, cb, debounce_us)) 
    { // <-- implement in AUXIO
        errorMessage = "AUXI beginInterrupt() failed.";
        return false;
    }
    _irqActive.store(true);
    return true;
}

void Button::clean() 
{
    if (_irqActive.load()) 
    {
        _auxi.endInterrupt(); // <-- implement in AUXIO (optional)
        _irqActive.store(false);
    }
    _auxi.end(); // release line (return to default/floating per kernel bias)
}

uint8_t Button::value() 
{
    int v = _auxi.read(); // <-- implement in AUXIO: return 0/1
    return static_cast<uint8_t>(v & 1);
}

uint8_t Button::state() 
{
    const uint8_t val = value();
    if (_pud == 2 /*pull-up*/) 
    {
        return val == 0 ? 1 : 0; // active-low when pull-up
    }
    return val; // active-high otherwise
}


// ####################################################################
// ResetButton class:

bool ResetButton::check() 
{
    if (!state()) return false;

    using namespace std::chrono_literals;
    std::cout << "System is Resetting ... !\n";
    std::this_thread::sleep_for(1s);
    std::cout << "3 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "2 Sec\n";
    std::this_thread::sleep_for(1s);
    std::cout << "1 Sec\n";
    std::this_thread::sleep_for(1s);

    if (state()) 
    {
        std::cout << "System is Shutdown ... !\n";
        std::this_thread::sleep_for(1s);
        // Prefer full path to be explicit; adjust to your distro if needed.
        std::system("sudo /sbin/shutdown -h now");
        for (;;) std::this_thread::sleep_for(1s);
    }

    std::system("sudo /sbin/reboot");
    for (;;) std::this_thread::sleep_for(1s);

    return false;
}



