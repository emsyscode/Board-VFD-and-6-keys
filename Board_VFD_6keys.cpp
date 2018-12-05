/****************************************************/
/* This is only one example of code structure       */
/* OFFCOURSE this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where can do changes and look to the results     */
/****************************************************/
//set your clock speed
#define F_CPU 16000000UL
//these are the include files. They are outside the project folder
#include <avr/io.h>
//#include <iom1284p.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define VFD_in 7// If 0 write LCD, if 1 read of LCD
#define VFD_clk 8 // Must be pulsed to LCD fetch data of bus
#define VFD_stb 9 // if 0 is a command, if 1 is a data0
#define AdjustPins    PIND // before is C, but I'm use port C to VFC Controle signals
unsigned char DigitTo7SegEncoder(unsigned char digit, unsigned char common);
/*Global Variables Declarations*/
unsigned char day = 7;  // start at 7 because the VFD start the day on the left side and move to rigth... grid is reverse way
unsigned char hours = 0;
unsigned char minutes = 0;
unsigned char seconds = 0;
unsigned char milisec = 0;
unsigned char points = 0;
unsigned char secs;
unsigned char digit;
unsigned char number;
unsigned char numberA;
unsigned char numberB;
unsigned char numberC;
unsigned char numberD;
unsigned char numberE;
unsigned char numberF;
unsigned char grid;
unsigned char wordA = 0;
unsigned char wordB = 0;
unsigned int k=0;
unsigned int segments[] ={
  // Here I'm forced to use the "0" as 10, because the 7 segments sart in "1"
  // This table is inverted
  //    This not respect the table for 7segm like "abcdefgh"  // 
      0b01110111, //0  // 0b11101110
      0b00100100, //1  // 0b00100100  // na primeira o 0b1 corresponde ao segmento 1 do VFD de 5 Grids
      0b01101011, //2  // 0b11010110 
      0b01101101, //3  // 0b10110110 
      0b00111100, //4  // 0b00111100
      0b01011101, //5  // 0b10111010
      0b01011111, //6  // 0b11111010
      0b01100100, //7  // 0b00100110
      0b01111111, //8  // 0b11111110
      0b01111100, //9  // 0b00111110
      0b00000000, //10  // 0b00000000  // empty display
  };
unsigned long grids[] ={
  //font data
  //  The grid on this display count from left to right  // 
    0b00010000, //  Grid 1
    0b00001000, //  Grid 2
    0b00000100, //  Grid 3
    0b00000010, //  Grid 4
    0b00000001, //  Grid 5
  };
void pt6312_init(void)
{
  delayMicroseconds(200); //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!
  // Configure VFD display (grids)
  cmd_with_stb(0b00000001);//  (0b01000000)    cmd1 5 grids 16 segm
  delayMicroseconds(1);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);//(BIN(01000000));
  delayMicroseconds(1);
  // Address 00H - 15H ( total of 11*2Bytes=176 Bits)
  cmd_with_stb(0b11000000);//(BIN(01100110)); 
  delayMicroseconds(1);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delayMicroseconds(1);
}
void cmd_without_stb(unsigned char a)
{
  // send without stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
void cmd_with_stb(unsigned char a)
{
  // send with stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
void test_VFD(void)
{
  /* 
  Here do a test for all segments of 5 grids
  each grid is controlled by a group of 2 bytes
  by these reason I'm send a burst of 2 bytes of
  data. The cycle for do a increment of 3 bytes on 
  the variable "i" on each test cycle of FOR.
  */
  // to test 6 grids is 6*3=18, the 8 gird result in 8*3=24.
 
  clear_VFD();
      
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b00000001); // cmd 1 // 5 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
        
         for (int i = 0; i < 5 ; i++){ // test base to 16 segm and 5 grids
         cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
         cmd_without_stb(0b11111111); // Data to fill table 5*16 = 80 bits
         }
    
      //cmd_without_stb(0b00000001); // cmd1 Here I define the 5 grids and 16 Segments
      //cmd_with_stb((0b10001000) | 7); //cmd 4
      digitalWrite(VFD_stb, HIGH);
      delay(1);
      delay(3000);  
}
void clear_VFD(void)
{
  /*
  Here I clean all registers 
  Could be done only on the number of grid
  to be more fast. The 12 * 3 bytes = 36 registers
  */
      for (int n=0; n < 10; n++){  // important be 10, if not, bright the half of wells./this on the VFD of 5 grids)
        cmd_with_stb(0b00000001); //       cmd 1 // 5 Grids & 16 Segments
        cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
            cmd_without_stb((0b11000000) | n); // cmd 3 //wich define the start address (00H to 15H)
            cmd_without_stb(0b00000000); // Data to fill table of 5 grids * 16 segm = 80 bits on the table
            //
            //cmd_with_stb((0b10001000) | 7); //cmd 4
            digitalWrite(VFD_stb, HIGH);
            delayMicroseconds(100);
     }
}
void AD16312_RunWeels(){
  int j, n;
  char x;
  short v = 0b0000000000000100;  // The short have a size of 16 bits(2 bytes)
 
        for (n=8; n < 10; n++){  //Note: only want write the position 9 & 10 of memory map (5 grids X 2 bytes)
          //clear_VFD();
            for(j = 0; j < 8; j++) {  // execute 8 times the for cycle
              //cmd1 Configure VFD display (grids) 
              cmd_with_stb(0b00000001);//  5 grids
              delay(1);  // 
              
              //cmd2 Write to memory display, increment address, normal operation 
              cmd_with_stb(0b01000000);//Teste mode setting to normal, Address increment Fixed, Write data to display memory...
              
              digitalWrite(VFD_stb, LOW);
              delay(1);
              //cmd3 Address 00H - 15H ( total of 11*2Bytes=176 Bits)
              cmd_without_stb((0b11000000) | n);//Increment active, then test all segments!
              delay(1); 
             if (n < 9 ){
              //Serial.println(n, DEC); // Only to debug
                ((x=v << 0) & 0x00FF);
                cmd_without_stb((x << j) & 0x00FF);
                //cmd_without_stb(0b00000001 << j);
              }
              else if ((n > 8 ) and (j < 8)){
               // Serial.println(n, DEC); // Only to debug
                //((x=v << 8) & 0xFF00);
                x=0b00000001;
                cmd_without_stb((x << j) & 0x7F); // Block the 8º bit wich brigth the CD symbol.
                //cmd_without_stb(0b00000001 << j);
              }
              
              delay(1);
              digitalWrite(VFD_stb, HIGH);
              //cmd4 set DIMM/PWM to value
              cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)//0 min - 7 max  )(0b01010000)
              delay(30);
            }
        }
}
void setup() {
// put your setup code here, to run once:
// initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
   Serial.begin(9600);
  seconds = 0x00;
  minutes =0x00;
  hours = 0x00;
  /*CS12  CS11 CS10 DESCRIPTION
  0        0     0  Timer/Counter1 Disabled 
  0        0     1  No Prescaling
  0        1     0  Clock / 8
  0        1     1  Clock / 64
  1        0     0  Clock / 256
  1        0     1  Clock / 1024
  1        1     0  External clock source on T1 pin, Clock on Falling edge
  1        1     1  External clock source on T1 pin, Clock on rising edge
 */
  // initialize timer1 
  cli();           // disable all interrupts
  // initialize timer1 
  //noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;// This initialisations is very important, to have sure the trigger take place!!!
  TCNT1  = 0;
  // Use 62499 to generate a cycle of 1 sex 2 X 0.5 Secs (16MHz / (2*256*(1+62449) = 0.5
  OCR1A = 62498;            // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= ((1 << CS12) | (0 << CS11) | (0 << CS10));    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
 
  
// Note: this counts is done to a Arduino 1 with Atmega 328... Is possible you need adjust
// a little the value 62499 upper or lower if the clock have a delay or advnce on hours.
   
//  a=0x33;
//  b=0x01;
CLKPR=(0x80);
//Set PORT
DDRD = 0xFF;  // IMPORTANT: from pin 0 to 7 is port D, from pin 8 to 13 is port B
PORTD=0x00;
DDRB =0xFF;
PORTB =0x00;
pt6312_init();
test_VFD();
clear_VFD();
//only here I active the enable of interrupts to allow run the test of VFD
//interrupts();             // enable all interrupts
sei();
}

/******************************************************************/
/************************** Update Clock **************************/
/******************************************************************/
void send_update_clock(void)
{
  if (secs >=60){
    secs =0;
    minutes++;
  }
  if (minutes >=60){
    minutes =0;
    hours++;
  }
  if (hours >=24){
    hours =0;
  }
    //*************************************************************
    DigitTo7SegEncoder(secs%10);
    //Serial.println(secs, DEC);
    numberA=number;
    DigitTo7SegEncoder(secs/10);
    //Serial.println(secs, DEC);
    numberB=number;
    SegTo32Bits();
    //*************************************************************
    DigitTo7SegEncoder(minutes%10);
    numberC=segments[number];
    DigitTo7SegEncoder(minutes/10);
    numberD=segments[number];
    SegTo32Bits();
    //**************************************************************
    DigitTo7SegEncoder(hours%10);
    numberE=segments[number];
    DigitTo7SegEncoder(hours/10);
    numberF=segments[number];
    SegTo32Bits();
    //**************************************************************
}
void SegTo32Bits(){
       //Serial.println(number,HEX);
  // This block is very important, it solve the difference 
  // between segments from grid 1 and grid 2(start 8 or 9)
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(10);
      cmd_with_stb(0b00000001); // cmd 1 // 5 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(10);
        cmd_without_stb((0b11000000) | grid); //cmd 3 wich define the start address (00H to 15H)
        wordB=segments[numberB];
              switch  (numberB){
              //skip the numbers 1,4,7,9 wich don't use the segment "1", equivalent to segment "d".
              case 0: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 1: wordA=segments[numberA];break;
              case 2: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 3: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 4: wordA=segments[numberA];break;
              case 5: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 6: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 7: wordA=segments[numberA];break;
              case 8: wordA=segments[numberA]; wordA=(wordA | 0x80);  break;
              case 9: wordA=segments[numberA];break;
              default: wordB=segments[numberB];break;
              }
     
          cmd_without_stb(wordA); // seconds unit
          cmd_without_stb(wordB >> 1); // seconds dozens  // Only this grid got a shift because the second  //  dozens
          cmd_without_stb((numberC) | 0x80); 
          cmd_without_stb(numberD);
          cmd_without_stb((numberE) | 0x80); 
          cmd_without_stb(numberF); 
           
      digitalWrite(VFD_stb, HIGH);
      delayMicroseconds(10);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(5);
       
}
void DigitTo7SegEncoder( unsigned char digit)
{
  switch(digit)
  {
    case 0:   number=0;     break;  // if remove the LongX, need put here the segments[x]
    case 1:   number=1;     break;
    case 2:   number=2;     break;
    case 3:   number=3;     break;
    case 4:   number=4;     break;
    case 5:   number=5;     break;
    case 6:   number=6;     break;
    case 7:   number=7;     break;
    case 8:   number=8;     break;
    case 9:   number=9;     break;
  }
} 
 
void adjustHMS(){
 // Important is necessary put a pull-up resistor to the VCC(+5VDC) to this pins (3, 4, 5)
 // if dont want adjust of the time comment the call of function on the loop
  /* Reset Seconds to 00 Pin number 3 Switch to GND*/
    if((AdjustPins & 0x08) == 0 )
    {
      _delay_ms(200);
      secs=00;
    }
    
    /* Set Minutes when SegCntrl Pin 4 Switch is Pressed*/
    if((AdjustPins & 0x10) == 0 )
    {
      _delay_ms(200);
      if(minutes < 59)
      minutes++;
      else
      minutes = 0;
    }
    /* Set Hours when SegCntrl Pin 5 Switch is Pressed*/
    if((AdjustPins & 0x20) == 0 )
    {
      _delay_ms(200);
      if(hours < 23)
      hours++;
      else
      hours = 0;
    }
}
void send7segm(){
  // This block is very important, it solve the difference 
  // between segments from grid 1 and grid 2(start 8 or 9)
  
      cmd_with_stb(0b00000001); // cmd 1 // 5 Grids & 16 Segments
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
              switch  (k){
              //skip the numbers 1,4,7,9 wich don't use the segment "1"
              case 0: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              case 2: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              case 3: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              case 5: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              case 6: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              case 8: wordA=segments[k]; wordB=(wordA | 0x80);  break;
              default: wordB=segments[k];
              }
     
          cmd_without_stb(wordB); // seconds unit
          cmd_without_stb((segments[k]) >> 1);  // Only this grid got a shift because the second  // seconds dozens
          cmd_without_stb(segments[k]);         // digit start a 8 instead of 9 like all others  //minuts units
          cmd_without_stb(segments[k]); //minuts dozens
          cmd_without_stb(segments[k]); // hours units
          cmd_without_stb(segments[k]); // hours dozens
          cmd_without_stb(segments[k]); // hours third digit not used
      digitalWrite(VFD_stb, HIGH);
      cmd_with_stb((0b10001000) | 7); //cmd 4
      delay(1);
      delay(1000);  
}
void readButtons(){
//Take special attention to the initialize digital pin LED_BUILTIN as an output.
//
int ledPin = 13;   // LED connected to digital pin 13
int inPin = 7;     // pushbutton connected to digital pin 7
int val = 0;       // variable to store the read value
int dataIn=0;
byte array[8] = {0,0,0,0,0,0,0,0};
byte together = 0;
unsigned char receive = 7; //define our transmit pin
unsigned char data = 0; //value to transmit, binary 10101010
unsigned char mask = 1; //our bitmask
array[0] = 1;
unsigned char btn1 = 0x41;
      digitalWrite(VFD_stb, LOW);
        delayMicroseconds(2);
      cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
       // cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 15H)
     // send without stb
  
  pinMode(7, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn 
  //on or off the pull up resistor  
  //This don't send the strobe signal, to be used in burst data send
         for (int z = 0; z < 3; z++){
             //for (mask=00000001; mask > 0; mask <<= 1) { //iterate through bit mask
                   for (int h =8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(inPin);
                      //digitalWrite(ledPin, val);    // sets the LED to the button's value
                           if (val & mask){ // if bitwise AND resolves to true
                             //Serial.print(val);
                            //data =data | (1 << mask);
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                            //Serial.print(val);
                           // data = data | (1 << mask);
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                    
                   } 
             
              Serial.print(z);
              Serial.print(" - " );
                        
                                  for (int bits = 7 ; bits > -1; bits--) {
                                      Serial.print(array[bits]);
                                   }
                        
                        if (z==1){
                          if(array[6] == 1){
                           hours++;
                          }
                        }
                          if (z==0){
                          if(array[2] == 1){
                           hours--;
                          }
                          }
                          if (z==0){
                          if(array[6] == 1){
                           minutes++;
                          }
                        }
                        if (z==0){
                          if(array[7] == 1){
                           minutes--;
                          }
                        }
                        if (z==0){
                          if(array[3] == 1){
                           secs++;
                          }
                        }
                          if (z==1){
                            if(array[7] == 1){
                              hours = 0;
                              minutes = 0;
                             secs=0;  // Set count of secs to zero to be more easy to adjust with other clocl.
                            }
                          }
                         
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();
 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(7, OUTPUT);  // Important this point!  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(1); 
 
}
void loop() {
  // You can comment untill while cycle to avoid the test running.
   test_VFD();
   clear_VFD();
   // Can use this cycle to teste all segments of VFD
   
       for(int h=0; h < 10; h++){ // Print digits from 0 to 9 1 by 1 to all grids.
       k=h;
       send7segm();
       }
  clear_VFD();
  
   while(1){
      send_update_clock();
      delay(200);
     readButtons();
      delay(200);
      AD16312_RunWeels();
   }
}
ISR(TIMER1_COMPA_vect)   {  //This is the interrupt request
                            // https://sites.google.com/site/qeewiki/books/avr-guide/timers-on-the-atmega328
      secs++;
} 
