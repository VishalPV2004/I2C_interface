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

unsigned int decToBcd(unsigned int);
unsigned int BcdTodec(unsigned int);

unsigned int hr;
unsigned char min;
unsigned char sec;

/*void main()
{
    lcd_init();

    // --- Set initial time once ---
    start_i2c();
    write_i2c(0xD0);           // Slave write address
    write_i2c(0x00);           // Start at seconds register
    write_i2c(decToBcd(0));    // Seconds = 00, CH=0
    write_i2c(decToBcd(59));   // Minutes = 59
    write_i2c(decToBcd(23));   // Hours = 23 (24-hr mode)
    stop_i2c();

    while(1){
        unsigned int sec, min, hr;

        delay(200); // Visible refresh

        // --- Read hours, minutes, seconds ---
				start_i2c();
				write_i2c(0xD0);
				write_i2c(0x00);
				start_i2c();
				write_i2c(0xD1);

				sec = BcdTodec(read_i2c(0) & 0x7F);  // mask CH
				min = BcdTodec(read_i2c(0) & 0x7F);
				hr  = BcdTodec(read_i2c(1) & 0x3F);  // mask hour mode bits
				stop_i2c();

        // --- Display on LCD ---
        lcd_cmd(0x80);  // Start of LCD line
        lcd_data((hr/10) + 0x30);
        lcd_data((hr%10) + 0x30);
        lcd_data(':');
        lcd_data((min/10) + 0x30);
        lcd_data((min%10) + 0x30);
        lcd_data(':');
        lcd_data((sec/10) + 0x30);
        lcd_data((sec%10) + 0x30);
    }
}*/

void main() {
    lcd_init();

    while(1) {
        // --- Read hours ---
        start_i2c();
        write_i2c(0xD0);
        write_i2c(0x02);  // Hours register
        start_i2c();
        write_i2c(0xD1);
        hr = BcdTodec(read_i2c(1) & 0x3F);
        stop_i2c();

        // --- Read minutes ---
        start_i2c();
        write_i2c(0xD0);
        write_i2c(0x01);  // Minutes register
        start_i2c();
        write_i2c(0xD1);
        min = BcdTodec(read_i2c(1) & 0x7F);
        stop_i2c();

        // --- Read seconds ---
        start_i2c();
        write_i2c(0xD0);
        write_i2c(0x00);  // Seconds register
        start_i2c();
        write_i2c(0xD1);
        sec = BcdTodec(read_i2c(1) & 0x7F);
        stop_i2c();

        // --- Display time ---
        lcd_cmd(0x80); // Line 1, position 0
        display_str("Time:");

        // Hours
        lcd_data((hr / 10) + '0');
        lcd_data((hr % 10) + '0');
        lcd_data(':');

        // Minutes
        lcd_data((min / 10) + '0');
        lcd_data((min % 10) + '0');
        lcd_data(':');

        // Seconds
        lcd_data((sec / 10) + '0');
        lcd_data((sec % 10) + '0');

        delay(200); // small delay
    }
}



unsigned int decToBcd(unsigned int value)
{
	return (((value/10) << 4) + (value % 10));
}

unsigned int BcdTodec(unsigned int value)
{
	return (((value >> 4)*10) + (value & 0x0F));
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
	lcd_cmd(0x0c);   // Display ON, cursor ON
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
