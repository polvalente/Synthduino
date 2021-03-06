# Synthduino
8-bit synthesizer based on the Mozzi Arduino library (http://sensorium.github.io/Mozzi/)

This project was written as part of the "Projeto Integrado" class of the Federal University of Rio de Janeiro.

## Modifications to the Mozzi Library

The main modifications needed were implemented as the flag USE_PORT_OUT, which should be defined in the mozzi_config.h header file.

If the flag is defined and AUDIO_MODE is STANDARD_PLUS, the defined flag "#OUT_PORT PORTC" will use the respective PORT of the arduino as the output bus for the 8-bit data. "#OUT_PORT_DDR" should be defined accordingly.

This flag takes effect on the MozziGuts.cpp file. It works by bypassing the PWM function from the TimerOne library and writing the output_buffer samples to the chosen output port. Some reading in Arduino Port Manipulation can be found <a href="https://www.arduino.cc/en/Reference/PortManipulation">here</a>

