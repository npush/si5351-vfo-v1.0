#define F_CPU 8000000
#include "Arduino.h"
#include "rotary.h"
#include "si5351.h"


// Declared weak in Arduino.h to allow user redefinitions.
int atexit(void (* /*func*/ )()) { return 0; }

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

#define ENCODER_A    3                      // Encoder pin A
#define ENCODER_B    2                      // Encoder pin B

int main(void)
{
	init();

	initVariant();

	Rotary r = Rotary(ENCODER_A, ENCODER_B);
	Si5351 si5351;
    
	while(1)
    {
        if (serialEventRun) serialEventRun();
    }
		return 0;
}
