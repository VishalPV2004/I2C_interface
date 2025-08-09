//single master multple slave

#include <reg51.h>

sbit rs  = P3^0;
sbit en  = P3^1;
sbit sda = P1^1;
sbit scl = P1^0;
sfr lcd_port = 0xA0;
sfr seg_port = 0x80;

unsigned char arr[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

// Function declarations
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_init();
void lcd_clear();
void delay(unsigned int);
void display_str(unsigned char *);

void start_i2c();
void stop_i2c();
unsigned char write_i2c(unsigned char);
unsigned char read_i2c(unsigned char);
void i2c_ack();
void i2c_nack();
void segment(unsigned char);
unsigned char x;
unsigned char y;
unsigned char z;
void main()
{
    lcd_init();
    display_str("I2C DATA : ");
    lcd_cmd(0x8B);  // Position cursor
    
    start_i2c();
    write_i2c(0xA4);    // Slave write address
    write_i2c(0x00);    // Memory address or register
    y = write_i2c('0');     // Data byte
	  if (y == 0)
		{
			z = (z + 1) % 9;
			segment(z);
		}
		else if (y == 1)
		{
			return;
		}
    stop_i2c();

    delay(10);

    start_i2c();
    write_i2c(0xA4);    // Slave write address
    write_i2c(0x00);    // Memory address or register
    start_i2c();        // Repeated start
    write_i2c(0xA1);    // Slave read address (0xA0 + 1)
    x = read_i2c(0);    // Read data, send ACK (0 = ACK)
    stop_i2c();

    // Display the read data on LCD (convert to char or hex)
    lcd_data(x);   

    while(1);
}

void segment(unsigned char s)
{
	seg_port = (arr[s]);
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
    delay(5);
    sda = 0;      // START condition: SDA goes low while SCL is high
    delay(5);
    scl = 0;
}

void stop_i2c()
{
    sda = 0;
    scl = 1;
    delay(5);
    sda = 1;      // STOP condition: SDA goes high while SCL is high
    delay(5);
}

unsigned char write_i2c(unsigned char s)
{
    unsigned char i, ack;

    for(i = 0; i < 8; i++)
    {
        sda = (s & 0x80) ? 1 : 0;  // Send MSB first
        scl = 1;
        delay(5);
        scl = 0;
        s <<= 1;
    }

    sda = 1;        // Release SDA for ACK bit from slave
    scl = 1;
    delay(5);
    ack = sda;      // Read ACK bit from slave (0 means ACK)
    scl = 0;

    return ack;     // 0 if ACK received, 1 if NACK
}

unsigned char read_i2c(unsigned char nack)
{
    unsigned char i, Data = 0;

    sda = 1; // Release SDA so slave can drive it

    // Configure SDA as input here, if possible

    for(i = 0; i < 8; i++)
    {
        scl = 1;
        delay(5);

        Data <<= 1;
        Data |= sda;  // Read bit from SDA

        scl = 0;
        delay(5);
    }

    if(nack)
        i2c_nack();
    else
        i2c_ack();

    return Data;
}

void i2c_ack()
{
    sda = 0;  // Pull SDA low to ACK
    scl = 1;
    delay(5);
    scl = 0;
    sda = 1;  // Release SDA
}

void i2c_nack()
{
    sda = 1;  // Release SDA (high) to NACK
    scl = 1;
    delay(5);
    scl = 0;
}
