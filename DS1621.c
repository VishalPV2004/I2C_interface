// Reading temperature values from sensor

#include<reg51.h>

// I2C functions
void start_i2c();
void stop_i2c();
unsigned char write_i2c(unsigned char c);
unsigned char read_i2c(unsigned char c);
void ack();
void nack();

// LCD functions
void lcd_init();
void display_str(unsigned char *str);
void lcd_clear();
void lcd_data(unsigned char Data);
void lcd_cmd(unsigned char Data);
void delay(unsigned int);


sbit rs  = P3^0;
sbit en  = P3^1;
sbit sda = P1^1;
sbit scl = P1^0;
sfr lcd_port = 0xA0;

unsigned char x;
unsigned char y;
unsigned char z;

void main()
{
    unsigned char temp;

    lcd_init();

    // Start temperature conversion
    start_i2c();
    write_i2c(0x90); // Address + write
    write_i2c(0xEE); // Start conversion command
    stop_i2c();

    delay(500); // Wait for conversion

    while (1)
    {
        // Point to temperature register
        start_i2c();
        write_i2c(0x90); // Address + write
        write_i2c(0xAA); // Read temperature command
        start_i2c();
        write_i2c(0x91); // Address + read
        temp = read_i2c(1); // Read one byte (integer temp)
        stop_i2c();

        // Display on LCD
        lcd_cmd(0x80);
        lcd_cmd(0x01); // Clear LCD
        display_str("Value : ");
        lcd_cmd(0x88);
        lcd_data((temp / 10) + '0'); // Tens place
        lcd_data((temp % 10) + '0'); // Ones place

        delay(1000); // 1s delay before next read
    }
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

void start_i2c()
{
	sda = 1;
	scl = 1;
	delay(5);
	sda = 0;
	delay(5);
	scl = 0;
}

void stop_i2c()
{
	sda = 0;
	delay(5);
	scl = 1;
	delay(5);
	sda = 1;
}

unsigned char write_i2c(unsigned char c)
{
	unsigned char i, ack;
	for(i = 0 ; i < 8 ; i++)
	{
		sda = (0x80 & c) ? 1 : 0;
		scl = 1;
		delay(5);
		scl = 0;
		c = c << 1;
	}
	
	sda = 1;
	scl = 1;
	delay(5);
	ack = sda;
	scl = 0;
	delay(5);
	return ack;
}

unsigned char read_i2c(unsigned char c)
{
	unsigned char Data = 0, i;
	sda = 1;
	for(i = 0 ; i < 8 ; i++)
	{
		scl = 1;
		delay(5);
		Data = Data << 1;
		Data = Data | sda;
		scl = 0;
		delay(5);
	}
	
	if (c)
	  nack();
	else
		ack();
	
	return Data;
	
}

void nack()
{
	sda = 1;
	scl = 1;
	delay(5);
	scl = 0;
}

void ack()
{
	sda = 0;
	scl = 1;
	delay(5);
	scl = 0;
}

