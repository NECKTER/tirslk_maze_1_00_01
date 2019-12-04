// Reflectance.c
// Provide functions to take measurements using a QTR-8RC reflectance
// sensor array (Pololu part number 961).  This works by outputting to
// the sensor, waiting, then reading the digital value of each of the
// eight phototransistors.  The more reflective the target surface is,
// the faster the voltage decays.
// Student version of GPIO lab
// Daniel and Jonathan Valvano
// July 2, 2017

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


// reflectance even LED illuminate connected to P5.3 or both
// reflectance odd LED illuminate connected to P9.2
// reflectance sensor 1 connected to P7.0 (robot's right, robot off road to left)
// reflectance sensor 2 connected to P7.1
// reflectance sensor 3 connected to P7.2
// reflectance sensor 4 connected to P7.3 center
// reflectance sensor 5 connected to P7.4 center
// reflectance sensor 6 connected to P7.5
// reflectance sensor 7 connected to P7.6
// reflectance sensor 8 connected to P7.7 (robot's left, robot off road to right)

#include <stdio.h>
#include <stdint.h>
#include "msp432.h"
#include "..\inc\Clock.h"

// ------------Reflectance_Init------------
// Initialize the GPIO pins associated with the QTR-8RC
// reflectance sensor.  Infrared illumination LEDs are
// initially off.
// Input: none
// Output: none

void Reflectance_Init(void){
    // write this as part of Lab 6
    Clock_Init48MHz();

    //make ir sensors input init
    P7->SEL0 = 0x00;
    P7->SEL1 = 0x00;
    P7->DIR = 0x00;                         // make all of p7 an input
    P7->OUT = 0x00;

    //init controller
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;                        // make p5.3 output
    P5->OUT |= 0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;                        // make p9.2 output
    P9->OUT |= 0x04;

    //init P1
//    P1->SEL0 = 0x00;
//    P1->SEL1 = 0x00;
//    P1->DIR = 0xFF;                        // make p1 output
//    P1->REN = 0x00;
//    P1->OUT = 0x00;
}

// ------------Reflectance_Read------------
// Read the eight sensors
// Turn on the 8 IR LEDs
// Pulse the 8 sensors high for 10 us
// Make the sensor pins input
// wait t us
// Read sensors
// Turn off the 8 IR LEDs
// Input: time to wait in usec
// Output: sensor readings
// Assumes: Reflectance_Init() has been called
uint8_t Reflectance_Read(uint32_t time){//part2
uint8_t result;
    // write this as part of Lab 6
       P7->DIR |= 0xFF;    //set to output
       P7->OUT |= 0xFF;    //set output 1
       Clock_Delay1us(10);

       P7->DIR &= ~0xFF;    //set to input
       Clock_Delay1ms(20);

       result = P7->IN;

 return result; // replace this line
}

// ------------Reflectance_Center------------
// Read the two center sensors
// Turn on the 8 IR LEDs
// Pulse the 8 sensors high for 10 us
// Make the sensor pins input
// wait t us
// Read sensors
// Turn off the 8 IR LEDs
// Input: time to wait in usec
// Output: 0 (off road), 1 off to left, 2 off to right, 3 on road
// (Left,Right) Sensors
// 1,1          both sensors   on line
// 0,1          just right     off to left
// 1,0          left left      off to right
// 0,0          neither        lost
// Assumes: Reflectance_Init() has been called
uint8_t Reflectance_Center(uint32_t time){ //part1
    // write this as part of Lab 6
    uint8_t result;
    while(1){
    P5->OUT |= 0x08;    //set output 1
    P7->DIR |= 0x01;    //set to output
    P7->OUT |= 0x01;    //set output 1
    Clock_Delay1us(10);
    P7->DIR &= ~0x01;    //set to input
    for (int i = 0; i < 10000; i += 1) { //read the ir value as the output diminishes to 0
        result = P7->IN;
        P1->OUT = result;
    }
    P5->OUT &= ~0x08; //setoutput to 0
    Clock_Delay1ms(10);
    }
}


// Perform sensor integration
// Input: data is 8-bit result from line sensor
// Output: position in 0.1mm relative to center of line
int32_t Reflectance_Position(uint8_t data){//part3
    // write this as part of Lab 6
    int32_t position = 0;
    int w[] = {332,237,142,47,-47,-142,-237,-322};
    int irs[8];
    //data into array
    for (int i = 0; i < 8; ++i) {
        irs[7-i] = data%2;
        data = data >> 1;
    }

    //calculate position
    int count = 0;
    for (int i = 0; i < 8; ++i) {
        if(irs[i]==1){
            position += w[i];
            count++;
        }
    }
    if(!count) return -371;
    if(count == 8) return 371;//treasure
    position = position/count;
//    printf("%d\n",position);

 return position; // replace this line
}


// ------------Reflectance_Start------------
// Begin the process of reading the eight sensors 0x01FE00FF 0x03FC02FD
// Turn on the 8 IR LEDs
// Pulse the 8 sensors high for 10 us
// Make the sensor pins input
// Input: none
// Output: none
// Assumes: Reflectance_Init() has been called
void Reflectance_Start(void){
    // write this as part of Lab 10

        // write this as part of Lab 6
   P5->OUT |= 0x08;
   P9->OUT |= 0x04;

   P7->DIR |= 0xFF;    //set to output
   P7->OUT |= 0xFF;    //set output 1
   Clock_Delay1us(10);
   P7->DIR &= ~0xFF;    //set to input
}


// ------------Reflectance_End------------
// Finish reading the eight sensors
// Read sensors
// Turn off the 8 IR LEDs
// Input: none
// Output: sensor readings
// Assumes: Reflectance_Init() has been called
// Assumes: Reflectance_Start() was called 1 ms ago
uint8_t Reflectance_End(void){
    // write this as part of Lab 10
    uint8_t result = P7->IN;
    P5->OUT &= ~0x08;
    P9->OUT &= ~0x04;

 return result; // replace this line
}

