/*The functions and macros defined in this header file are for DS1307 Real Time Clock at 200 KHz TWI frequency.*/

#ifndef _DS1307_H_
#define _DS1307_H_	1



#include <avr/io.h>
#include "twi.h"



/*This function is declared to write the second value in DS1307.*/

void ds1307_second_write(unsigned char second);



/*This function is declared to write the minute value in DS1307.*/

void ds1307_minute_write(unsigned char minute);



/*This function is declared to write the hour value with 12/24 hour format and AM/PM value if 12 hour format is selected in DS1307. 
0 - AM
1 - PM

0 - 24 hour format
1 - 12 hour format
*/

void ds1307_hour_write(unsigned char hour,unsigned char hour_format, unsigned char am_pm);



/*This function is declared to write the day value in DS1307. Day value varies from 1 to 7.*/

void ds1307_day_write(unsigned char day);



/*This function is declared to write the date value in DS1307.*/

void ds1307_date_write(unsigned char date);



/*This function is declared to write the month value in DS1307.*/

void ds1307_month_write(unsigned char month);



/*This function is declared to write the year value in DS1307.*/

void ds1307_year_write(unsigned char year);



/*This function is declared to read the second value of DS1307.*/

unsigned char ds1307_read_second();



/*This function is declared to read the minute value of DS1307.*/

unsigned char ds1307_read_minute();



/*This function is declared to read the hour value with 12/24 hour format and AM/PM value of DS1307.*/

unsigned char ds1307_read_hour();



/*This function is declared to read the day value of DS1307.*/

unsigned char ds1307_read_day();



/*This function is declared to read the date value of DS1307.*/

unsigned char ds1307_read_date();



/*This function is declared to read the month value of DS1307.*/

unsigned char ds1307_read_month();



/*This function is declared to read the year value of DS1307.*/

unsigned char ds1307_read_year();



/*This function is declared to convert the decimal numbers to BCD numbers.*/

unsigned char convert_decimal_to_bcd(unsigned char decimal_number);



/*This function is declared to convert the BCD numbers to decimal numbers.*/

unsigned char convert_bcd_to_decimal(unsigned char bcd_number);



/*Function definations*/
void ds1307_second_write(unsigned char second)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x00);
second=convert_decimal_to_bcd(second);
twi_status=twi_master_send_data(second);
twi_send_stop();
}

void ds1307_minute_write(unsigned char minute)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x01);
minute=convert_decimal_to_bcd(minute);
twi_status=twi_master_send_data(minute);
twi_send_stop();
}

void ds1307_hour_write(unsigned char hour,unsigned char hour_format, unsigned char am_pm)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x02);
if(hour_format==1)
{
	hour=convert_decimal_to_bcd(hour);
	hour=hour | (1<<6) | (am_pm<<5);
	twi_status=twi_master_send_data(hour);
}
else
{
	hour=convert_decimal_to_bcd(hour);
	twi_status=twi_master_send_data(hour);
}
twi_send_stop();
}

void ds1307_day_write(unsigned char day)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x03);
day=convert_decimal_to_bcd(day);
twi_status=twi_master_send_data(day);
twi_send_stop();
}

void ds1307_date_write(unsigned char date)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x04);
date=convert_decimal_to_bcd(date);
twi_status=twi_master_send_data(date);
twi_send_stop();
}

void ds1307_month_write(unsigned char month)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x05);
month=convert_decimal_to_bcd(month);
twi_status=twi_master_send_data(month);
twi_send_stop();
}

void ds1307_year_write(unsigned char year)
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x06);
year=convert_decimal_to_bcd(year);
twi_status=twi_master_send_data(year);
twi_send_stop();
}

unsigned char ds1307_read_second()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x00);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_minute()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x01);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_hour()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x02);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_day()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x03);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_date()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x04);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_month()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x05);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char ds1307_read_year()
{
unsigned char twi_status;
twi_send_start();
twi_status=twi_send_address_rw(0xd0);
twi_status=twi_master_send_data(0x06);
twi_send_repeated_start();
twi_status=twi_send_address_rw(0xd1);
twi_status=twi_master_receive_last_data();
return twi_status;
twi_send_stop();
}

unsigned char convert_decimal_to_bcd(unsigned char decimal_number)
{
decimal_number=((decimal_number/10)*16)+(decimal_number%10);
return decimal_number;
}

unsigned char convert_bcd_to_decimal(unsigned char bcd_number)
{
bcd_number=((bcd_number>>4)*10)+(bcd_number & 0x0f);
return bcd_number;
}  
#endif
