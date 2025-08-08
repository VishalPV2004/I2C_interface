#include <reg51.h>

sbit rs  = P3^0;
sbit en  = P3^1;
sbit sda = P1^1;
sbit scl = P1^0;
sfr lcd_port = 0xA0;

// Function declarations
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_init();
void lcd_clear();
void delay(unsigned int);
void display_str(unsigned char *);

void start_i2c();
void stop_i2c();
void write_i2c(unsigned char Data);
unsigned char read_i2c();
void i2c_ack();
void i2c_nack();

unsigned char x;

void main()
{
	lcd_init();
	lcd_data('o');

	start_i2c();
	write_i2c(0xA0);   // Device address + write bit
	write_i2c(0x00);   // Memory/register address
	write_i2c('o');    // Data to be written
	stop_i2c();

	start_i2c();
	write_i2c(0xA0);
	write_i2c(0x00);
	start_i2c();
	write_i2c(0xA1);
	x = read_i2c();
	stop_i2c();
	
	lcd_cmd(0x80);
	lcd_clear();
	display_str("I2C Data : ");
	lcd_data(x);
	
	while(1);
}

void delay(unsigned int ms)
{
	unsigned int i, j;
	for(i = 0; i < ms; i++)
		for(j = 0; j < 1275; j++);
}

void lcd_init()
{
	lcd_cmd(0x38);   // Function set
	lcd_cmd(0x0E);   // Display ON, cursor ON
	lcd_cmd(0x01);   // Clear display
	lcd_cmd(0x06);   // Entry mode
	lcd_cmd(0x80);   // Set DDRAM to 1st line
}

void lcd_cmd(unsigned char Data)
{
	rs = 0;
	lcd_port = Data;
	en = 1;
	delay(2);
	en = 0;
	delay(2);
}

void lcd_data(unsigned char Data)
{
	rs = 1;
	lcd_port = Data;
	en = 1;
	delay(2);
	en = 0;
	delay(2);
}

void lcd_clear()
{
	lcd_cmd(0x01);
	lcd_cmd(0x02);
}

void display_str(unsigned char *str)
{
	while(*str)
		lcd_data(*str++);
}

// I2C Functions

void start_i2c()
{
	sda = 1;
	scl = 1;
	delay(2);
	sda = 0;  // START condition
	delay(2);
	scl = 0;
}

void stop_i2c()
{
	sda = 0;
	scl = 1;
	delay(2);
	sda = 1;  // STOP condition
	delay(2);
}

void write_i2c(unsigned char Data)
{
	unsigned char i;
	for(i = 0; i < 8; i++)
	{
		sda = (Data & 0x80) ? 1 : 0; // MSB first
		scl = 1;
		delay(2);
		scl = 0;
		Data <<= 1;
	}
	// ACK bit (from slave)
	sda = 1; // Release SDA
	scl = 1;
	delay(2);
	scl = 0;
}

unsigned char read_i2c()
{
	unsigned char i, Data = 0;
	sda = 1; // Release SDA for input

	for(i = 0; i < 8; i++)
	{
		scl = 1;
		delay(2);
		Data = (Data << 1) | sda;
		scl = 0;
		delay(2);
	}
	
	i2c_nack();  // Master sends NACK
	return Data;
}

void i2c_ack()
{
	sda = 0;
	scl = 1;
	delay(2);
	scl = 0;
}

void i2c_nack()
{
	sda = 1;
	scl = 1;
	delay(2);
	scl = 0;
}
