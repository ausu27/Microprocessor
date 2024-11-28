#ifndef OK_STM767_H
#define OK_STM767_H

/* ---------------------------------------------------------------------------- */

typedef unsigned char	U08;			// data type definition
typedef   signed char	S08;

typedef unsigned short 	U16;
typedef   signed short	S16;

typedef unsigned int 	U32;
typedef   signed int	S32;



/* ---------------------------------------------------------------------------- */
/*		사용자 정의 함수 선언						*/
/* ---------------------------------------------------------------------------- */

void Initialize_MCU(void);			// initialize STM32F767VGT6 MCU
void Delay_us(U32 time_us);			// time delay for us in 216MHz
void Delay_ms(U32 time_ms);			// time delay for ms in 216MHz
void Beep(void);				// beep for 50 ms
void Beep_3times(void);				// beep 3 times
unsigned char Key_input(void);			// input key KEY1~KEY4
unsigned char Key_input_silent(void);		// input key KEY1~KEY4 without beep sound
void LED_on(void);				// turn LED1 on
void LED_off(void);				// turn LED1 off
void LED_toggle(void);				// toggle LED1 to blink

void Initialize_LCD(void);			// initialize text LCD module
void LCD_command(U08 command);			// write a command(instruction) to text LCD
void LCD_data(U08 data);			// display a character on text LCD
void LCD_string(U08 command, U08 *string);	// display a string on LCD

void LCD_binary(U16 number, U08 digit);		// display 8 or 16 bit binary number
void LCD_unsigned_decimal(U32 number, U08 zerofill, U08 digit); // display unsigned decimal number
void LCD_signed_decimal(S32 number, U08 zerofill, U08 digit);	// display signed decimal number
void LCD_hexadecimal(U32 number, U08 digit);	// display hexadecimal number
void LCD_0x_hexadecimal(U32 number, U08 digit);	// display hexadecimal number with 0x
void LCD_unsigned_float(float number, U08 integral, U08 fractional); // display unsigned floating-point number
void LCD_signed_float(float number, U08 integral, U08 fractional);   // display signed floating-point number

void DS3234_initialize(void);			// initialize DS3234 RTC
unsigned char DS3234_read(U16 address);		// read DS3234
void DS3234_write(U16 address, U08 value);	// write DS3234

/* ---------------------------------------------------------------------------- */
/*		사용자 정의 함수 선언						*/
/* ---------------------------------------------------------------------------- */

void Initialize_TFT_LCD(void);			// initialize TFT-LCD with HX8347-A
void TFT_command(U16 IR);			// write IR to TFT-LCD 
void TFT_data(U16 data);			// write data to TFT-LCD 
void TFT_write(U16 reg, U16 val);		// write TFT-LCD register
void TFT_clear_screen(void);			// TFT-LCD clear screen with black color
void TFT_color_screen(U16 color);		// TFT-LCD full screen color
void TFT_GRAM_address(U16 xPos, U16 yPos);	// set GRAM address of TFT-LCD
void TFT_xy(U08 xChar, U08 yChar);		// set character position
void TFT_color(U16 colorfore, U16 colorback);	// set foreground and background color
void TFT_pixel(U16 xPos, U16 yPos, U16 color);	// write a pixel
void TFT_landscape_mode(void);			// set landscape entry mode
void TFT_portrait_mode(void);			// set portrait entry mode
void TFT_cursor(U16 cursor_color);		// set cursor and color
void TFT_underscore(U16 underscore_color);	// set underscore and color
void TFT_outline(U16 outline_color);		// set outline and color
void TFT_string(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, U08 *str);	// write TFT-LCD string
void TFT_English(U08 code);			// write a English ASCII character
unsigned short KS_code_conversion(U16 KS_code);	// convert 완성형(KS) to 조합형(KSSM)
void TFT_Korean(U16 code);			// write a Korean character
void TFT_English_pixel(U16 Xpixel,U16 Ypixel, U08 code); // write a English ASCII character

void TFT_binary(U32 number, U08 digit);		// display binary number
void TFT_unsigned_decimal(U32 number, U08 zerofill, U08 digit); // display unsigned decimal number
void TFT_signed_decimal(S32 number, U08 zerofill, U08 digit);	// display signed decimal number
void TFT_hexadecimal(U32 number, U08 digit);	// display hexadecimal number
void TFT_0x_hexadecimal(U32 number, U08 digit);	// display hexadecimal number with 0x
void TFT_unsigned_float(float number, U08 integral, U08 fractional); // display unsigned floating-point number
void TFT_signed_float(float number, U08 integral, U08 fractional);   // display signed floating-point number

void Line(S16 x1,S16 y1, S16 x2,S16 y2, U16 color);	// draw a straight line
void Rectangle(S16 x1,S16 y1, S16 x2,S16 y2, U16 color);// draw a rectangle
void Block(S16 x1,S16 y1, S16 x2,S16 y2, U16 color, U16 fill); // draw a rectangle with filled color
void Circle(S16 x1,S16 y1, S16 r, U16 color);	// draw a circle
void Sine(S16 peak, U08 mode, U16 color);	// draw a sine curve

void Initialize_touch_screen(void);		// initialize touch screen
unsigned short Read_ADS7846(U16 command);	// read X, Y value from ADS7846
void Touch_screen_input(void);			// touch screen input of X, Y average value from ADS7846

/* ---------------------------------------------------------------------------- */
/*	TFT-LCD 기본 문자 출력 함수(영문 : 8x16 픽셀, 한글 : 16x16 픽셀)	*/
/* ---------------------------------------------------------------------------- */

#define Transparent	0x0821			//   1,   1,   1
#define White		0xFFFF			// 255, 255, 255
#define Black		0x0000			//   0,   0,   0
#define Red		0xF800			// 255,   0,   0
#define Green		0x07E0			//   0, 255,   0
#define Blue		0x001F			//   0,   0, 255
#define Yellow		0xFFE0			// 255, 255,   0
#define Cyan		0x07FF			//   0, 255, 255
#define Magenta		0xF81F			// 255,   0, 255
#define Brown		0xA145			// 160,  40,  40
#define Khaki		0xF731			// 240, 228, 136
#define Orange		0xFD20			// 255, 164,   0
#define Pink		0xFB56			// 255, 104, 176
#define Silver		0xC618			// 192, 192, 192
#define Violet		0xEC1D			// 232, 128, 232
#define Olive		0x8400			// 128, 128,   0
#define Purple		0x8010			// 128,   0, 128
#define Maroon          0x7800			// 128,   0,   0
#define Navy            0x000F			//   0,   0, 128
#define DarkGreen       0x03E0			//   0, 128,   0
#define DarkCyan        0x03EF			//   0, 128, 128
#define DarkGrey        0x7BEF			// 128, 128, 128
#define LightGrey       0xC618			// 192, 192, 192

/* ---------------------------------------------------------------------------- */
/*		    TFT-32A 보드 터치 스크린 컨트롤러 입력 함수			*/
/* ---------------------------------------------------------------------------- */

#define x_touch_min	250			// minimum value of touch screen x-axis
#define x_touch_max	3700			// maximum value of touch screen x-axis
#define y_touch_min	350			// minimum value of touch screen y-axis
#define y_touch_max	3750			// maximum value of touch screen y-axis

#define ADS7846_CMD_X	0x00D0			// 12-bit resolution X position measurement command
#define ADS7846_CMD_Y	0x0090			// 12-bit resolution Y position measurement command

#define KEY1		0x0E			// KEY1 value
#define KEY2		0x0D			// KEY2 value
#define KEY3		0x0B			// KEY3 value
#define KEY4		0x07			// KEY4 value
#define no_key		0x0F			// no key input
#define MENU_key	0x0A			// MENU key

#endif