#include "Button.h"
#include "Button.h"

Button::Button(uint8_t pin, uint8_t pud)
{
    _pin = pin;
    _pud = pud;
}

bool Button::begin(void)
{
    if(_pin > 30)
    {
        errorMessage = "Error Button object: pin configuration is in wrong range.";
        return false;
    }

    bcm2835_gpio_fsel(_pin, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(_pin, _pud);

    return true;
}

void Button::_InterruptHandler(int /*gpio*/, int /*level*/, uint32_t /*tick*/) 
{

}

void Button::clean(void)
{
    bcm2835_gpio_fsel(_pin, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_set_pud(_pin, BCM2835_GPIO_PUD_OFF);
}

uint8_t Button::value(void)
{
    return bcm2835_gpio_lev(_pin);
}

uint8_t Button::state(void)
{
    uint8_t val = value();

    if(_pud == BCM2835_GPIO_PUD_UP)
    {
        if(val == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    return val;
}


// #########################################################

bool ResetButton::check(void)
{
    if (state())
    {
        cout<< "System is Resetting ... !"<< endl;
        bcm2835_delay(1000);
        cout<< "3 Sec"<< endl;
        bcm2835_delay(1000);
        cout<< "2 Sec"<< endl;
        bcm2835_delay(1000);
        cout<< "1 Sec"<< endl;
        bcm2835_delay(1000);
        if (state())
        {
            cout<< "System is Shutdown ... !"<< endl;
            bcm2835_delay(1000);
            system("sudo shutdown now");
            while(1);
        }
        system("sudo reboot");
        while(1);
    }

    return false;
}