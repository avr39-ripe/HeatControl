#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <heatcontrol.h>

#include <SmingCore/SPI.h>

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

//    for(int i = 0; i < num_ch; i++)
//    {
//    	int curr_state = pinState(i);
//    	if (curr_state & JUSTPRESSED)
//    	{
//    		setState(out_reg, i, !getState(out_reg,i));
//    		Serial.print("ON ch "); Serial.println(i);
//    	}
//
//    	if (curr_state & JUSTRELEASED)
//		{
//			Serial.print("OFF ch "); Serial.println(i);
//		}
//    	if ((getState(in_reg,i) && (getState(in_reg_prev, i) == false)))
//    	{
//    		setState(in_reg_prev, i, true);
//    		setState(out_reg, i, !getState(out_reg,i));
//    		Serial.print("On ch "); Serial.println(i);
//    	}
//
//    	if (((getState(in_reg,i) == false) && (getState(in_reg_prev, i))))
//		{
//			setState(in_reg_prev, i, false);
//			//setState(out_reg, i, false);
//			Serial.print("Off ch "); Serial.println(i);
//		}
//    }
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

int pinState(int ch)
{
	if ((getState(in_reg, ch) && (getState(in_reg_prev, ch) == false)))
	{
		setState(in_reg_prev, ch, true);
		return (PRESSED | JUSTPRESSED | CHANGED);
	}
	else if (getState(in_reg, ch))
		return (PRESSED);

	if (((getState(in_reg, ch) == false) && (getState(in_reg_prev, ch))))
	{
		setState(in_reg_prev, ch, false);
		return (RELEASED | JUSTRELEASED | CHANGED);
	}
	else if (getState(in_reg, ch) == false)
		return (RELEASED);
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
