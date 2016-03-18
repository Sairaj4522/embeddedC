/*The functions and macros defined in this header file are for TWI(Two Wire Interface) Protocol of all 8-bit AVR Micricontrollers with Similar Register Configuration 
with ATmega 16*/

#ifndef _TWI_H_
#define _TWI_H_		1
     


#include<avr/io.h>



/*The function is declared to initialize the TWI at 100 KHz frequency.*/

void twi_init();



/*The function is declared to set the microcontroller as master and to send the START pulse to the TWI slave.*/

unsigned char twi_send_start();



/*The function is declared to send the RESTART pulse to the TWI slave by the TWI master.*/

unsigned char twi_send_repeated_start(void);


/*The function is declared to send the 7 bit slave address and 1 bit read/write bit by TWI master.*/

unsigned char twi_send_address_rw(unsigned char address_rw);



/*The function is declared to send data to the TWI slave by the TWI master.*/

unsigned char twi_master_send_data(unsigned char data);



/*The function is declared to send string to the TWI slave by the TWI master.*/

unsigned char twi_master_send_string(unsigned char *string);



/*The function is declared to receive data sent by TWI slave to TWI master.*/

unsigned char twi_master_receive_data();



/*The function is declared to receive string sent by TWI slave to TWI master.*/

unsigned char *twi_master_receive_string(unsigned char number_of_bytes,unsigned char *string);



/*The function is declared to receive the last data sent by TWI slave to TWI master.*/

unsigned char twi_master_receive_last_data();



/*The function is declared to send the STOP pulse to the TWI slave by the TWI master.*/

void twi_send_stop();



/*Function definations*/

void twi_init()
{
TWSR=0x00;
TWBR=0x02;
}

unsigned char twi_send_start()
{
TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 	
while (!(TWCR & (1<<TWINT)))
	;
return (TWSR & 0xf8);
while((TWSR & 0xf8) != 0x08)
	;
}

unsigned char twi_send_repeated_start(void)
{
TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 		
while (!(TWCR & (1<<TWINT)))
	;   		
return (TWSR & 0xf8);
while((TWSR & 0xf8) != 0x10)
	;   		  								
}

unsigned char twi_send_address_rw(unsigned char address_rw)
{
TWDR = address_rw; 
TWCR = (1<<TWINT)|(1<<TWEN);	   
while (!(TWCR & (1<<TWINT)))
	;
return (TWSR & 0xf8);
while(((TWSR & 0xf8) != 0x18) || ((TWSR & 0xf8) != 0x40))
	;		   
}

unsigned char twi_master_send_data(unsigned char data)
{
TWDR = data; 
TWCR = (1<<TWINT) |(1<<TWEN);	   
while (!(TWCR & (1<<TWINT)))
;	   
return (TWSR & 0xf8);
while((TWSR & 0xf8) != 0x28)
	;   		 		   					   
}

unsigned char twi_master_send_string(unsigned char *string)
{
while (*string)
{
	TWDR = *string++; 
	TWCR = (1<<TWINT) |(1<<TWEN);	   
	while (!(TWCR & (1<<TWINT)))
	;	   
	while((TWSR & 0xf8) != 0x28)
		;   		 		   					   
}
return (TWSR & 0xf8);
}

unsigned char twi_master_receive_data()
{
TWCR =(1<<TWEA)|(1<<TWINT)|(1<<TWEN);
while (!(TWCR & (1<<TWINT)))
	;	   	   
return(TWDR);
while((TWSR & 0xf8) != 0x50)
	; 
}

unsigned char *twi_master_receive_string(unsigned char number_of_bytes,unsigned char *string)
{
for(unsigned char i=0;i<number_of_bytes;i++)
{
	TWCR =(1<<TWEA)|(1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)))
		;	   	   
	*(string+i)=TWDR;
	while((TWSR & 0xf8) != 0x50)
		; 
}
*(string+number_of_bytes)=twi_master_receive_last_data();
*(string+number_of_bytes)=0;
}

unsigned char twi_master_receive_last_data()
{
TWCR =(1<<TWINT)|(1<<TWEN);
while (!(TWCR & (1<<TWINT)))
	;	   	   
return(TWDR);
while((TWSR & 0xf8) != 0x58)
	; 
}

void twi_send_stop()
{
TWCR =  (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
while ((TWCR & (1<<TWSTO)))
	;  	 
} 					                        
#endif
