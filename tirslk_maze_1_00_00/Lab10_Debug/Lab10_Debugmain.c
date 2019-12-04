// Lab10_Debugmain.c
// Runs on MSP432
// Student version to Debug lab
// Daniel and Jonathan Valvano
// July 2, 2017
// Interrupt interface for QTR-8RC reflectance sensor array
// Pololu part number 961.
// Debugging dump, and Flash black box recorder

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/
// Negative logic bump sensors
// P8.7 Bump5
// P8.6 Bump4
// P8.5 Bump3
// P8.4 Bump2
// P8.3 Bump1
// P8.0 Bump0

// reflectance LED illuminate connected to P5.3
// reflectance sensor 1 connected to P7.0 (robot's right, robot off road to left)
// reflectance sensor 2 connected to P7.1
// reflectance sensor 3 connected to P7.2
// reflectance sensor 4 connected to P7.3 center
// reflectance sensor 5 connected to P7.4 center
// reflectance sensor 6 connected to P7.5
// reflectance sensor 7 connected to P7.6
// reflectance sensor 8 connected to P7.7 (robot's left, robot off road to right)

#include "msp.h"
#include "..\inc\bump.h"
#include "..\inc\Reflectance.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTickInts.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\FlashProgram.h"
#include "..\inc\Motor.h"
#include "..\inc\TimerA1.h"
#include "..\inc\TExaS.h"

int interrupt_count = 1;
#define SIZE 3
#define readFrequenct 10 //1 over this value is duty cycle
#define ROM_start 0x00020000
#define ROM_end 0x0003FFFF
#define REDLED (*((volatile uint8_t *)(0x42098060)))
#define BLUELED (*((volatile uint8_t *)(0x42098068)))

int32_t IRData[SIZE], previous[SIZE];
int32_t IRavg = 0;
int8_t i = 0, j = 0;
int max = 2000;
int adjust = 1000;
int turn = 500;
int wayOff = 0;
int count = 0;
int treasureFound = 0;

void TimedPause(uint32_t time){
  Motor_Stop();
  while(LaunchPad_Input()==0);  // wait for touch
  while(LaunchPad_Input());     // wait for release
  Clock_Delay1ms(time);          // run for a while and stop
}

void SysTick_Handler(void){ // every 1ms
  // write this as part of Lab 10
    if(interrupt_count == 0){//100Hz
        IRData[i] = Reflectance_Position(Reflectance_End());
        i = (i+1)%SIZE;
    }
    Reflectance_Start();//1000Hz
    interrupt_count = (interrupt_count + 1)%readFrequenct;
}

void collision(){
    if(Bump_Read() < 0x3F){
        Motor_Stop();
        Motor_Backward(max,max);
        Clock_Delay1ms(1500);
        Motor_Right(max,max);
        Clock_Delay1ms(750);
        Motor_Forward(max,max);
        Clock_Delay1ms(750);
        Motor_Stop();
    }
}

void bufferInit(){
    uint8_t i;
    for (i = 0; i < SIZE; ++i) {
        IRData[i] = 0;
        previous[i] = 0;
    }
}

int32_t average(int32_t *values, uint8_t size){
    int32_t average = 0;
    uint8_t i;
    for (i = 0; i < size; ++i) {
        average = average + values[i];
    }
    return average/size;
}

void right(int speed, int inplace){
    REDLED &= ~0x01;
    BLUELED |= 0x01;
    if(!inplace) Motor_Forward(max,max-speed);
    else Motor_Left(speed, speed);
    count = 0;

//    Clock_Delay1ms(50);
}

void left(int speed, int inplace){
    BLUELED &= ~0x01;
    REDLED |= 0x01;
    if(!inplace)Motor_Forward(max-speed,max);
    else Motor_Right(speed, speed);
    count = 0;

//    Clock_Delay1ms(100);
}

int main(void){
  // write this as part of Lab 10
    Clock_Init48MHz();
    LaunchPad_Init();
    Reflectance_Init();
    Bump_Init();
    Motor_Init();
    TimerA1_Init(&collision,5000);
    SysTick_Init((48*1000)*3, 0);
    EnableInterrupts();
    bufferInit();
    TimedPause(500);
//    TExaS_Init(LOGICANALYZER_P7);
  while(1){
  // write this as part of Lab 10
//      Motor_Stop();
//      Clock_Delay1ms(100);

      IRavg = average(IRData, SIZE);
      switch (IRavg) {
        case 371://treasure
            REDLED |= 0x01;
            BLUELED |= 0x01;
//            Motor_Right(max,max);
//            return 0;
            treasureFound = treasureFound + 1;
            TimedPause(500);
            break;
        case -370 ... -238://Way left
            left(4000, 1);
            break;
        case -237 ... -143://left
            left(max, 0);
            break;
        case -142 ... -71://slightly left
            left(adjust, 0);
            break;
        case -70 ... 70://centered
            REDLED |= 0x01;
            BLUELED |= 0x01;
            Motor_Forward(max,max);
            Clock_Delay1ms(50);
            break;
        case 71 ... 142://slightly right
            right(adjust, 0);
            break;
        case 143 ... 237://right
            right(max, 0);
            break;
        case 238 ... 370://way right
            right(4000, 1);
            break;
        default://spin??!!Lost
//          REDLED &= ~0x01;
                BLUELED &= ~0x01;
                Motor_Backward(max/2,max/2);
                Clock_Delay1ms(10);
                count = count + 1;
            if(count == 19){
                BLUELED |= ~0x01;
                Motor_Backward(max,max);
                Clock_Delay1ms(850);
                Motor_Left(max, max);
                Clock_Delay1ms(1000);
                count = 0;
/*                if(count2 == 1){
                    Motor_Left(-max, max);
                    Clock_Delay1ms(10000);
                    break;
                }*/
//                count2 = 1;
            }
            break;
          }//if backs up a few times in a row go back and turn left
      previous[j] = IRavg;
      j = (j+1)%SIZE;
  }
}
