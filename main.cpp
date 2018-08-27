#define F_CPU 8000000
#include <avr/io.h>
#include "rotary.h"
#include "si5351.h"

#define ENCODER_A    3                      // Encoder pin A
#define ENCODER_B    2                      // Encoder pin B


int main(void)
{

	Rotary r = Rotary(ENCODER_A, ENCODER_B);
	Si5351 si5351;
    
	while(1)
    {
        
    }
}
