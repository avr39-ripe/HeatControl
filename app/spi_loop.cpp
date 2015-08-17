#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "../include/heatcontrol.h"
#include <SPI.h>

uint8_t out_reg[num_reg] = {255,255};
uint8_t in_reg[num_reg];
uint8_t in_reg_prev[num_reg] = {255,255};

void SPI_loop()
{
	counter++;
//for(int cnt = 0; cnt < 4; cnt++)
//{
  uint8_t first_bit;
  int regIndex;
  int bitIndex;
  
  digitalWrite(reg_in_latch, LOW);
  
  delayMicroseconds(5);
  
  digitalWrite(reg_in_latch, HIGH);
  
  digitalWrite(reg_out_latch, LOW);

  pinMode(miso_pin, INPUT); //workaround for 74hc165 "left shifted bit" bug
  first_bit = digitalRead(miso_pin); //read and store very first bit of 74hc165 chain outside regular SPI.transfer

  SPI.begin();

  for(int i = 0; i < num_reg; i++)
  {
    in_reg[i] = SPI.transfer(out_reg[num_reg - 1 - i]);
  }

  digitalWrite(reg_out_latch, HIGH);

  in_reg[1] = (in_reg[1] >> 1) | ((in_reg[0] & 1) << 7); //re-arrange bits in place
  in_reg[0] = (in_reg[0] >> 1) | (first_bit << 7); //here we use our stored first bit

//  for(int i = 0; i < num_reg; i++)
//  {
//    Serial.print("REG-IN"); Serial.print(i);Serial.print(" ");
//    print_byte(in_reg[i]);
//    Serial.print("REG-OUT"); Serial.print(i);Serial.print(" ");
//    print_byte(out_reg[i]);
//  }

    for(int i = 0; i < num_ch; i++)
    {
    	if ((getState(in_reg,i) && (getState(in_reg_prev, i) == false)))
    	{
    		setState(in_reg_prev, i, true);
    		setState(out_reg, i, true);
    		Serial.print("On ch "); Serial.println(i);
    	}

    	if (((getState(in_reg,i) == false) && (getState(in_reg_prev, i))))
		{
			setState(in_reg_prev, i, false);
			setState(out_reg, i, false);
			Serial.print("Off ch "); Serial.println(i);
		}
//    	setState(out_reg, i, getState(in_reg,i));
    }

//for(int i = 0; i < num_ch; i++)
//  {
//	regIndex = i >> 3;
//	bitIndex = i ^ regIndex << 3;
//
//	if((in_reg[regIndex] & (1 << bitIndex)) == 0)
//	{
//		out_reg[regIndex] &= ~(1 << bitIndex);
//	}
//	else
//	{
//		out_reg[regIndex] |= (1 << bitIndex);
//	}
//    debouncePin(i);
//     switch (i) {
//
//     default:
//    if ( inPins[i]._changed && inPins[i]._pressed) {
//           byteIndex = i / 8;
//           shiftIndex = i % 8;
//           out_reg[byteIndex] ^= (1 << shiftIndex);
//         }
//     }
//}

//} //cnt loop
}

bool inState(int ch)
{
	int regIndex = ch >> 3;
	int bitIndex = ch ^ regIndex << 3;

		if((in_reg[regIndex] & (1 << bitIndex)) == 0)
		{
			return true; //Active LOW
		}
		else
		{
			return false;
		}
}

void outSetState(int ch, uint8_t state)
{
	int regIndex = ch >> 3;
	int bitIndex = ch ^ regIndex << 3;

	if (state == 1)
	{
		out_reg[regIndex] &= ~(1 << bitIndex); //Active LOW
	}
	else
	{
		out_reg[regIndex] |= (1 << bitIndex);
	}
}

bool outGetState(int ch)
{
	int regIndex = ch >> 3;
	int bitIndex = ch ^ regIndex << 3;

	if((out_reg[regIndex] & (1 << bitIndex)) == 0)
	{
		return true; //Active LOW
	}
	else
	{
		return false;
	}
}

bool getState(uint8_t * reg, int ch)
{
	int regIndex = ch >> 3;
	int bitIndex = ch ^ regIndex << 3;

	if((reg[regIndex] & (1 << bitIndex)) == 0)
	{
		return true; //Active LOW
	}
	else
	{
		return false;
	}
}

void setState(uint8_t * reg, int ch, uint8_t state)
{
	int regIndex = ch >> 3;
	int bitIndex = ch ^ regIndex << 3;

	if (state == 1)
	{
		reg[regIndex] &= ~(1 << bitIndex); //Active LOW
	}
	else
	{
		reg[regIndex] |= (1 << bitIndex);
	}
}
// A function that prints all the 1's and 0's of a byte, so 8 bits +or- 2
void print_byte(byte val)
{
    byte i;
    for(byte i=0; i<=7; i++)
    {
      Serial.print(val >> i & 1, BIN); // Magic bit shift, if you care look up the <<, >>, and & operators
    }
    Serial.print("\n"); // Go to the next line, do not collect $200
}
