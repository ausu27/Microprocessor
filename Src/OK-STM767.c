/* ============================================================================ */
/*		OK-STM767 V1.0 키트용(STM32F767VGT6) 기본 헤더 파일		*/
/* ============================================================================ */
/*					Programmed by Duck-Yong Yoon in 2016.   */

// -------------------------------------------------------------------------------
//   이 헤더파일은 OK-STM767 키트 V1.0을 위한 것입니다. 이것은 아래의 조건에서만
//  올바른 동작을 보장합니다.
// -------------------------------------------------------------------------------
//	(1) 시스템 클록  : 216MHz
//	(2) C 컴파일러   : IAR EWARM V7.80.2
//	(3) 최적화 옵션  : High/Size
//	(4) CSTACK 크기  : 0x1000
//	(5) TFT-LCD 모듈 : Ohm사 FL320QVC18-A0(HX8347-A)
// -------------------------------------------------------------------------------

#include "stm32f767xx.h"
#include <math.h>
#include "OK-STM767.h"
#include "STM_Font.h"

unsigned char  ScreenMode = 'L';		// screen mode(P = portrait, L = landscape)
unsigned char  XcharacterLimit = 40;		// character number of line (30 or 40)
unsigned char  YcharacterLimit = 30;		// line number of screen (40 or 30)
unsigned char  Xcharacter, Ycharacter;		// Xcharacter(0-29), Ycharacter(0-39) for portrait
						// Xcharacter(0-39), Ycharacter(0-29) for landscape
unsigned char  Xcursor, Ycursor;		// cursor position
unsigned char  cursor_flag = 0;			// 0 = cursor off, 1 = cursor on
unsigned short cursor;				// cursor color
unsigned char  underscore_flag = 0;		// 0 = underscore off, 1 = underscore on
unsigned short underscore;			// underscore color
unsigned char  outline_flag = 0;		// 0 = outline off, 1 = outline on
unsigned short outline;				// outline color
unsigned short foreground, background;		// foreground and background color
unsigned char  Kfont_type = 'M';		// M = 명조체, G = 고딕체, P = 필기체

void Initialize_TFT_LCD(void)			/* initialize TFT-LCD with HX8347-A */
{
  RCC->AHB1ENR |= 0x00000018;			// enable clock of port D,E

  GPIOD->MODER = 0x55555555;			// 포트D 설정
  GPIOD->ODR = 0x00000000;			// TFT_D15~TFT_D0 = 0
  GPIOD->OSPEEDR = 0x55555555;			// TFT-LCD signal = 50MHz medium speed

  GPIOE->MODER &= 0xFFFFFF00;			// 포트E 설정
  GPIOE->MODER |= 0x00000055;
  GPIOE->ODR |= 0x0000000D;			// -TFT_RESET = -TFT_CS = -TFT_WR = 1
  GPIOE->OSPEEDR &= 0xFFFFFF00;			// 50MHz medium speed
  GPIOE->OSPEEDR |= 0x00000055;

  GPIOE->BSRR = 0x00080000;			// -TFT_RESET = 0
  Delay_ms(1);
  GPIOE->BSRR = 0x00000008;			// -TFT_RESET = 1
  Delay_ms(120);

  TFT_write(0x02, 0x0000);			// window setting
  TFT_write(0x03, 0x0000);			// x = 0~319
  TFT_write(0x04, 0x0001);
  TFT_write(0x05, 0x003F);
  TFT_write(0x06, 0x0000);			// y = 0~239
  TFT_write(0x07, 0x0000);
  TFT_write(0x08, 0x0000);
  TFT_write(0x09, 0x00EF);

  TFT_write(0x01, 0x0006);			// display setting
  TFT_write(0x16, 0x0068);			// MV=1, MX=1, MY=0, BGR=1
  TFT_write(0x18, 0x0000);			// SM=0
  TFT_write(0x70, 0x0065);			// GS=1, SS=1, 16 bit/pixel
  TFT_write(0x23, 0x0095);
  TFT_write(0x24, 0x0095);
  TFT_write(0x25, 0x00FF);
  TFT_write(0x27, 0x0002);
  TFT_write(0x28, 0x0002);
  TFT_write(0x29, 0x0002);
  TFT_write(0x2A, 0x0002);
  TFT_write(0x2C, 0x0002);
  TFT_write(0x2D, 0x0002);
  TFT_write(0x3A, 0x0001);
  TFT_write(0x3B, 0x0001);
  TFT_write(0x3C, 0x00F0);
  TFT_write(0x3D, 0x0000);
  Delay_ms(20);
  TFT_write(0x35, 0x0038);
  TFT_write(0x36, 0x0078);
  TFT_write(0x3E, 0x0038);
  TFT_write(0x40, 0x000F);
  TFT_write(0x41, 0x00F0); 

  TFT_write(0x19, 0x0049);			// power setting
  TFT_write(0x93, 0x000F);
  Delay_ms(10);
  TFT_write(0x20, 0x0040);
  TFT_write(0x1D, 0x0007);
  Delay_ms(10); 
  TFT_write(0x1E, 0x0000);
  TFT_write(0x1F, 0x0004);
  TFT_write(0x44, 0x0040);			// VCom control for 3.3V
  TFT_write(0x45, 0x0012);
  Delay_ms(10); 
  TFT_write(0x1C, 0x0004);
  Delay_ms(20);
  TFT_write(0x43, 0x0080);
  Delay_ms(5);
  TFT_write(0x1B, 0x0018);
  Delay_ms(40); 
  TFT_write(0x1B, 0x0010);
  Delay_ms(40);

  TFT_write(0x46, 0x00A4);			// gamma setting 
  TFT_write(0x47, 0x0053);
  TFT_write(0x48, 0x0010);
  TFT_write(0x49, 0x005F);
  TFT_write(0x4A, 0x0004);
  TFT_write(0x4B, 0x003F);
  TFT_write(0x4C, 0x0002);
  TFT_write(0x4D, 0x00F6);
  TFT_write(0x4E, 0x0012);
  TFT_write(0x4F, 0x004C);
  TFT_write(0x50, 0x0046);
  TFT_write(0x51, 0x0044);

  TFT_write(0x90, 0x007F);			// display on setting
  TFT_write(0x26, 0x0004);
  Delay_ms(40);
  TFT_write(0x26, 0x0024);
  TFT_write(0x26, 0x002C);
  Delay_ms(40); 
  TFT_write(0x26, 0x003C);

  TFT_write(0x57, 0x0002);			// internal VDDD setting
  TFT_write(0x55, 0x0000);
  TFT_write(0xFE, 0x005A);
  TFT_write(0x57, 0x0000);  

  TFT_clear_screen();				// clear screen
}

void TFT_command(U16 IR)			/* write IR to TFT-LCD */
{
  GPIOE->BSRR = 0x00060000;			// -CS = 0, Rs = 0
  GPIOD->ODR  = IR;
  GPIOE->BSRR = 0x00010000;			// -WR = 0
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  GPIOE->BSRR = 0x00000001;			// -WR = 1
  asm("NOP");
  GPIOE->BSRR = 0x00000004;			// -CS = 1
}

void TFT_data(U16 data)				/* write data to TFT-LCD */
{
  GPIOE->BSRR = 0x00040002;			// -CS = 0, RS = 1
  GPIOD->ODR  = data;
  GPIOE->BSRR = 0x00010000;			// -WR = 0
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  asm("NOP");
  GPIOE->BSRR = 0x00000001;			// -WR = 1
  asm("NOP");
  GPIOE->BSRR = 0x00000004;			// -CS = 1
}

void TFT_write(U16 reg, U16 val)		/* write TFT-LCD register */
{
  TFT_command(reg);
  TFT_data(val);
}

/* ---------------------------------------------------------------------------- */
/*		TFT 보드 출력제어 함수						*/
/* ---------------------------------------------------------------------------- */

void TFT_clear_screen(void)			/* TFT-LCD clear screen with black color */
{
  TFT_color_screen(Black);
}

void TFT_color_screen(U16 color)		/* TFT-LCD full screen color */
{
  unsigned short i, j;

  TFT_GRAM_address(0,0);

  for(i = 0; i < 320; i++)
    for(j = 0; j < 240; j++)
      TFT_data(color);
}

void TFT_GRAM_address(U16 xPos, U16 yPos)	/* set GRAM address of TFT-LCD */
{
  if(ScreenMode == 'L')
    { if((xPos > 319) || (yPos > 239))
        return;
      TFT_write(0x02, xPos >> 8);		// xPos = 0~319
      TFT_write(0x03, xPos & 0x00FF);
      TFT_write(0x06, 0x0000);			// yPos = 0~239
      TFT_write(0x07, yPos);
    }
  else
    { if((xPos > 239) || (yPos > 319))
        return;
      TFT_write(0x02, 0x0000);			// xPos = 0~239
      TFT_write(0x03, xPos);
      TFT_write(0x06, yPos >> 8);		// yPos = 0~319
      TFT_write(0x07, yPos & 0x00FF);
    }

  TFT_command(0x22);
}

void TFT_xy(U08 xChar, U08 yChar)		/* set character position (x,y) */
{
  Xcharacter = xChar;
  Ycharacter = yChar;
}

void TFT_color(U16 colorfore, U16 colorback)	/* set foreground and background color */
{
  foreground = colorfore;
  background = colorback;
}

void TFT_pixel(U16 xPos, U16 yPos, U16 color)	/* write a pixel */
{
  if(ScreenMode == 'L')
    { if((xPos > 319) || (yPos > 239))
        return;
      TFT_write(0x02, xPos >> 8);		// xPos = 0~319
      TFT_write(0x03, xPos & 0x00FF);
      TFT_write(0x06, 0x0000);			// yPos = 0~239
      TFT_write(0x07, yPos);
    }
  else
    { if((xPos > 239) || (yPos > 319))
        return;
      TFT_write(0x02, 0x0000);			// xPos = 0~239
      TFT_write(0x03, xPos);
      TFT_write(0x06, yPos >> 8);		// yPos = 0~319
      TFT_write(0x07, yPos & 0x00FF);
    }

  if(color != Transparent)			// transparent mode ?
    TFT_write(0x22, color);
}

/* ---------------------------------------------------------------------------- */
/*		TFT-LCD 문자 출력 함수						*/
/* ---------------------------------------------------------------------------- */

void TFT_landscape_mode(void)			/* set landscape mode */
{
  TFT_write(0x16, 0x0068);			// MV=1, MX=1, MY=0, BGR=1

  TFT_write(0x02, 0x0000);			// x = 0~319
  TFT_write(0x03, 0x0000);
  TFT_write(0x04, 0x0001);
  TFT_write(0x05, 0x003F);
  TFT_write(0x06, 0x0000);			// y = 0~239
  TFT_write(0x07, 0x0000);
  TFT_write(0x08, 0x0000);
  TFT_write(0x09, 0x00EF);

  XcharacterLimit = 40;				// 40x30 character unit
  YcharacterLimit = 30;
  ScreenMode = 'L';
}

void TFT_portrait_mode(void)			/* set portrait mode */
{
  TFT_write(0x16, 0x0008);			// MV=0, MX=0, MY=0, BGR=1

  TFT_write(0x02, 0x0000);			// x = 0~239
  TFT_write(0x03, 0x0000);
  TFT_write(0x04, 0x0000);
  TFT_write(0x05, 0x00EF);
  TFT_write(0x06, 0x0000);			// y = 0~319
  TFT_write(0x07, 0x0000);
  TFT_write(0x08, 0x0001);
  TFT_write(0x09, 0x003F);

  XcharacterLimit = 30;				// 30x40 character unit
  YcharacterLimit = 40;
  ScreenMode = 'P';
}

void TFT_cursor(U16 cursor_color)		/* set cursor and color */
{
  if(cursor_color == Transparent)		// disable cursor
    cursor_flag = 0;
  else						// enable cursor
    { cursor_flag = 1;
      cursor = cursor_color;
    }
}

void TFT_underscore(U16 underscore_color)	/* set underscore and color */
{
  if(underscore_color == Transparent)		// disable underscore
    underscore_flag = 0;
  else						// enable underscore
    { underscore_flag = 1;
      underscore = underscore_color;
    }
}

void TFT_outline(U16 outline_color)		/* set outline and color */
{
  if(outline_color == Transparent)		// disable outline
    outline_flag = 0;
  else						// enable outline
    { outline_flag = 1;
      outline = outline_color;
    }
}

void TFT_string(U08 xChar, U08 yChar, U16 colorfore, U16 colorback, U08 *str)	/* write TFT-LCD string */
{
  unsigned char ch1, ch2;
  unsigned short hangeul;

  Xcharacter = xChar;
  Ycharacter = yChar;

  foreground = colorfore;
  background = colorback;

  while(*str)
    { ch1 = *str;
      str++;

      if(ch1 < 0x80)				// English ASCII character
	TFT_English(ch1);
      else					// Korean
	{ ch2 = *str;
	  hangeul = (ch1 << 8) + ch2;
          str++;
	  hangeul = KS_code_conversion(hangeul);// convert 완성형 to 조합형
	  TFT_Korean(hangeul);
	}
    }
}

void TFT_English(U08 code)			/* write a English ASCII character */
{
  unsigned char  data, x, y;
  unsigned short pixel[8][16];
  unsigned short xPos, yPos, dot0, dot1, dot2, dot3, dot4;

  for(x = 0; x < 8; x++)			// read English ASCII font
    { data = E_font[code][x];
      for(y = 0; y < 8; y++)
        { if(data & 0x01) pixel[x][y] = foreground;
	  else            pixel[x][y] = background;
	  data = data >> 1;
	}
    }

  for(x = 0; x < 8; x++)
    { data = E_font[code][x+8];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y+8] = foreground;
	  else            pixel[x][y+8] = background;
	  data = data >> 1;
	}
    }

  if(outline_flag == 1)				// display outline
    for(x = 0; x < 8; x++)
      { dot0 = E_font[code][x] + E_font[code][x+8]*256;
        dot1 = dot0 >> 1;				 // up side
        dot2 = dot0;					 // down side
        dot3 = E_font[code][x+1] + E_font[code][x+9]*256;// left side
        dot4 = E_font[code][x-1] + E_font[code][x+7]*256;// right side

        for(y = 0; y < 15; y++)
	  { if(!(dot0 & 0x0001))
	      { if(dot1 & 0x0001) pixel[x][y] = outline;
	        if(dot2 & 0x0001) pixel[x][y] = outline;
	        if((dot3 & 0x0001) && (x < 7 )) pixel[x][y] = outline;
	        if((dot4 & 0x0001) && (x > 0 )) pixel[x][y] = outline;
	      }

            dot1 >>= 1;
            dot2 = dot0;
	    dot0 >>= 1;
	    dot3 >>= 1;
	    dot4 >>= 1;
          }
      }
  if(Xcharacter >= XcharacterLimit)		// end of line ?
    { Xcharacter = 0;
      Ycharacter += 2;
      if(Ycharacter >= (YcharacterLimit-1))	// end of screen ?
        Ycharacter = 0;
    }

  if(underscore_flag == 1)
    { for(x = 0; x < 8; x++)			// display underscore
          pixel[x][15] = underscore;
    }

  if((cursor_flag == 1) && (Xcharacter == Xcursor) &&(Ycharacter == Ycursor))
    { for(x = 0; x < 8; x++)			// display cursor
        { pixel[x][14] = cursor;
          pixel[x][15] = cursor;
        }
    }

  if(background != Transparent)			// transparent mode ?
    { xPos = Xcharacter*8;

      for(y = 0; y < 16; y++)			// if no, write a character in higher speed
        { yPos = Ycharacter*8 + y;

          if(ScreenMode == 'L')
	    { TFT_write(0x02, xPos >> 8);	// xPos = 0~319
              TFT_write(0x03, xPos & 0x00FF);
              TFT_write(0x06, 0x0000);		// yPos = 0~239
              TFT_write(0x07, yPos);
	    }
          else
            { TFT_write(0x02, 0x0000);		// xPos = 0~239
              TFT_write(0x03, xPos);
              TFT_write(0x06, yPos >> 8);	// yPos = 0~319
              TFT_write(0x07, yPos & 0x00FF);
            }

          TFT_command(0x22);
          TFT_data(pixel[0][y]);
          TFT_data(pixel[1][y]);
          TFT_data(pixel[2][y]);
          TFT_data(pixel[3][y]);
          TFT_data(pixel[4][y]);
          TFT_data(pixel[5][y]);
          TFT_data(pixel[6][y]);
          TFT_data(pixel[7][y]);
        }
    }
  else
    { for(y = 0; y < 16; y++)			// if yes, write a character
        for(x = 0; x < 8; x++)
          TFT_pixel(Xcharacter*8 + x, Ycharacter*8 + y, pixel[x][y]);
    }

  Xcharacter += 1;				// go to next position
}

unsigned short KS_code_conversion(U16 KS_code)	/* convert 완성형(KS) to 조합형(KSSM) */
{
  unsigned char  HB, LB;
  unsigned short index, KSSM;

  HB = KS_code >> 8;
  LB = KS_code & 0x00FF;

  if(KS_code >= 0xB0A1 && KS_code <= 0xC8FE)
    { index = (HB - 0xB0)*94 + LB - 0xA1;
      KSSM  =  KS_Table[index][0] * 256;
      KSSM |=  KS_Table[index][1];

      return KSSM;
    }
  else
    return 0;
}

void TFT_Korean(U16 code)			/* write a Korean character */
{
  unsigned char  cho_5bit, joong_5bit, jong_5bit;
  unsigned char  cho_bul, joong_bul, jong_bul = 0, i, jong_flag;
  unsigned short ch;
  unsigned char  Kbuffer[32] = {0};		// 32 U08 Korean font buffer

  cho_5bit   = table_cho[(code >> 10) & 0x001F];  // get 5bit(14-10) of chosung
  joong_5bit = table_joong[(code >> 5) & 0x001F]; // get 5bit(09-05) of joongsung
  jong_5bit  = table_jong[code & 0x001F];	  // get 5bit(04-00) of jongsung

  if(jong_5bit == 0)				// if jongsung not exist
    { jong_flag = 0;
      cho_bul = bul_cho1[joong_5bit];
      if((cho_5bit == 1) || (cho_5bit == 16))
   	joong_bul = 0;
      else
	joong_bul = 1;
    }
  else						// if jongsung exist
    { jong_flag = 1;
      cho_bul = bul_cho2[joong_5bit];
      if((cho_5bit == 1) || (cho_5bit == 16))
	joong_bul = 2;
      else
	joong_bul = 3;
      jong_bul = bul_jong[joong_5bit];
    }

  ch = cho_bul*20 + cho_5bit;			// get chosung font 
  if(Kfont_type == 'M')
    for(i = 0; i < 32; i++) Kbuffer[i] = KM_font[ch][i];
  else if(Kfont_type == 'G')
    for(i = 0; i < 32; i++) Kbuffer[i] = KG_font[ch][i];
  else if(Kfont_type == 'P')
    for(i = 0; i < 32; i++) Kbuffer[i] = KP_font[ch][i];

  ch = 8*20 + joong_bul*22 + joong_5bit;	// OR joongsung font
  if(Kfont_type == 'M')
    for(i = 0; i < 32; i++) Kbuffer[i] |= KM_font[ch][i];
  else if(Kfont_type == 'G')
    for(i = 0; i < 32; i++) Kbuffer[i] |= KG_font[ch][i];
  else if(Kfont_type == 'P')
    for(i = 0; i < 32; i++) Kbuffer[i] |= KP_font[ch][i];

  if(jong_flag)					// OR jongsung font
    { ch = 8*20 + 4*22 + jong_bul*28 + jong_5bit;
      if(Kfont_type == 'M')
        for(i = 0; i < 32; i++) Kbuffer[i] |= KM_font[ch][i];
      else if(Kfont_type == 'G')
        for(i = 0; i < 32; i++)	Kbuffer[i] |= KG_font[ch][i];
      else if(Kfont_type == 'P')
        for(i = 0; i < 32; i++)	Kbuffer[i] |= KP_font[ch][i];
    }

  unsigned char  data, x, y;
  unsigned short pixel[16][16];
  unsigned short xPos, yPos, dot0, dot1, dot2, dot3, dot4;

  for(x = 0; x < 16; x++)			// read Korean font
    { data = Kbuffer[x];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y] = foreground;
	  else            pixel[x][y] = background;
	  data = data >> 1;
	}
    }

  for(x = 0; x < 16; x++)
    { data = Kbuffer[x+16];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y+8] = foreground;
	  else            pixel[x][y+8] = background;
	  data = data >> 1;
	}
    }

  if(outline_flag == 1)				// display outline
    for(x = 0; x < 16; x++)
      { dot0 = Kbuffer[x] + Kbuffer[x+16]*256;
        dot1 = dot0 >> 1;			 // up side
        dot2 = dot0;				 // down side
        dot3 = Kbuffer[x+1] + Kbuffer[x+17]*256; // left side
        dot4 = Kbuffer[x-1] + Kbuffer[x+15]*256; // right side

        for(y = 0; y < 16; y++)
	  { if(!(dot0 & 0x0001))
	      { if(dot1 & 0x0001) pixel[x][y] = outline;
	        if(dot2 & 0x0001) pixel[x][y] = outline;
 	        if((dot3 & 0x0001) && (x < 15 )) pixel[x][y] = outline;
	        if((dot4 & 0x0001) && (x > 0 ))  pixel[x][y] = outline;
	      }

	    dot1 >>= 1;
	    dot2 = dot0;
	    dot0 >>= 1;
	    dot3 >>= 1;
	    dot4 >>= 1;
          }
      }

  if(Xcharacter >= (XcharacterLimit-1))		// end of line ?
    { Xcharacter = 0;
      Ycharacter += 2;
      if(Ycharacter >= (YcharacterLimit-1))	// end of screen ?
        Ycharacter = 0;
    }
  
  if(underscore_flag == 1)
    { for(x = 0; x < 16; x++)			// display underscore
          pixel[x][15] = underscore;
    }

  if((cursor_flag == 1) && (Xcharacter == Xcursor) &&(Ycharacter == Ycursor))
    { for(x = 0; x < 16; x++)			// display cursor
        { pixel[x][14] = cursor;
          pixel[x][15] = cursor;
        }
    }

  if(background != Transparent)			// transparent mode ?
    { xPos = Xcharacter*8;

      for(y = 0; y < 16; y++)			// if no, write a character in higher speed
        { yPos = Ycharacter*8 + y;

          if(ScreenMode == 'L')
	    { TFT_write(0x02, xPos >> 8);	// xPos = 0~319
              TFT_write(0x03, xPos & 0x00FF);
              TFT_write(0x06, 0x0000);		// yPos = 0~239
              TFT_write(0x07, yPos);
	    }
          else
            { TFT_write(0x02, 0x0000);		// xPos = 0~239
              TFT_write(0x03, xPos);
              TFT_write(0x06, yPos >> 8);	// yPos = 0~319
              TFT_write(0x07, yPos & 0x00FF);
            }

          TFT_command(0x22);
          TFT_data(pixel[0][y]);
          TFT_data(pixel[1][y]);
          TFT_data(pixel[2][y]);
          TFT_data(pixel[3][y]);
          TFT_data(pixel[4][y]);
          TFT_data(pixel[5][y]);
          TFT_data(pixel[6][y]);
          TFT_data(pixel[7][y]);
          TFT_data(pixel[8][y]);
          TFT_data(pixel[9][y]);
          TFT_data(pixel[10][y]);
          TFT_data(pixel[11][y]);
          TFT_data(pixel[12][y]);
          TFT_data(pixel[13][y]);
          TFT_data(pixel[14][y]);
          TFT_data(pixel[15][y]);
        }
    }
  else
    { for(y = 0; y < 16; y++)			// if yes, write a character
        for(x = 0; x < 16; x++)
          TFT_pixel(Xcharacter*8 + x, Ycharacter*8 + y, pixel[x][y]);
    }

  Xcharacter += 2;				// go to next position
}

void TFT_English_pixel(U16 Xpixel,U16 Ypixel, U08 code)	/* write a English ASCII character */
{
  unsigned char data, x, y;
  unsigned short pixel[8][16];

  for(x = 0; x < 8; x++)			// read English ASCII font
    { data = E_font[code][x];
      for(y = 0; y < 8; y++)
        { if(data & 0x01) pixel[x][y] = foreground;
	  else            pixel[x][y] = background;
	  data = data >> 1;
	}
    }

  for(x = 0; x < 8; x++)
    { data = E_font[code][x+8];
      for(y = 0; y < 8; y++)
	{ if(data & 0x01) pixel[x][y+8] = foreground;
	  else            pixel[x][y+8] = background;
	  data = data >> 1;
	}
    }

  for(y = 0; y < 16; y++)			// write character
    for(x = 0; x < 8; x++)
      TFT_pixel(Xpixel + x, Ypixel + y, pixel[x][y]);
}

/* ---------------------------------------------------------------------------- */
/*		TFT-LCD 수치 데이터 출력 함수					*/
/* ---------------------------------------------------------------------------- */

void TFT_binary(U32 number, U08 digit)		/* display binary number */
{
  unsigned char i;

  if((digit == 0) || (digit > 32)) return;

  for(i = digit; i > 0; i--)
    TFT_English(((number >> (i-1)) & 0x01) + '0');
}

void TFT_unsigned_decimal(U32 number, U08 zerofill, U08 digit) /* display unsigned decimal number */
{
  unsigned char zero_flag, character;
  unsigned int  div;

  if((digit == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { character = number / div;
      if((character == 0) && (zero_flag == 0) && (div != 1))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      number %= div;
      div /= 10;
    }
}

void TFT_signed_decimal(S32 number, U08 zerofill, U08 digit)	/* display signed decimal number */
{
  unsigned char zero_flag, character;
  unsigned int  div;

  if((digit == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    TFT_English('+');
  else
    { TFT_English('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { character = number / div;
      if((character == 0) && (zero_flag == 0) && (div != 1))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      number %= div;
      div /= 10;
    }
}

void TFT_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number */
{
  unsigned char i, character;

  if((digit == 0) || (digit > 8)) return;

  for(i = digit; i > 0; i--)
    { character = (number >> 4*(i-1)) & 0x0F;
      if(character < 10) TFT_English(character + '0');
      else               TFT_English(character - 10 + 'A');
    }
}

void TFT_0x_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number with 0x */
{
  unsigned char i, character;

  if((digit == 0) || (digit > 8)) return;

  TFT_English('0');
  TFT_English('x');

  for(i = digit; i > 0; i--)
    { character = (number >> 4*(i-1)) & 0x0F;
      if(character < 10) TFT_English(character + '0');
      else               TFT_English(character - 10 + 'A');
    }
}

void TFT_unsigned_float(float number, U08 integral, U08 fractional) /* display unsigned floating-point number */
{
  unsigned char zero_flag, digit, character;	// integral = digits of integral part
  unsigned int  div, integer;			// fractional = digits of fractional part

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        TFT_English('.');
      digit++;
    }
}

void TFT_signed_float(float number, U08 integral, U08 fractional) /* display signed floating-point number */
{
  unsigned char zero_flag, digit, character;
  unsigned int  div, integer;

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    TFT_English('+');
  else
    { TFT_English('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        TFT_English(character + ' ');
      else
        { zero_flag = 1;
          TFT_English(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        TFT_English('.');
      digit++;
    }
}

/* ---------------------------------------------------------------------------- */
/*		그래픽 함수							*/
/* ---------------------------------------------------------------------------- */

void Line(S16 x1,S16 y1, S16 x2,S16 y2, U16 color)	/* draw a straight line */
{
  short x, y;

  if((x1 == x2) && (y1 == y2))			// if x1 == x2 and y1 == y2, it is a dot
    TFT_pixel(x1, y1, color);
  else
    { if(x1 < x2) x = x2 - x1;			// calculate x length
      else        x = x1 - x2;
      if(y1 < y2) y = y2 - y1;			// calculate y length
      else        y = y1 - y2;

      if((x >= y) && (x1 <= x2))		// if x is long and y is short
        for(x = x1; x <= x2; x++)
          { y = y1 + (int)(x - x1)*(int)(y2 - y1)/(x2 - x1);
            TFT_pixel(x, y, color);
          }
      else if((x >= y) && (x1 > x2))
        for(x = x1; x >= x2; x--)
          { y = y1 + (int)(x - x1)*(int)(y2 - y1)/(x2 - x1);
            TFT_pixel(x, y, color);
          }
      else if((x < y) && (y1 <= y2))		// if x is short and y is long
        for(y = y1; y <= y2; y++)
          { x = x1 + (int)(y - y1)*(int)(x2 - x1)/(y2 - y1);
            TFT_pixel(x, y, color);
          }
      else if((x < y) && (y1 > y2))
        for(y = y1; y >= y2; y--)
          { x = x1 + (int)(y - y1)*(int)(x2 - x1)/(y2 - y1);
            TFT_pixel(x, y, color);
          }
    }
}

void Rectangle(S16 x1,S16 y1, S16 x2,S16 y2, U16 color) /* draw a rectangle */
{
  Line(x1,y1, x1,y2, color);			// horizontal line
  Line(x2,y1, x2,y2, color);
  Line(x1,y1, x2,y1, color);			// vertical line
  Line(x1,y2, x2,y2, color);
}

void Block(S16 x1,S16 y1, S16 x2,S16 y2, U16 color, U16 fill) /* draw a rectangle with filled color */
{
  short i;

  Line(x1,y1, x1,y2, color);			// horizontal line
  Line(x2,y1, x2,y2, color);
  Line(x1,y1, x2,y1, color);			// vertical line
  Line(x1,y2, x2,y2, color);

  if((y1 < y2) && (x1 != x2))			// fill block
    { for(i = y1+1; i <= y2-1; i++)
         Line(x1+1,i, x2-1,i, fill);
    }
  else if((y1 > y2) && (x1 != x2))
    { for(i = y2+1; i <= y1-1; i++)
         Line(x1+1,i, x2-1,i, fill);
    }
}

void Circle(S16 x1,S16 y1, S16 r, U16 color)	/* draw a circle */
{
  short x, y;
  float s;

  for(y = y1 - r*3/4; y <= y1 + r*3/4; y++)	// draw with y variable
    { s = sqrt((int)r*(int)r - (int)(y-y1)*(int)(y-y1)) + 0.5;
      x = x1 + (int)s;
      TFT_pixel(x, y, color);
      x = x1 - (int)s;
      TFT_pixel(x, y, color);
    }

  for(x = x1 - r*3/4; x <= x1 + r*3/4; x++)	// draw with x variable
    { s = sqrt((int)r*(int)r - (int)(x-x1)*(int)(x-x1)) + 0.5;
      y = y1 + (int)s;
      TFT_pixel(x, y, color);
      y = y1 - (int)s;
      TFT_pixel(x, y, color);
    }
}

#define M_PI       3.141592654

void Sine(S16 peak, U08 mode, U16 color)	/* draw a sine curve */
{
  short x, y;

  if(mode == 0)
    for(x = 0; x <= 319; x++)
      { y = 120 - (int)(sin((float)x * 1.6875 * M_PI / 180.) * peak + 0.5);
        TFT_pixel(x, y, color);
      }
  else
    for(x = 0; x <= 319; x++)
      { y = 120 + (int)(sin((float)x * 1.6875 * M_PI / 180.) * peak + 0.5);
        TFT_pixel(x, y, color);
      }
}

/* ---------------------------------------------------------------------------- */
/*		    TFT-32A 보드 터치 스크린 컨트롤러 입력 함수			*/
/* ---------------------------------------------------------------------------- */
unsigned short x_12bit, y_12bit;		// x_12bit(0-4095), y_12bit(0-4095)
unsigned short x_touch, y_touch;		// x_touch(0-239),  y_touch(0-319)

void Initialize_touch_screen(void)		/* initialize touch screen */
{
  RCC->AHB1ENR |= 0x00000002;			// enable port B
  RCC->APB1ENR |= 0x00004000;			// enable SPI2 clock

  GPIOB->MODER &= 0x00FFF3FF;			// alternate function
  GPIOB->MODER |= 0xA9000000;			// PB12 = -TS_CS, PB5 = -TS_INT
  GPIOB->AFR[1] &= 0x000FFFFF;			// PB15 = SPI2_MOSI, PB14 = SPI2_MISO, PB13 = SPI2_SCK
  GPIOB->AFR[1] |= 0x55500000;
  GPIOB->ODR |= 0x00001000;			// -TS_CS = 1

  SPI2->CR1 = 0x0364;				// master mode, 54MHz/32 = 1.6875MHz (Max 2.5MHz)
  SPI2->CR2 = 0x0F00;				// 16-bit data, disable SS output, CPOL = CPHA = 0

  Read_ADS7846(ADS7846_CMD_X);			// dummy input for -TS_INT signal
}

unsigned short Read_ADS7846(U16 command)	/* read X, Y value from ADS7846 */
{
  unsigned short axis;

  axis = SPI2->DR;				// clear RXNE flag
  GPIOB->BSRR = 0x10000000;			// -TS_CS = 0
  SPI2->DR = command;
  while((SPI2->SR & 0x0003) != 0x0003);
  axis = SPI2->DR;
  Delay_us(1);
  SPI2->DR = 0x0000;
  while((SPI2->SR & 0x0003) != 0x0003);
  axis = SPI2->DR;
  GPIOB->BSRR = 0x00001000;			// -TS_CS = 1

  axis >>= 3;
  return axis;
}

void Touch_screen_input(void)			/* touch screen input of X, Y average value from ADS7846 */
{
  unsigned char i;
  unsigned short temp;

  x_12bit = 0;					// initial value
  y_12bit = 0;

  for(i = 0; i < 16; i++)			// read X, Y value by 16 times if -PENIRQ enable
    { if((GPIOB->IDR & 0x00000020) == 0)	// -TS_INT = 0 ?
        x_12bit += Read_ADS7846(ADS7846_CMD_X);	// if yes, measure X position
      else					// if not, return with 0
        { x_12bit = 0;
	  y_12bit = 0;
	  break;
	}
      Delay_us(10);

      if((GPIOB->IDR & 0x00000020) == 0)	// -TS_INT = 0 ?
        y_12bit += Read_ADS7846(ADS7846_CMD_Y);	// if yes, measure Y position
      else					// if not, return with 0
        { x_12bit = 0;
	  y_12bit = 0;
	  break;
	}
      Delay_us(10);
    }

  x_12bit >>= 4;				// calculate average for 16 times
  y_12bit >>= 4;

  if((x_12bit == 0) && (y_12bit == 0))
    { x_touch = 0;
      y_touch = 0;
      return;
    }

  if(x_12bit <= x_touch_min)			// convert to pixel x-axis value
    x_touch = 0;
  else if(x_12bit >= x_touch_max)
    x_touch = 239;
  else
    x_touch = (unsigned int)((float)(x_12bit - x_touch_min) * 239./(float)(x_touch_max - x_touch_min));

  if(y_12bit <= y_touch_min)			// convert to pixel y-axis value
    y_touch = 319;
  else if(y_12bit >= y_touch_max)
    y_touch = 0;
  else
    y_touch = 319 - (unsigned int)((float)(y_12bit - y_touch_min) * 319./(float)(y_touch_max - y_touch_min));

  if(ScreenMode == 'L')				// adjust axis value for landscape mode
    { temp = x_12bit;				// exchange x_12bit and y_12bit
      x_12bit = y_12bit;
      y_12bit = temp;
      temp = x_touch;				// exchange x_touch and y_touch
      x_touch = y_touch;
      y_touch = 239 - temp;
    }
}

/* ---------------------------------------------------------------------------- */
/*		OK-STM767 키트 기본 함수					*/
/* ---------------------------------------------------------------------------- */

void SystemInit(void)				/* dummy system function */
{
  asm volatile("NOP");
}

void Initialize_MCU(void)			/* initialize STM32F767VGT6 MCU */
{
// (1) 명령 캐시 및 데이터 캐시 설정
  SCB_EnableICache();				// enable L1 instruction cache
  SCB_EnableDCache();				// enable L1 data cache

// (2) ART 가속기, 프리페치 버퍼, 웨이트 사이클 설정
  FLASH->ACR = 0x00000307;			// 7 waits, enable ART accelerator and prefetch

// (3) HSE 및 PLL 설정(시스템 클록 SYSCLK = 216MHz)
  RCC->CR |= 0x00010001;			// HSE on, HSI on
  while((RCC->CR & 0x00000002) == 0);		// wait until HSIRDY = 1
  RCC->CFGR = 0x00000000;			// SYSCLK = HSI
  while((RCC->CFGR & 0x0000000C) != 0);		// wait until SYSCLK = HSI

  RCC->CR = 0x00010001;				// PLL off, HSE on, HSI on
  RCC->PLLCFGR = 0x09403608;			// SYSCLK = HSE*PLLN/PLLM/PLLP = 16MHz*216/8/2 = 216MHz
						// PLL48CK = HSE*PLLN/PLLM/PLLQ = 16MHz*216/8/9 = 48MHz
  RCC->CR = 0x01010001;				// PLL on, HSE on, HSI on
  while((RCC->CR & 0x02000000) == 0);		// wait until PLLRDY = 1

// (4) 오버드라이브 설정
  RCC->APB1ENR |= 0x10000000;			// 전원모듈 클록(PWREN = 1)
  PWR->CR1 |= 0x00010000;			// over-drive enable(ODEN = 1)
  while((PWR->CSR1 & 0x00010000) == 0);		// ODRDY = 1 ?
  PWR->CR1 |= 0x00020000;			// over-drive switching enable(ODSWEN = 1)
  while((PWR->CSR1 & 0x00020000) == 0);		// ODSRDY = 1 ?

// (5) 주변장치 클록 설정(APB1CLK = APB2CLK = 54MHz)
  RCC->CFGR = 0x3040B402;			// SYSCLK = PLL, AHB = 216MHz, APB1 = APB2 = 54MHz
						// MCO1 = HSE, MCO2 = SYSCLK/4 (timer = 108MHz)
  while((RCC->CFGR & 0x0000000C) != 0x00000008);// wait until SYSCLK = PLL
  RCC->CR |= 0x00080000;			// CSS on

// (6) I/O 보상 설정
  RCC->APB2ENR |= 0x00004000;			// 주변장치 클럭(SYSCFG = 1)
  SYSCFG->CMPCR = 0x00000001;			// enable compensation cell

// (7) 키트의 주변장치에 클록을 공급
  RCC->AHB1ENR |= 0x0000001F;			// 포트 A,B,C,D,E에 클럭 공급
  RCC->AHB2ENR |= 0x00000000;			// disable special peripherals
  RCC->AHB3ENR |= 0x00000000;			// disable FSMC
  RCC->APB1ENR |= 0x2024C000;			// 주변장치 클럭(DAC=I2C1=USART3=SPI3=SPI2=1)
  RCC->APB2ENR |= 0x00000100;			// 주변장치 클럭(ADC1 = 1)

// (8) 키트에 맞게 GPIO를 초기화
  GPIOA->MODER  = 0xA956FFFF;			// 포트A 설정
  GPIOA->AFR[1] = 0x00000000;			// PA15 = JTDI, PA14 = JTCK, PA13 = JTMS, PA8 = MCO1
  GPIOA->AFR[0] = 0x00000000;			// PA7/6 = ADC12_IN7/6, PA5/4 = DAC_OUT2/1
  GPIOA->ODR = 0x00000000;			// PA3~0 = ADC123_IN3~0
  GPIOA->OSPEEDR = 0xFC030000;			// JTDI = JTCK = JTMS = MCO1 = 180MHz very high speed

  GPIOB->MODER  = 0xA9AAA294;			// 포트B 설정
  GPIOB->AFR[1] = 0x55507799;			// PB15 = SPI2_MOSI, PB14 = SPI2_MISO, PB13 = SPI2_SCK
   						// PB11 = USART3_RX, PB10 = USART3_TX, PB9 = CAN1_TX, PB8 = CAN1_RX
  GPIOB->AFR[0] = 0x44000000;			// PB7 = I2C1_SDA, PB6 = I2C1_SCL, PB4 = NJTRST, PB3 = JTDO
  GPIOB->ODR = 0x00001000;			// -TS_CS = 1
  GPIOB->OTYPER = 0x000000C0;			// PB7(I2C1_SCL) = PB6(I2C1_SDA) = open drain
  GPIOB->OSPEEDR = 0x000000C0;			// JTDO = 180MHz very high speed

  GPIOC->MODER  = 0x46A91500;			// 포트C 설정
  GPIOC->AFR[1] = 0x00066600;			// PC12 = SPI3_MOSI, PC11 = SPI3_MISO, PC10 = SPI3_SCK, PC9 = MCO2
  GPIOC->AFR[0] = 0x00000000;
  GPIOC->ODR = 0x00002160;			// -SD_CS = -MP3_RESET = -MP3_DCS = -MP3_CS = 1, BUZZER = LED = 0
  GPIOC->OSPEEDR = 0x000C0000;			// MCO2 = 180MHz very high speed

  GPIOD->MODER  = 0x55555555;			// 포트D 설정
  GPIOD->ODR = 0x00000000;			// TFT_D15~TFT_D0 = 0
  GPIOD->OSPEEDR = 0x55555555;			// TFT-LCD signal = 50MHz medium speed

  GPIOE->MODER  = 0x6AAA5555;			// 포트E 설정, -RTC_CS = 1, LCD_E = 0
  GPIOE->AFR[1] = 0x05551111;			// PE14 = SPI4_MOSI, PE13 = SPI4_MISO, PE12 = SPI4_SCK
  GPIOE->AFR[0] = 0x00000000;			// PE11/10/9/8 = TIM1_CH2/CH2N/CH1/CH1N
  GPIOE->ODR = 0x0000800D;			// -RTC_CS = -TFT_RESET = -TFT_CS = -TFT_WR = 1
  GPIOE->OSPEEDR = 0x00000455;			// LCD and TFT-LCD signal = 50MHz medium speed
}

void Delay_us(U32 time_us)			/* time delay for us in 216MHz */
{
  register unsigned int i;

  for(i = 0; i < time_us*2; i++)
    { asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 5
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 10
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 15
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 20
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 25
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 30
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 35
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 40
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 45
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 50
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 55
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 60
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 65 
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 70
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 75
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 80
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 85
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 90
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 95
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 100
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 105
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 110
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 115
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 120
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 125
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 130 
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 135
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 140
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 145
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 150
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 155
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 160
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 165
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 170
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 175
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 180
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 185
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 190
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 195
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 200
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 205
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 210
      asm("NOP");
      asm("NOP");
      asm("NOP");				// 213
    }
}

void Delay_ms(U32 time_ms)			/* time delay for ms in 216MHz */
{
  register unsigned int i;

  for(i = 0; i < time_ms; i++)
    Delay_us(1000);
}

void Beep(void)					/* beep for 50 ms */
{ 
  GPIOC->BSRR = 0x00008000;   			// buzzer on
  Delay_ms(50);					// delay 50 ms
  GPIOC->BSRR = 0x80000000;			// buzzer off
}

void Beep_3times(void)				/* beep 3 times */
{ 
  Beep();		   			// beep for 50 ms
  Delay_ms(100);
  Beep();		   			// beep for 50 ms
  Delay_ms(100);
  Beep();		   			// beep for 50 ms
}



unsigned char key_flag = 0;

unsigned char Key_input(void)			/* input key KEY1~KEY4 */
{
  unsigned char key;

  key = GPIOC->IDR & 0x0000000F;		// any key pressed ?
  if(key == 0x0F)				// if no key, check key off
    { if(key_flag == 0)
        return key;
      else
        { Delay_ms(20);
          key_flag = 0;
          return key;
        }
    }
  else						// if key input, check continuous key
    { if(key_flag != 0)				// if continuous key, treat as no key input
        return 0x0F;
      else					// if new key, beep and delay for debounce
        { Beep();
          key_flag = 1;
          return key;
        }
    }
}

unsigned char Key_input_silent(void)		/* input key KEY1~KEY4 without beep sound */
{
  unsigned char key;

  key = GPIOC->IDR & 0x0000000F;		// any key pressed ?
  if(key == 0x0F)				// if no key, check key off
    { if(key_flag == 0)
        return key;
      else
        { Delay_ms(20);
          key_flag = 0;
          return key;
        }
    }
  else						// if key input, check continuous key
    { if(key_flag != 0)				// if continuous key, treat as no key input
        return 0x0F;
      else					// if new key, delay for debounce
        { Delay_ms(20);
          key_flag = 1;
          return key;
        }
    }
}

void LED_on(void)				/* turn LED1 on */
{
  GPIOC->BSRR = 0x00000010;			// LED1 on
}

void LED_off(void)				/* turn LED1 off */
{
  GPIOC->BSRR = 0x00100000;			// LED1 off
}

void LED_toggle(void)				/* toggle LED1 to blink */
{
  GPIOC->ODR ^= 0x00000010;			// toggle LED1
}

/* ---------------------------------------------------------------------------- */
/*		텍스트 LCD 기본 함수						*/
/* ---------------------------------------------------------------------------- */

void Initialize_LCD(void)			/* initialize text LCD module */
{
  RCC->AHB1ENR |= 0x00000018;			// enable clock of port D,E

  GPIOD->MODER &= 0xFFFF0000;			// 포트D 설정
  GPIOD->MODER |= 0x00005555;
  GPIOD->ODR &= 0xFFFFFF00;			// TFT_D7~TFT_D0 = 0
  GPIOD->OSPEEDR &= 0xFFFF0000;			// 50MHz medium speed
  GPIOD->OSPEEDR |= 0x00005555;

  GPIOE->MODER &= 0xFFFFF3F3;			// 포트E 설정
  GPIOE->MODER |= 0x00000404;
  GPIOE->ODR &= 0xFFFFFFDD;			// LCD_E = TFT_RS = 0
  GPIOE->OSPEEDR &= 0xFFFFF3F3;			// 50MHz medium speed
  GPIOE->OSPEEDR |= 0x00000404;

  LCD_command(0x38);				// function set(8 bit, 2 line, 5x7 dot)
  LCD_command(0x0C);				// display control(display ON, cursor OFF)
  LCD_command(0x06);				// entry mode set(increment, not shift)
  LCD_command(0x01);				// clear display
  Delay_ms(2);
}

void LCD_command(U08 command)			/* write a command(instruction) to text LCD */
{
  GPIOE->BSRR = 0x00220000;			// E = 0, Rs = 0
  GPIOD->ODR = command;				// output command
  Delay_us(1);
  GPIOE->BSRR = 0x00000020;			// E = 1
  Delay_us(1);
  GPIOE->BSRR = 0x00200000;			// E = 0
  Delay_us(50);
}

void LCD_data(U08 data)				/* display a character on text LCD */
{
  GPIOE->BSRR = 0x00200002;			// E = 0, Rs = 1
  GPIOD->ODR = data;				// output data
  Delay_us(1);
  GPIOE->BSRR = 0x00000020;			// E = 1
  Delay_us(1);
  GPIOE->BSRR = 0x00200000;			// E = 0
  Delay_us(50);
}

void LCD_string(U08 command, U08 *string)	/* display a string on LCD */
{
  LCD_command(command);				// start position of string
  while(*string != '\0')			// display string
    { LCD_data(*string);
       string++;
    }
}

/* ---------------------------------------------------------------------------- */
/*		텍스트 LCD 모듈의 수치 데이터 출력 함수				*/
/* ---------------------------------------------------------------------------- */

void LCD_binary(U16 number, U08 digit)		/* display 8 or 16 bit binary number */
{
  unsigned char i, shift;

  if((digit == 0) || (digit > 16)) return;

  shift = digit - 1;
  for(i = 0; i < digit; i++ )
    { LCD_data(((number >> shift) & 0x01) + '0');
      shift--;
    }
}

void LCD_unsigned_decimal(U32 number, U08 zerofill, U08 digit) /* display unsigned decimal number */
{
  unsigned char zero_flag, character;
  unsigned long div;

  if((digit == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { character = number / div;
      if((character == 0) && (zero_flag == 0) && (div != 1))
        LCD_data(character + ' ');
      else
        { zero_flag = 1;
          LCD_data(character + '0');
        }
      number %= div;
      div /= 10;
    }
}

void LCD_signed_decimal(S32 number, U08 zerofill, U08 digit)	/* display signed decimal number */
{
  unsigned char zero_flag, character;
  unsigned long div;

  if((digit == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    LCD_data('+');
  else
    { LCD_data('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  zero_flag = zerofill;
  while(div > 0)				// display number
    { character = number / div;
      if((character == 0) && (zero_flag == 0) && (div != 1))
        LCD_data(character + ' ');
      else
        { zero_flag = 1;
          LCD_data(character + '0');
        }
      number %= div;
      div /= 10;
    }
}

void LCD_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number */
{
  unsigned char i, character;

  if((digit == 0) || (digit > 8)) return;

  for(i = digit; i > 0; i--)
    { character = (number >> 4*(i-1)) & 0x0F;
      if(character < 10) LCD_data(character + '0');
      else               LCD_data(character - 10 + 'A');
    }
}

void LCD_0x_hexadecimal(U32 number, U08 digit)	/* display hexadecimal number with 0x */
{
  unsigned char i, character;

  if((digit == 0) || (digit > 8)) return;

  LCD_data('0');
  LCD_data('x');

  for(i = digit; i > 0; i--)
    { character = (number >> 4*(i-1)) & 0x0F;
      if(character < 10) LCD_data(character + '0');
      else               LCD_data(character - 10 + 'A');
    }
}

void LCD_unsigned_float(float number, U08 integral, U08 fractional) /* display unsigned floating-point number */
{
  unsigned char zero_flag, digit, character;	// integral = digits of integral part
  unsigned long div, integer;			// fractional = digits of fractional part

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        LCD_data(character + ' ');
      else
        { zero_flag = 1;
          LCD_data(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        LCD_data('.');
      digit++;
    }
}

void LCD_signed_float(float number, U08 integral, U08 fractional) /* display signed floating-point number */
{
  unsigned char zero_flag, digit, character;
  unsigned long div, integer;

  digit = integral + fractional;
  if((integral == 0) || (fractional == 0) || (digit > 9)) return;

  if(number >= 0)				// display sign
    LCD_data('+');
  else
    { LCD_data('-');
      number = -number;
    }

  div = 1;
  while(--digit) div *= 10;

  while(fractional--) number *= 10.;
  integer = (U32)(number + 0.5);

  zero_flag = 0;
  digit = 1;
  while(div > 0)				// display number
    { character = integer / div;
      if((character == 0) && (zero_flag == 0) && (digit != integral))
        LCD_data(character + ' ');
      else
        { zero_flag = 1;
          LCD_data(character + '0');
        }
      integer %= div;
      div /= 10;

      if(digit == integral)
        LCD_data('.');
      digit++;
    }
}

/* ---------------------------------------------------------------------------- */
/*			DS3234 RTC 관련 함수					*/
/* ---------------------------------------------------------------------------- */

void DS3234_initialize(void)			/* initialize DS3234 RTC */
{
  unsigned char SEC, MIN, HOUR, WEEK, DATE, MON, YEAR;

  RCC->AHB1ENR |= 0x00000010;			// enable clock of port E
  RCC->APB2ENR |= 0x00002000;			// enable SPI4 clock

  GPIOE->MODER &= 0x00FFFFFF;			// alternate function and output mode
  GPIOE->MODER |= 0x6A000000;
  GPIOE->AFR[1] &= 0xF000FFFF;			// PE14 = SPI4_MOSI, PE13 = SPI4_MISO, PE12 = SPI4_SCK
  GPIOE->AFR[1] |= 0x05550000;
  GPIOE->ODR |= 0x00008000;			// -RCS_CS = 1

  SPI4->CR1 = 0x0365;				// master mode, 54MHz/32 = 1.6875MHz (Max 4MHz)
  SPI4->CR2 = 0x0F00;				// 16-bit data, disable SS output, CPOL=0 and CPHA=1

  SEC  =  DS3234_read(0x00);			// if not second, initialize
  MIN  =  DS3234_read(0x01);			// if not minute, initialize
  HOUR =  DS3234_read(0x02);			// if not 12 hour system, initialize
  WEEK =  DS3234_read(0x03);			// if not weekday, initialize
  DATE =  DS3234_read(0x04);			// if not date, initialize
  MON  =  DS3234_read(0x05);			// if not month, initialize
  YEAR =  DS3234_read(0x06);			// if not year, initialize

  if((SEC > 0x59) || (MIN > 0x59) || (HOUR < 0x40) || (WEEK > 0x07) || (DATE > 0x31) || (MON > 0x12) || (YEAR > 0x99))
    { DS3234_write(0x00, 0x00);			// second = 0x00
      DS3234_write(0x01, 0x00);			// minute = 0x00
      DS3234_write(0x02, 0x12 + 0x40);		// hour = 0x12 AM
      DS3234_write(0x03, 0x01);			// weekday = 0x01(Sunday)
      DS3234_write(0x04, 0x01);			// date = 0x01
      DS3234_write(0x05, 0x01);			// month = 0x01
      DS3234_write(0x06, 0x17);			// year = 0x2017
      DS3234_write(0x0E, 0x00);			// control register
      DS3234_write(0x0F, 0x00);			// control register
    }
}

unsigned char DS3234_read(U16 address)		/* read DS3234 */
{
  unsigned short U16;

  U16 = SPI4->DR;				// clear RXNE flag
  GPIOE->BSRR = 0x80000000;			// -RTS_CS = 0
  SPI4->DR = address << 8;
  while((SPI4->SR & 0x0003) != 0x0003);
  U16 = SPI4->DR;
  GPIOE->BSRR = 0x00008000;			// -RTS_CS = 1

  return U16 & 0x00FF;
}

void DS3234_write(U16 address, U08 value)	/* write DS3234 */
{
  unsigned short U16;

  U16 = SPI4->DR;				// clear RXNE flag
  GPIOE->BSRR = 0x80000000;			// -RTS_CS = 0
  SPI4->DR = ((0x80 + address) << 8) + value;
  while((SPI4->SR & 0x0003) != 0x0003);
  U16 |= SPI4->DR;
  GPIOE->BSRR = 0x00008000;			// -RTS_CS = 1
}