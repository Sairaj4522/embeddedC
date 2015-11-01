/*************************************************************
*	Microcontroller			:ATmega32
*	Date: 22-JANUARY-2015
*	Last modified: 12-FEBRUARY-2015
*
*	10-Feb-2015
*	Swapped PORTB and PORTD Connections so as to solve PCB routing problem
*
*	11-Feb-2015
*	Completed PCB design.
*	Made hardware modifications for later maintenance.
*	two more diodes used so if one pin of ULN2003 doesn't work
*	we can programmatically  solve the issue by just changing
*	the output configuration on Port B
*
*	12-Feb-2015
*	Editing PCB for final changes
*
*	Previous Button mapping:
*	PORTA:
*	OK		0x01
*	BACK	0x02
*	ADC		0x04
*	P1		0x08
*	P2		0x10
*	MENU	0x20
*
*	Changed button connections for Mode switch and Menu button
*	Latest Button Mapping:
*	PORTA:
*	OK		0x01
*	BACK	0x02
*	ADC		0x04
*	MENU	0x08
*	P1		0x10
*	P2		0x20
*	Code updated to use latest mapping
*	and the PCB design was completed. The final PCB layout saved  ~/bkps as eagle120220150019bkp.zip
*	
* 	Made a couple more changes to PCB layout and the final PCB Rev1.0 is backed up in ~/bkps as eagle120220151016bkp.zip
*
*	13-Feb-2015
*	One possibility brought up today about the Autoclk 2.0 Duos (installed in Homeopathy College)
*	problem of showing wierd characters on display and the lcd getting stuck when the relay is activated
*	the problem is that the way the power supply circuit on the board.
*	The board has a DC connector to which we connect 12v 1A adapter, following that is the 7805 voltage regulator
*	and this Voltage regulator supplies power to the entire circuit, the other part connected to the 12V supply is the Relay
*	and ULN2003 relay driver. So the entire circuit in view of the Power supply (12V Dc adapter) is
*	like 7805 circuit(and the circuit it powers i.e microcontroller, LCD and other IC and component connected to 5v dc supply)
*	in PARALLEL with the Relay circuit(which includes ULN2003 and the Relay).
*	So when the Relay is activated, the relay will draw a certain amount of current so as to lock the coil, and since it's in parallel
*	with the 7805 circuit there will be less current flowing in 7805 circuit as compared to the previous condition when Relay wasnt active.
*	So this is a possibility why there's problem with that autoclk. Now same power supply circuit design has been implemented in Autoclk 3.0
*	hence we will face same problems.
*
*	Added one more Mode switch to the PCB design so as to support Duos version of the Autoclk Product.
*	Need to update code so as to support that functionality.
*
*	23-Oct-2015
*	Added Buzzer to eagle schematic and pcb, and final change has been done for this version of AutoClk
*
*	Port C Pins usage
*	PC2		Buzzer
*	PC3		bell1
*	//Only hardware support so far
*	PC4		bell2
*	PC5		bell3
*	PC6		bell4
*	PC7		bell5
*
*
*************************************************************/

#include <avr/io.h>
/*Includes io.h header file where all the Input/Output Registers and its Bits are defined for all AVR microcontrollers*/
#include<avr/eeprom.h>
/*Includes EEPROM header file where all the EEPROM Registers and its Bits are defined for all AVR microcontrollers*/
#define	F_CPU	16000000
/*Defines a macro for the delay.h header file. F_CPU is the microcontroller frequency value for the delay.h header file. Default value of F_CPU in delay.h header file is 1000000(1MHz)*/

#include <util/delay.h>
/*Includes delay.h header file which defines two functions, _delay_ms (millisecond delay) and _delay_us (microsecond delay)*/

#define		LCD_DATA_PORT		PORTD
/*Defines a macro for the lcd.h header File. LCD_DATA_PORT is the microcontroller PORT Register to which the data pins of the LCD are connected. Default PORT Register for data pins in lcd.h header file is PORTA*/

#define 	LCD_CONT_PORT		PORTB
/*Defines a macro for the lcd.h header File. LCD_CONT_PORT is the microcontroller PORT Register to which the control pins of the LCD are connected. Default PORT Register for control pins in lcd.h header file is PORTB*/

#define 	LCD_RS 		PB0
/*Defines a macro for the lcd.h header file. LCD_RS is the microcontroller Port pin to which the RS pin of the LCD is connected. Default Port pin for RS pin in lcd.h header file is PB0*/

#define 	LCD_RW 		PB1
/*Defines a macro for the lcd.h header file. LCD_RW is the microcontroller Port pin to which the RW pin of the LCD is connected. Default Port pin for RW pin in lcd.h header file is PB1*/

#define 	LCD_EN 		PB2
/*Defines a macro for the lcd.h header file. LCD_EN is the microcontroller Port pin to which the EN pin of the LCD is connected. Default Port pin for EN pin in lcd.h header file is PB2*/

#include "lcd.h"
/*Includes lcd.h header file which defines different functions for all Alphanumeric LCD(8-Bit Interfacing Method). LCD header file version is 1.1*/

#include "ds1307.h"
/*Includes ds1307.h header file which defines different functions for DS1307 Real Time Clock. DS1307 header file version is 1.1*/

//------------------------------------------------------------------------------
// define Real Time Clock register addresses
//------------------------------------------------------------------------------
#define SECOND	 0x00
#define MINUTE 	 0x01
#define HOUR 	 0x02
#define WEEKDAY  0x03
#define DATE 	 0x04
#define MONTH    0x05
#define YEAR     0x06
#define CONTROL  0x07

//eeprom section
//the maximum timings that can be set are 59
//since its stored in eeeprom as words so
//the total bytes/address required to store
//timings for each mode is 118 + 2(intervals i.e. count for the storing the no. of timings entered)
//by spacing by 50 we store in eeprom starting from 0 then next mode at 170 and next at 340 and so on...
#define MODE0 0
#define MODE1 170
#define MODE2 340

//int h1=0;
//int h0=0;
//int s0=0;
unsigned char temp1;
/*char *ampm_bit[] = {
                    "AM",
					"PM",
					};

char *display_format[] = {
					      "  :  :  ",
                          "  /  /  ",
		       	         };*/

// global declaration
unsigned char date, month, year, hr, min, sec;



//adc to set time of the clock
//unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;

//Function to Initialize ADC
void adc_init() {
	ADCSRA = 0x00;
	ADMUX = 0x20;	                //Vref=5V external --- ADLAR=1 --- MUX4:0 = 0000
	ACSR = 0x80;
	ADCSRA = 0x86;		            //ADEN=1 --- ADIE=1 --- ADPS2:0 = 1 1 0
}


//This Function returns the corresponding Analog Value

uint8_t ReadVoltage(void)
{
	ADMUX = _BV(ADLAR) | _BV(REFS1) | _BV(REFS0) | _BV(MUX1);//Left Adjust
		ADCSRA |= _BV(ADSC); // start conversion
		while (ADCSRA & (1 << ADSC));
		return ADCH;

}

// This Function prints the Analog Value Of Corresponding Channel No. at required Row
// and Coloumn Location.
int print_hour(char row, char coloumn,unsigned char channel) {
	ADC_Value = ReadVoltage();
	ADC_Value = (ADC_Value*24) / 255;
	//lcd_print(row, coloumn, ADC_Value, 2);

	if(ADC_Value <= 23)
	{
		if(ADC_Value<=9)
					{
						lcd_cursor(row,coloumn);
						lcd_string_write("0");
						lcd_cursor(row,coloumn+1);
						lcd_number_write(ADC_Value,10);
					}
			if(ADC_Value>9)
				{
					lcd_cursor(row,coloumn);
					lcd_number_write(ADC_Value,10);
				}
		return ADC_Value;
	}
    if(ADC_Value >23)
	   return 23;


}

int print_minute(char row, char coloumn,unsigned char channel) {
	ADC_Value = ReadVoltage();
	ADC_Value = (ADC_Value*60) / 255;
	//lcd_print(row, coloumn, ADC_Value, 2);
	lcd_cursor(row,coloumn);
	if(ADC_Value <= 59)
	{
			if(ADC_Value<=9)
					{
						lcd_cursor(row,coloumn);
						lcd_string_write("0");
						lcd_cursor(row,coloumn+1);
						lcd_number_write(ADC_Value,10);
					}
			if(ADC_Value>9)
				{
					lcd_cursor(row,coloumn);
					lcd_number_write(ADC_Value,10);
				}
			return ADC_Value;
	}
	if(ADC_Value >59)
	   return 59;


}

int print_yr(char row, char coloumn,unsigned char channel) {
	ADC_Value = ReadVoltage();
	ADC_Value = (ADC_Value*100) / 255;
	if(ADC_Value <= 99)
		{
			if(ADC_Value<=9)
						{
							lcd_cursor(row,coloumn);
							lcd_string_write("0");
							lcd_cursor(row,coloumn+1);
							lcd_number_write(ADC_Value,10);
						}
				if(ADC_Value>9)
					{
						lcd_cursor(row,coloumn);
						lcd_number_write(ADC_Value,10);
					}
				return ADC_Value;
		}
		if(ADC_Value >99)
		   return 99;
}

int print_date(char row, char coloumn,unsigned char channel) {
	ADC_Value = ReadVoltage();
	ADC_Value = (ADC_Value*31) / 255;
	if(ADC_Value <= 31)
		{
			if(ADC_Value<=9)
						{
							if(ADC_Value<1)
								ADC_Value=1;
							lcd_cursor(row,coloumn);
							lcd_string_write("0");
							lcd_cursor(row,coloumn+1);
							lcd_number_write(ADC_Value,10);
						}
				if(ADC_Value>9)
					{
						lcd_cursor(row,coloumn);
						lcd_number_write(ADC_Value,10);
					}
				return ADC_Value;
		}
		if(ADC_Value >31)
		   return 31;
}

int print_month(char row, char coloumn,unsigned char channel) {
	ADC_Value = ReadVoltage();
	ADC_Value = (ADC_Value*12) / 255;
	if(ADC_Value <= 11)
		{
			if(ADC_Value<=9)
						{
							lcd_cursor(row,coloumn);
							lcd_string_write("0");
							lcd_cursor(row,coloumn+1);
							lcd_number_write(ADC_Value+1,10);
						}
				if(ADC_Value>9)
					{
						lcd_cursor(row,coloumn);
						lcd_number_write(ADC_Value+1,10);
					}
				return ADC_Value+1;
		}
		if(ADC_Value >11)
		   return 12;
}

//Function to display date and time on the LCD

void display_time(void) {
	char *weekdays[] = {
	                    "SUN",
	                    "MON",
						"TUE",
	                    "WED",
						"THU",
	                    "FRI",
	                    "SAT",
	    	       	   };
	//unsigned char second_byte = 0,minute_byte = 0,hour_byte = 0;
	int day1 = 0;
	//unsigned char day = 0;

	//unsigned char prev_sec = 0,prev_hour_byte = 0;

	//hour_byte = ds1307_read_hour();            // read hour regiter and store
	//hour_byte = hour_calculation(hour_byte);// call for mode display
	hr=convert_bcd_to_decimal(ds1307_read_hour());
	//minute_byte = ds1307_read_minute();       // read minute regiter and store
	min=convert_bcd_to_decimal(ds1307_read_minute());
	//second_byte = ds1307_read_second();        // read seconds regiter and store
	sec = convert_bcd_to_decimal(ds1307_read_second());

	day1=convert_bcd_to_decimal(ds1307_read_day());
	date=convert_bcd_to_decimal(ds1307_read_date());
	month=convert_bcd_to_decimal(ds1307_read_month());
	year=convert_bcd_to_decimal(ds1307_read_year());




	//displaying style
	if(date<=9)
					{
						lcd_cursor(1,3);
						lcd_string_write("0");
						lcd_cursor(1,4);
						lcd_number_write(date,10);
					} else {
						lcd_cursor(1,3);
						lcd_number_write(date,10);
					}
	if(month<=9)
				{
					lcd_cursor(1,6);
					lcd_string_write("0");
					lcd_cursor(1,7);
					lcd_number_write(month,10);
				} else {
						lcd_cursor(1,6);
						lcd_number_write(month,10);
					}
	if(year<=9)
					{
						lcd_cursor(1,9);
						lcd_string_write("0");
						lcd_cursor(1,10);
						lcd_number_write(year,10);
					} else {
						lcd_cursor(1,9);
						lcd_number_write(year,10);
					}


	lcd_cursor(1,13);
	//day=weekdays[day1-1];
	//lcd_number_write(day1,10);
	lcd_string_write(weekdays[day1-1]);
	//lcd_data_write(day);

    // displaying style
	if(hr<=9)
					{
						lcd_cursor(2,3);
						lcd_string_write("0");
						lcd_cursor(2,4);
						lcd_number_write(hr,10);
					} else {
						lcd_cursor(2,3);
						lcd_number_write(hr,10);
					}

	if(min<=9)
					{
						lcd_cursor(2,6);
						lcd_string_write("0");
						lcd_cursor(2,7);
						lcd_number_write(min,10);
					} else {
						lcd_cursor(2,6);
						lcd_number_write(min,10);
					}

	if(sec<=9)
					{
						lcd_cursor(2,9);
						lcd_string_write("0");
						lcd_cursor(2,10);
						lcd_number_write(sec,10);
					} else {
						lcd_cursor(2,9);
						lcd_number_write(sec,10);
					}


}

void display_date_time_format(){
	lcd_cursor(1,5);
	lcd_string_write("/");
	lcd_cursor(1,8);
	lcd_string_write("/");
	lcd_cursor(2,5);
	lcd_string_write(":");
	lcd_cursor(2,8);
	lcd_string_write(":");
}

int menu_option_reset(const int mode_addr)// mode_addr can be MODE0 MODE1 or MODE2
{
	int intervals;

	lcd_command_write(0x01); //clear screen
	lcd_cursor(1,1);

	switch(mode_addr){
		case MODE0:
		case MODE1:
			lcd_string_write("No. of Timings?");
			break;
		case MODE2:
			lcd_string_write("No. of Alarms?");
			break;
	}

	lcd_cursor(2,1);

	while(1)
	{
		intervals = print_minute(2,1,0);
		temp1=PINA;

		if((temp1 & 0x01)!=0x00) // OK button
		{
			lcd_command_write(0x01); //clear screen
			break;
		}

		if((temp1 & 0x02)!=0x00) // EXIT button
		{
			lcd_command_write(0x01); //clear screen
			return 0;
		}

	}
	if(intervals > 0){
	eeprom_write_word((uint16_t *) (mode_addr) , intervals);

	int i;
	for(i=0; i<intervals; i++)
	{
		clock:
		lcd_cursor(1,1);
		lcd_string_write("HRS:min   P");

		switch(mode_addr){
			case MODE0:
				lcd_number_write(0,10);
				break;
			case MODE1:
				lcd_number_write(1,10);
				break;
			case MODE2:
				lcd_number_write(2,10);
				break;
		}

		lcd_string_write(" T");

		lcd_number_write(i+1,10);
		lcd_cursor(2,1);
		lcd_string_write("  :         of  ");
		lcd_cursor(2,11);
		lcd_number_write(i+1,10);
		lcd_cursor(2,15);
		lcd_number_write(intervals,10);



		while(1) // setting hrs
		{
			temp1=PINA;
			hr=print_hour(2,1,0);

			if((temp1 & 0x01)!=0x00) // OK button
			{
				break;
			}

			if((temp1 & 0x02)!=0x00) // EXIT button
			{
				lcd_command_write(0x01); //clear screen
				return 0;
				break;
			}
		}

		lcd_cursor(1,1);
		lcd_string_write("hrs:MIN   P");

				switch(mode_addr){
					case MODE0:
						lcd_number_write(0,10);
						break;
					case MODE1:
						lcd_number_write(1,10);
						break;
					case MODE2:
						lcd_number_write(2,10);
						break;
				}
				lcd_string_write(" T");


		while(1) // setting minutes
		{
			temp1=PINA;
			min=print_minute(2,4,0);

			if((temp1 & 0x01)!=0x00) // OK button
			{
				break;
			}

			if((temp1 & 0x02)!=0x00) // EXIT button
			{
				lcd_command_write(0x01); //clear screen
				goto clock;
				break;
			}
		}
		eeprom_write_word((uint16_t *) (mode_addr+i*2+2) , (hr*100+min));
		lcd_cursor(2,9);
//		lcd_number_write((mode_addr+i*2),10);
//		_delay_ms(100);
		lcd_command_write(0x01); //clear screen
	}
	}
}

void menu_option_run(const int mode_addr){
	int i=0; //restart counter
	int intervals = 0;
	intervals = eeprom_read_word((uint16_t *) mode_addr);	// Load total no. of timings stored in eeprom
	int break_loop = 0;

	lcd_cursor(2,15);
	lcd_string_write("P");

	switch(mode_addr){
		case MODE0:
			lcd_number_write(0, 10);
			break;
		case MODE1:
			lcd_number_write(1, 10);
			break;
		case MODE2:
			lcd_number_write(2, 10);
			break;
	}

	while(1)
	{
		display_time();

		if((hr==(eeprom_read_word((uint16_t *) (mode_addr + i*2+2))/100)) && (min == (eeprom_read_word((uint16_t *) (mode_addr + i*2+2) )%100))){
			//  ring_bell_long(1);
			PORTC = (PC2<<1)|(PC3<<1);	//Pins PC2(buzzer) and PC3(bell1) will go high
			_delay_ms(500);
			PORTC = (PC2<<0)|(PC3<<0);
			//lcd_command_write(0x01);
			i++;
		}
		if(i>intervals)
			break;
		temp1=PINA;

		switch(mode_addr){
			case MODE0:
				break_loop = ((temp1 & 0x08) !=0x00)||((temp1 & 0x10) !=0x00)||((temp1 & 0x20) !=0x00);
				break;
			case MODE1:
				break_loop = ((temp1 & 0x08) !=0x00)||((temp1 & 0x20) !=0x00);
				break;
			case MODE2:
				break_loop = ((temp1 & 0x10) !=0x00)||((temp1 & 0x08) !=0x00);
				break;

		}
		if(break_loop == 1){
			break;
		}
	}
}

int menu_option_mode(const int mode_addr){
	int menu = 0, menu_select = 0, sub_select =0, goto_result=0;
	// NOTE: goto_result is used to tell the calling function to go back to main menu

	lcd_command_write(0x01); //clear screen
	lcd_cursor(1,1);
	lcd_string_write("   P");

	switch(mode_addr){
		case MODE0:
			lcd_number_write(0,10);
			break;
		case MODE1:
			lcd_number_write(1, 10);
			break;
		case MODE2:
			lcd_number_write(2, 10);
			break;
	}

	lcd_string_write("   Clock   ");

	while(1)
	{
		lcd_cursor(2,1);
		menu=ReadVoltage();
		temp1 = PINA;
		if(menu < 30)
		{
			//modify
			lcd_string_write("MODIFY    reset");
			sub_select=1;
		}
		if(menu > 30)
		{
			//reset
			lcd_string_write("modify    RESET");
			sub_select=2;
		}
		if((temp1 & 0x01)!=0x00) // OK button
		{
			break;
		}
		if((temp1 & 0x02)!=0x00) // EXIT button
		{
			lcd_command_write(0x01); //clear screen
			goto_result = 1;
			break;
		}
	}

	while(1)
	{
		if(sub_select==2) //resetting the eeprom
		{
			lcd_command_write(0x01); //clear screen
			menu_option_reset(mode_addr);
			return 1;
		}
		if(sub_select==1) //editting selected timings
		{
			int i=0,i_prev=0;
			int  intervals = eeprom_read_word(mode_addr);
			if(intervals == -1){
				lcd_cursor(1,1);
				lcd_command_write(0x01); //clear screen
				switch(mode_addr){
					case MODE0:
					case MODE1:
						lcd_string_write(" No Timings Set ");
						break;
					case MODE2:
						lcd_string_write(" No Alarms Set ");
						break;
				}
				_delay_ms(50);
				break;
			} else {
				modify:

				lcd_cursor(2,1);
				lcd_string_write("  :         of  ");
				while(1)
					{
					i = (ReadVoltage()*0.090196078);
					if(i<0)
						i=0;
					if(i>(intervals-1))
						i=intervals-1;
					/*
					if(i-i_prev !=0)
					{
						lcd_cursor(2,1);
						lcd_string_write("  :         of  ");
					}
					*/
					lcd_cursor(1,1);
					lcd_string_write("HRS:MIN   P");
					switch(mode_addr){
						case MODE0:
							lcd_number_write(0,10);
							break;
						case MODE1:
							lcd_number_write(1, 10);
							break;
						case MODE2:
							lcd_number_write(2, 10);
							break;
					}
					lcd_string_write(" T");
					lcd_number_write(i+1,10);
					//lcd_cursor(1,16);
					//lcd_string_write(" ");
					if((eeprom_read_word(mode_addr+i*2+2)/100) <= 9)
					{
						lcd_cursor(2,1);
						lcd_string_write("0");
						lcd_number_write(eeprom_read_word(mode_addr+i*2+2)/100,10);
					}
					if((eeprom_read_word(mode_addr+i*2+2)/100) > 9)
					{
						lcd_cursor(2,1);
						lcd_number_write(eeprom_read_word(mode_addr+i*2+2)/100,10);
					}

					if((eeprom_read_word(mode_addr+i*2+2)%100) <= 9)
					{
						lcd_cursor(2,4);
						lcd_string_write("0");
						lcd_number_write(eeprom_read_word(mode_addr+i*2+2)%100,10);
					}
					if(eeprom_read_word(mode_addr+i*2+2)%100 > 9)
					{
						lcd_cursor(2,4);
						lcd_number_write(eeprom_read_word(mode_addr+i*2+2)%100,10);
					}
					//lcd_string_write(eeprom_read_word(mode_addr+i+1));
					//lcd_data_write(eeprom_read_word(mode_addr+i+1));

					/*lcd_cursor(2,4);
					lcd_number_write(eeprom_read_word(mode_addr+i+1)%100,10)	;*/

					if(i+1 <= 9)
						lcd_cursor(2,11);
					if(i+1 > 9)
						lcd_cursor(2,10);
					lcd_number_write(i+1,10);
					lcd_cursor(2,15);
					lcd_number_write(intervals,10);
					temp1=PINA;
					if((temp1 & 0x01)!=0x00) // OK button
					{
						//edit the data
						lcd_cursor(2,1);
						lcd_string_write("  :             ");
						set:
						while(1) // setting hrs
						{
							temp1=PINA;
							hr=print_hour(2,1,0);
							if((temp1 & 0x01)!=0x00) // OK button
							{
								break;
							}
							if((temp1 & 0x02)!=0x00) // EXIT button
							{
								lcd_command_write(0x01); //clear screen
								goto modify;
								break;
							}
						}

						while(1) // setting minutes
						{
							temp1=PINA;
							min=print_minute(2,4,0);
							if((temp1 & 0x01)!=0x00) // OK button
							{
								break;
							}
							if((temp1 & 0x02)!=0x00) // EXIT button
							{
								//lcd_command_write(0x01); //clear screen
								goto set;
								break;
							}
						}
						eeprom_write_word((uint16_t *) (mode_addr+i*2+2) , (hr*100+min));
						lcd_cursor(2,1);
						lcd_string_write("                ");
					}
					if((temp1 & 0x02)!=0x00) // EXIT button
					{
						lcd_command_write(0x01); //clear screen
						return 1;
					}
					i_prev=i;


					}
			}
		}

		//ending codes to go back to menu
		menu_select = 0;
		sub_select=0;
		return 1;
	}

}

int main(){
	// variables used in the code
	//int hr = 0, min = 0, min_prev = 0, sec = 0, day = 0, date = 0, month = 0,
		int yr =0;
		int menu = 0, menu_select = 0, sub_select =0;
		//int a = 0,

		int r = 0, d = 0;

		//int pause=0;
		//int sw1, sw2, h;
		//int s;
		int leap[12]={-1,2,3,6,1,4,6,2,5,0,3,5};
		int nonleap[12]={0,3,3,6,1,4,6,2,5,0,3,5};


		adc_init();
	DDRD=0xff;
	/*All the 8 pins of PortD are declared output (data pins of LCD are connected)*/

	DDRB=0x07;
	/*PB0, PB1 and PB2 pins of PortB are declared output (control pins of LCD are connected)*/
	DDRC=DDRC | 0xfc;
	//PC2 and PC3 set as output for relay/buzzer

	//switches and inputs
	DDRA = DDRA & 0x00;   //PORTE pin set as input
	PORTA = PORTA | 0xFF; //PORTE internal pull-up enabled
	ADCSRA=0X85; //adc config

	//unsigned char sec=0,min=0;
	/*Variable declaration*/

	twi_init();
	/*TWI initialisaiton*/

	lcd_init();
	/*LCD initialisaiton*/


	/*String display in 1st row of LCD*/
	lcd_cursor(1,1);
	lcd_string_write("-----EXMPLR-----");
	lcd_command_write(0xc0);
	/*Cursor moves to 2nd row 1st column of LCD*/

	//Position the LCD cursor at "row", "column"


	lcd_cursor(2,1);
	lcd_string_write("  AUTOCLK  3.0  ");
	/*String display in 2nd row of LCD*/

	_delay_ms(100);
	/*Display stays for 2 second*/
	lcd_command_write(0x01);
		/*Clear screen*/

	while(1)
	{
		main:
		temp1 = PINA;

		//battery low

		/*
		date=convert_bcd_to_decimal(ds1307_read_date());
		if(date==0)
		{
			while(1)
			{
				temp1 = PINA;
				lcd_cursor(1,1);
				lcd_string_write("Check 3V Battery");
				lcd_cursor(2,1);
				lcd_string_write("        Press OK");
				if((temp1 & 0x01)!=0x00) // OK button
				{ 	lcd_command_write(0x01);
					goto master_clock;
				}
			}
		}
		*/
	//read the Port L

			//set mode: used to set the time HRS:MIN:SEC of RTC timer
			if((temp1 & 0x08) != 0x00) // MENU button
			{
				// choose menu
				lcd_command_write(0x01); //clear screen

				//menu
				menu:
				lcd_cursor(1,1);
				lcd_string_write("MENU:           ");

				while(1)
				{
					temp1 = PINA;
					menu = ReadVoltage();
					lcd_cursor(2,1);
					if(menu > 0 && menu <= 30)
					{
						// set master time
						lcd_string_write("| Master Clock >");
						menu_select=1;
						//goto master_clock;
					}
					if(menu > 30 && menu <= 60)
					{
					// set P0 time
						lcd_string_write("<  P0   Clock  >");
						menu_select=2;
						//goto P0_clock;
					}
					if(menu > 60 && menu <= 90)
					{
					// set P1 time
						lcd_string_write("<  P1   Clock  >");
						menu_select=3;
					}
					if(menu > 90 && menu <= 255)
					{
					// set P2 time
					    lcd_string_write("<  P2   Clock  |");
						menu_select=4;
					}
					if((temp1 & 0x02)!=0x00) // EXIT button
					{
						lcd_command_write(0x01); //clear screen
						menu_select=0;
						goto main;
					}

					if((temp1 & 0x01)!=0x00) // OK button
					{
						//function into mode as per menu_select
						lcd_command_write(0x01); //clear screen
						lcd_cursor(2,1);


						while(1)
						{
							//set master clock

							if(menu_select == 1)
							{
								master_clock:
								//_continue=0;
								lcd_cursor(1,1);
								lcd_string_write("Set HRS:min:sec");
								lcd_cursor(2,1);
								lcd_string_write("  :  :          ");

								while(1) // setting hrs
								{
									temp1=PINA;
									hr=print_hour(2,1,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{
										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto menu;
										break;
									}
								}

								lcd_cursor(1,1);
								lcd_string_write("Set hrs:MIN:sec");

								while(1) // setting minutes
								{
									temp1=PINA;
									min=print_minute(2,4,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{

										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto master_clock;
										break;
									}
								}

								lcd_cursor(1,1);
								lcd_string_write("Set hrs:min:SEC");



								while(1) // setting seconds
								{
									temp1=PINA;
									sec=print_minute(2,7,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{

										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto master_clock;
										break;
									}
								}
								ds1307_hour_write(hr,0,0);
								ds1307_minute_write(min);
								ds1307_second_write(sec);

								date:

								lcd_cursor(1,1);
								lcd_string_write("Set DD:mm:yy     ");
								lcd_cursor(2,1);
								lcd_string_write("  :  :          ");

								while(1) //setting date
								{
									temp1=PINA;
									date = print_date(2,1,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{
										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto master_clock;
										break;
									}

								}
								if((temp1 & 0x02)!=0x00) // EXIT button
								{
									lcd_command_write(0x01); //clear screen
									goto menu;
									break;
								}
								lcd_cursor(1,1);
								lcd_string_write("Set dd:MM:yy      ");

								while(1) //setting month
								{
									temp1=PINA;
									month = print_month(2,4,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{
										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto date;
										break;
									}

								}

								lcd_cursor(1,1);
								lcd_string_write("Set dd:mm:YY");

								while(1) //setting year
								{
									temp1=PINA;
									yr = print_yr(2,7,0);
									if((temp1 & 0x01)!=0x00) // OK button
									{
										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto date;
										break;
									}

								}
								ds1307_date_write(date);
								ds1307_month_write(month);
								ds1307_year_write(yr);
								if(yr%4 == 0)
								{
									d=date+leap[month-1]+yr+(yr/4)+6;
								} else
								{
									d=date+nonleap[month-1]+yr+(yr/4)+6;
								}

								r=d%7 + 1;

								ds1307_day_write(r);


								//ending codes to go back to menu
								menu_select=0;
								goto menu; //all values set, goto menu


							}

							//set P0
							if(menu_select==2){
								if(menu_option_mode(MODE0) == 1)
									goto menu;
							}
							/*{

								lcd_command_write(0x01); //clear screen
								lcd_cursor(1,1);
								lcd_string_write("   P0   Clock   ");
								while(1)
								{
									lcd_cursor(2,1);
									menu=ReadVoltage();
									temp1 = PINA;
									if(menu < 30)
									{
										//modify
										lcd_string_write("MODIFY    reset");
										sub_select=1;
									}
									if(menu > 30)
									{
										//reset
										lcd_string_write("modify    RESET");
										sub_select=2;
									}
									if((temp1 & 0x01)!=0x00) // OK button
									{
										break;
									}
									if((temp1 & 0x02)!=0x00) // EXIT button
									{
										lcd_command_write(0x01); //clear screen
										goto menu;
										break;
									}
								}

								while(1)
								{
									if(sub_select==2) //resetting the eeprom
									{
										lcd_command_write(0x01); //clear screen
										menu_option_reset(MODE0);
										goto menu;
										break;
									}
									if(sub_select==1) //editting selected timings
									{
										int i=0,i_prev=0;
										int  intervals = eeprom_read_word(MODE0);
										if(intervals == -1){
											lcd_cursor(1,1);
											lcd_command_write(0x01); //clear screen
											lcd_string_write(" No Timings Set ");
											_delay_ms(50);
										} else {
									P0_modify:

										lcd_cursor(2,1);
										lcd_string_write("  :         of  ");
										while(1)
											{
											i = (ReadVoltage()*0.090196078);
											if(i<0)
												i=0;
											if(i>(intervals-1))
												i=intervals-1;

											if(i-i_prev !=0)
											{
												lcd_cursor(2,1);
												lcd_string_write("  :         of  ");
											}

											lcd_cursor(1,1);
											lcd_string_write("HRS:MIN   P0 T");
											lcd_number_write(i+1,10);
											//lcd_cursor(1,16);
											//lcd_string_write(" ");
											if((eeprom_read_word(MODE0+i*2+2)/100) <= 9)
											{
												lcd_cursor(2,1);
												lcd_string_write("0");
												lcd_number_write(eeprom_read_word(MODE0+i*2+2)/100,10);
											}
											if((eeprom_read_word(MODE0+i*2+2)/100) > 9)
											{
												lcd_cursor(2,1);
												lcd_number_write(eeprom_read_word(MODE0+i*2+2)/100,10);
											}

											if((eeprom_read_word(MODE0+i*2+2)%100) <= 9)
											{
												lcd_cursor(2,4);
												lcd_string_write("0");
												lcd_number_write(eeprom_read_word(MODE0+i*2+2)%100,10);
											}
											if(eeprom_read_word(MODE0+i*2+2)%100 > 9)
											{
												lcd_cursor(2,4);
												lcd_number_write(eeprom_read_word(MODE0+i*2+2)%100,10);
											}
											//lcd_string_write(eeprom_read_word(MODE0+i+1));
											//lcd_data_write(eeprom_read_word(MODE0+i+1));

											lcd_cursor(2,4);
											lcd_number_write(eeprom_read_word(MODE0+i+1)%100,10)	;

											if(i+1 <= 9)
												lcd_cursor(2,11);
											if(i+1 > 9)
												lcd_cursor(2,10);
											lcd_number_write(i+1,10);
											lcd_cursor(2,15);
											lcd_number_write(intervals,10);
											temp1=PINA;
											if((temp1 & 0x01)!=0x00) // OK button
											{
												//edit the data
												lcd_cursor(2,1);
												lcd_string_write("  :             ");
												P0_set:
												while(1) // setting hrs
												{
													temp1=PINA;
													hr=print_hour(2,1,0);
													if((temp1 & 0x01)!=0x00) // OK button
													{
														break;
													}
													if((temp1 & 0x02)!=0x00) // EXIT button
													{
														lcd_command_write(0x01); //clear screen
														goto P0_modify;
														break;
													}
												}

												while(1) // setting minutes
												{
													temp1=PINA;
													min=print_minute(2,4,0);
													if((temp1 & 0x01)!=0x00) // OK button
													{
														break;
													}
													if((temp1 & 0x02)!=0x00) // EXIT button
													{
														//lcd_command_write(0x01); //clear screen
														goto P0_set;
														break;
													}
												}
												eeprom_write_word((uint16_t *) (MODE0+i*2+2) , (hr*100+min));
												lcd_cursor(2,1);
												lcd_string_write("                ");
											}
											if((temp1 & 0x02)!=0x00) // EXIT button
											{
												lcd_command_write(0x01); //clear screen
												goto menu;
												break;
											}
											i_prev=i;


											}
									}
								}

								//ending codes to go back to menu
								menu_select = 0;
								sub_select=0;
								goto menu;
							}


						}*/
						//P1 MODE
							//set P0
						if(menu_select==3){
							if(menu_option_mode(MODE1) == 1)
								goto menu;
						}
						/*{

							lcd_command_write(0x01); //clear screen
							lcd_cursor(1,1);
							lcd_string_write("   P1   Clock   ");
							while(1)
							{
								lcd_cursor(2,1);
								menu=ReadVoltage();
								temp1 = PINA;
								if(menu < 30)
								{
									//modify
									lcd_string_write("MODIFY    reset");
									sub_select=1;
								}
								if(menu > 30)
								{
									//reset
									lcd_string_write("modify    RESET");
									sub_select=2;
								}
								if((temp1 & 0x01)!=0x00) // OK button
								{
									break;
								}
								if((temp1 & 0x02)!=0x00) // EXIT button
								{
									lcd_command_write(0x01); //clear screen
									goto menu;
									break;
								}
							}

							while(1)
							{
								if(sub_select==2) //resetting the eeprom
								{
									lcd_command_write(0x01); //clear screen
									menu_option_reset(MODE1);
									goto menu;
									break;
								}

								if(sub_select==1) //editting selected timings
								{
									int i=0,i_prev=0;
									int  intervals = eeprom_read_word(MODE1);
									if(intervals == -1){
										lcd_cursor(1,1);
										lcd_command_write(0x01); //clear screen
										lcd_string_write(" No Timings Set ");
										_delay_ms(50);
									} else {
								P1_modify:

									lcd_cursor(2,1);
									lcd_string_write("  :         of  ");
									while(1)
										{
										i = (ReadVoltage()*0.090196078);
										if(i<0)
											i=0;
										if(i>(intervals-1))
											i=intervals-1;

										if(i-i_prev !=0)
										{
											lcd_cursor(2,1);
											lcd_string_write("  :         of  ");
										}

										lcd_cursor(1,1);
										lcd_string_write("HRS:MIN   P1 T");
										lcd_number_write(i+1,10);
										lcd_cursor(1,16);
										lcd_string_write(" ");
										if(eeprom_read_word(MODE1+i*2+2)/100 <= 9)
										{
											lcd_cursor(2,1);
											lcd_string_write("0");
											lcd_number_write(eeprom_read_word(MODE1+i*2+2)/100,10);
										}
										if(eeprom_read_word(MODE1+i*2+2)/100 > 9)
										{
											lcd_cursor(2,1);
											lcd_number_write(eeprom_read_word(MODE1+i*2+2)/100,10);
										}

										if(eeprom_read_word(MODE1+i*2+2)%100 <= 9)
										{
											lcd_cursor(2,4);
											lcd_string_write("0");
											lcd_number_write(eeprom_read_word(MODE1+i*2+2)%100,10);
										}
										if(eeprom_read_word(MODE1+i*2+2)%100 > 9)
										{
											lcd_cursor(2,4);
											lcd_number_write(eeprom_read_word(MODE1+i*2+2)%100,10);
										}
										//lcd_string_write(eeprom_read_word(MODE0+i+1));
										//lcd_data_write(eeprom_read_word(MODE0+i+1));

										lcd_cursor(2,4);
										lcd_number_write(eeprom_read_word(MODE0+i+1)%100,10)	;

										if(i+1 <= 9)
											lcd_cursor(2,11);
										if(i+1 > 9)
											lcd_cursor(2,10);
										lcd_number_write(i+1,10);
										lcd_cursor(2,15);
										lcd_number_write(intervals,10);
										temp1=PINA;
										if((temp1 & 0x01)!=0x00) // OK button
										{
											//edit the data
											lcd_cursor(2,1);
											lcd_string_write("  :             ");
											P1_set:
											while(1) // setting hrs
											{
												temp1=PINA;
												hr=print_hour(2,1,0);
												if((temp1 & 0x01)!=0x00) // OK button
												{
													break;
												}
												if((temp1 & 0x02)!=0x00) // EXIT button
												{
													lcd_command_write(0x01); //clear screen
													goto P1_modify;
													break;
												}
											}

											while(1) // setting minutes
											{
												temp1=PINA;
												min=print_minute(2,4,0);
												if((temp1 & 0x01)!=0x00) // OK button
												{

													break;
												}
												if((temp1 & 0x02)!=0x00) // EXIT button
												{
													//lcd_command_write(0x01); //clear screen
													goto P1_set;
													break;
												}
											}
											eeprom_write_word((uint16_t *) (MODE1+i*2+2) , (hr*100+min));
											lcd_cursor(2,1);
											lcd_string_write("                ");
										}
										if((temp1 & 0x02)!=0x00) // EXIT button
										{
											lcd_command_write(0x01); //clear screen
											goto menu;
											break;
										}
										i_prev=i;


										}
								}
							}

							//ending codes to go back to menu
							menu_select = 0;
							sub_select=0;
							goto menu;
						}
						}*/
						//P2 MODE
							//set P2
						if(menu_select==4){
							if(menu_option_mode(MODE2) == 1)
								goto menu;
						}
						/*{

							lcd_command_write(0x01); //clear screen
							lcd_cursor(1,1);
							lcd_string_write("   P2   Clock   ");
							while(1)
							{
								lcd_cursor(2,1);
								menu=ReadVoltage();
								temp1 = PINA;
								if(menu < 30)
								{
									//modify
									lcd_string_write("MODIFY    reset");
									sub_select=1;
								}
								if(menu > 30)
								{
									//reset
									lcd_string_write("modify    RESET");
									sub_select=2;
								}
								if((temp1 & 0x01)!=0x00) // OK button
								{
									break;
								}
								if((temp1 & 0x02)!=0x00) // EXIT button
								{
									lcd_command_write(0x01); //clear screen
									goto menu;
									break;
								}
							}

							while(1)
							{
								if(sub_select==2) //resetting the eeprom
								{
									lcd_command_write(0x01); //clear screen
									menu_option_reset(MODE2);
									goto menu;
									break;
								}
								if(sub_select==1) //editting selected timings
								{
									int i=0,i_prev=0;
									int  intervals = eeprom_read_word(MODE2);
									if(intervals == -1){
										lcd_cursor(1,1);
										lcd_command_write(0x01); //clear screen
										lcd_string_write(" No Alarms Set ");
										_delay_ms(50);
									} else {
								P2_modify:

									lcd_cursor(2,1);
									lcd_string_write("  :         of  ");
									while(1)
										{
										i = (ReadVoltage()*0.090196078);
										if(i<0)
											i=0;
										if(i>(intervals-1))
											i=intervals-1;

										if(i-i_prev !=0)
										{
										lcd_cursor(2,1);
										lcd_string_write("  :         of  ");
										}

										lcd_cursor(1,1);
										lcd_string_write("HRS:MIN   P2 T");
										lcd_number_write(i+1,10);
										lcd_cursor(1,16);
										lcd_string_write(" ");
										if(eeprom_read_word(MODE2+i*2+2)/100 <= 9)
										{
											lcd_cursor(2,1);
											lcd_string_write("0");
											lcd_number_write(eeprom_read_word(MODE2+i*2+2)/100,10);
										}
										if(eeprom_read_word(MODE2+i*2+2)/100 > 9)
										{
											lcd_cursor(2,1);
											lcd_number_write(eeprom_read_word(MODE2+i*2+2)/100,10);
										}

										if(eeprom_read_word(MODE2+i*2+2)%100 <= 9)
										{
											lcd_cursor(2,4);
											lcd_string_write("0");
											lcd_number_write(eeprom_read_word(MODE2+i*2+2)%100,10);
										}
										if(eeprom_read_word(MODE2+i*2+2)%100 > 9)
										{
											lcd_cursor(2,4);
											lcd_number_write(eeprom_read_word(MODE2+i*2+2)%100,10);
										}

										//lcd_string_write(eeprom_read_word(MODE0+i+1));
										//lcd_data_write(eeprom_read_word(MODE0+i+1));

										lcd_cursor(2,4);
										lcd_number_write(eeprom_read_word(MODE0+i+1)%100,10)	;

										if(i+1 <= 9)
											lcd_cursor(2,11);
										if(i+1 > 9)
											lcd_cursor(2,10);
										lcd_number_write(i+1,10);
										lcd_cursor(2,15);
										lcd_number_write(intervals,10);
										temp1=PINA;
										if((temp1 & 0x01)!=0x00) // OK button
										{
											//edit the data
											lcd_cursor(2,1);
											lcd_string_write("  :             ");
											P2_set:
											while(1) // setting hrs
											{
												temp1=PINA;
												hr=print_hour(2,1,0);
												if((temp1 & 0x01)!=0x00) // OK button
												{
													break;
												}
												if((temp1 & 0x02)!=0x00) // EXIT button
												{
													lcd_command_write(0x01); //clear screen
													goto P2_modify;
													break;
												}
											}

											while(1) // setting minutes
											{
												temp1=PINA;
												min=print_minute(2,4,0);
												if((temp1 & 0x01)!=0x00) // OK button
												{

													break;
												}
												if((temp1 & 0x02)!=0x00) // EXIT button
												{
													//lcd_command_write(0x01); //clear screen
													goto P2_set;
													break;
												}
											}
											eeprom_write_word((uint16_t *) (MODE2+i*2+2) , (hr*100+min));
											lcd_cursor(2,1);
											lcd_string_write("                ");
										}
										if((temp1 & 0x02)!=0x00) // EXIT button
										{
											lcd_command_write(0x01); //clear screen
											goto menu;
											break;
										}
										i_prev=i;


										}
								}
							}

							//ending codes to go back to menu
							menu_select = 0;
							sub_select=0;
							goto menu;
						}
						}*/

						lcd_command_write(0x01); //clear screen
						break;
					}

					if((temp1 & 0x02)!=0x00) // EXIT button
					{
						lcd_command_write(0x01); //clear screen
						break;
					}

				}


			}
			}
			// else { // RUN MODE: either P0(normal routine); P1(exam routine); P2/Intervals?(custom routine
				display_date_time_format();	// draw / / for date and : : for time on LCD
				if((temp1 & 0x10)!=0x00)
				{
					menu_option_run(MODE1);
				}
				if((temp1 & 0x20)!=0x00)
				{
					menu_option_run(MODE2);
				}
				if(((temp1 & 0x10)==0x00) && ((temp1 & 0x20)==0x00))
				{
					menu_option_run(MODE0);
				}
				//bell_H(h);
				//bell_S(s);
		//	}

	} //while loop

	return 0;
}//main function

//TODO Extract code out of main function to create a new function for modify feature,
// function name would be menu_option_modify()

//TODO add feature to set different bell durations

//TODO add fail safe features such as battery damaged, prompt to set clock timing,

