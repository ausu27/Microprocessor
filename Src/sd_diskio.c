#include "stm32f767xx.h"
#include "sd_diskio.h"
#include "diskio.h"
#include "OK-STM767.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

unsigned char SD_type;				// SD card version(Ver1.X or Ver2.0)

/* ---------------------------------------------------------------------------- */

void SPI3_low_speed(void)			/* initialize SPI3(210.9375kHz) */
{
  GPIOC->MODER &= 0xC00C03FF;			// alternate and input/output mode
  GPIOC->MODER |= 0x06A11400;
  GPIOC->AFR[1] |= 0x00066600;			// PC12 = SPI3_MOSI, PC11 = SPI3_MISO, PC10 = SPI3_SCK
  GPIOC->ODR |= 0x00002160;			// -SD_CS = -MP3_RESET = -MP3_DCS = -MP3_CS = 1
  GPIOC->OSPEEDR |= 0x0AA02800;			// -SD_CS = SPI3 = -MP3_DCS/CS = 100MHz high speed

  RCC->APB1ENR |= 0x00008000;			// enable SPI3 clock

  SPI3->CR1 = 0x037C;				// master mode, 54MHz/256 = 210.9375kHz (Max 400kHz)
  SPI3->CR2 = 0x1700;				// 8-bit data, disable SS output, CPOL = CPHA = 0
}

void SPI3_medium_speed(void)			/* initialize SPI3(1.6875MHz) */
{
  GPIOC->MODER &= 0xC00C03FF;			// alternate and input/output mode
  GPIOC->MODER |= 0x06A11400;
  GPIOC->AFR[1] |= 0x00066600;			// PC12 = SPI3_MOSI, PC11 = SPI3_MISO, PC10 = SPI3_SCK
  GPIOC->ODR |= 0x00002160;			// -SD_CS = -MP3_RESET = -MP3_DCS = -MP3_CS = 1
  GPIOC->OSPEEDR |= 0x0AA02800;			// -SD_CS = SPI3 = -MP3_DCS/CS = 100MHz high speed

  RCC->APB1ENR |= 0x00008000;			// enable SPI3 clock

  SPI3->CR1 = 0x0364;				// master mode, 54MHz/32 = 1.6875MHz
  SPI3->CR2 = 0x1700;				// 8-bit data, disable SS output, CPOL = CPHA = 0
}

void SPI3_high_speed(void)			/* initialize SPI3(6.75MHz) */
{
  GPIOC->MODER &= 0xC00C03FF;			// alternate and input/output mode
  GPIOC->MODER |= 0x06A11400;
  GPIOC->AFR[1] |= 0x00066600;			// PC12 = SPI3_MOSI, PC11 = SPI3_MISO, PC10 = SPI3_SCK
  GPIOC->ODR |= 0x00002160;			// -SD_CS = -MP3_RESET = -MP3_DCS = -MP3_CS = 1
  GPIOC->OSPEEDR |= 0x0AA02800;			// -SD_CS = SPI3 = -MP3_DCS/CS = 100MHz high speed

  RCC->APB1ENR |= 0x00008000;			// enable SPI3 clock

  SPI3->CR1 = 0x0354;				// master mode, 54MHz/8 = 6.75MHz (Max 25MHz)
  SPI3->CR2 = 0x1700;				// 8-bit data, disable SS output, CPOL = CPHA = 0
}

unsigned char SPI3_write(U08 data)		/* send a U08 to SPI3 and receive */
{
  unsigned char U08 = 0;
  unsigned int address;

  U08 |= SPI3->DR;				// clear RXNE flag

  address = (uint8_t)SPI3 + 0x0C;
  *(__IO uint8_t *)address = data;

  while((SPI3->SR & 0x0003) != 0x0003);

  return SPI3->DR;
}

/* ---------------------------------------------------------------------------- */

U08 Initialize_SD(void)			/* initialize SD/SDHC card */
{
  unsigned char i, j;
  unsigned char R1, CMD_flag, R3[5], R7[5];
  unsigned char SD_success = 1;
  
  TFT_string(0,2,White,Magenta,"           (1) SD ??? ????           ");
  Delay_ms(100);

  if(GPIOC->IDR & 0x00004000)
    { TFT_string(10,20,Red,Black,"SD ??? ??????? !");
      Beep_3times();
      while(1);
    }

  SPI3_low_speed();				// initialize SPI3 (below 400kHz)
  GPIOC->BSRR = 0x00002000;			// -SD_CS = 1
  for(i = 0; i < 10; i++) SPI3_write(0xFF);	// set SPI mode(80 clocks)
  Delay_ms(1);

  GPIOC->BSRR = 0x20000000;			// -SD_CS = 0

  TFT_string(4,5,Magenta,Black,"CMD0 :  ");	// second CMD0
  TFT_color(Cyan, Black);
  SD_command(CMD0, 0);				// send CMD0(reset and go to SPI mode)
  CMD_flag = 0;
  for(i = 0; i < 5; i++)			// display R1
    { R1 = SPI3_write(0xFF);
      TFT_hexadecimal(R1,2);
      TFT_English(' ');
      if(R1 == 0x01)
        CMD_flag = 1;
    }
  if(CMD_flag == 1)
    TFT_string(30,5,Green,  Black,"/ OK !");
  else
    { TFT_string(30,5,Red,Black,"/ Error!");
      TFT_string(6,20,Red,Black,"SD ??? ???????? ?????? !");
      TFT_hexadecimal(SPI3->CR2,4);
      Beep_3times();
      while(1);
    }

  GPIOC->BSRR = 0x00002000;			// -SD_CS = 1

  SPI3_high_speed();				// initialize SPI3 (below 25MHz)
  GPIOC->BSRR = 0x20000000;			// -SD_CS = 0

  TFT_string(4,7,Magenta,Black,"CMD8 :  ");
  TFT_color(Cyan, Black);
  SD_command(CMD8, 0x000001AA);			// send CMD8(check version)
  CMD_flag = 0;
  for(i = 0; i < 5; i++)
    { TFT_xy(12,7);
      R7[0] =  SPI3_write(0xFF);
      TFT_hexadecimal(R7[0],2);
      TFT_English(' ');
      if(R7[0] == 0x05)				// if R1 = 0x05, Ver 1.X
        { SD_type = VER1X;
          CMD_flag = 1;
          TFT_string(30,7,Magenta,Black,"/ V1.X");
	  break;
	}
      else if(R7[0] == 0x01)			// if R1 = 0x01, Ver 2.X
        { for(j = 1; j <= 4; j++)
            { R7[j] = SPI3_write(0xFF);
              TFT_hexadecimal(R7[j],2);
              TFT_English(' ');
            }
          if((R7[3] == 0x01) && (R7[4] == 0xAA))
	    { SD_type = VER2X;
	      CMD_flag = 1;
              TFT_string(30,7,Magenta,Black,"/ V2.X");
	      break;
            }
	}
    }
  SPI3_write(0xFF);				// send dummy clocks
  if(CMD_flag == 0)
    { TFT_string(30,7,Red,Black,"/ Error!");
      TFT_string(5,20,Red,Black,"SD card with unknown format !");
      Beep_3times();
      while(1);
    }

  CMD_flag = 0;
  for(i = 0; i < 100; i++)			// send ACMD41(start initialization)
    { TFT_string(4,10,Magenta,Black,"CMD55:  ");
      TFT_color(Cyan, Black);
      SD_command(CMD55, 0);
      Delay_ms(1);
      for(j = 0; j < 5; j++)			// receive R1 = 0x01
        { R1 = SPI3_write(0xFF);
          TFT_hexadecimal(R1,2);
          TFT_English(' ');
        }
      TFT_string(4,12,Magenta,Black,"ACMD41: ");
      TFT_color(Cyan, Black);
      SD_command(ACMD41, 0x40000000);		// HCS = 1
      Delay_ms(1);
      for(j = 0; j < 5; j++)			// receive R1 = 0x00
        { R1 = SPI3_write(0xFF);
          TFT_hexadecimal(R1,2);
          TFT_English(' ');
          if(R1 == 0x00) {
            CMD_flag = 1;
            SD_success = 0;                      /// for diskio.c disk_initialize()
          }  
        }
      if(CMD_flag == 1)
        break;
    }
  if(CMD_flag == 1)
    TFT_string(30,12,Green,  Black,"/ OK !");
  else
    { TFT_string(30,12,Red,Black,"/ Error!");
      TFT_string( 9,20,Red,Black,"SD card is not ready !");
      Beep_3times();
      while(1);
    }

  if(SD_type == VER2X)
    { TFT_string(4,14,Magenta,Black,"CMD58:  "); // send CMD58(check SDHC)
      TFT_color(Cyan, Black);
      SD_command(CMD58, 0);
      for(i = 0; i < 5; i++)
        { TFT_xy(12,14);
          R3[0] =  SPI3_write(0xFF);
          TFT_hexadecimal(R3[0],2);
          TFT_English(' ');
          if(R3[0] == 0x00)
            { for(j = 1; j <= 4; j++)
                { R3[j] = SPI3_write(0xFF);
                  TFT_hexadecimal(R3[j],2);
                  TFT_English(' ');
	        }
              break;
	    }
        }
      if((R3[1] & 0xC0) == 0xC0)		// if CCS = 1, High Capacity(SDHC)
        { SD_type = VER2X_HC;
          TFT_string(30,14,Magenta,Black,"/ SDHC");
        }
      else					// if CCS = 0, Standard Capacity
        { SD_type = VER2X_SC;
          TFT_string(30,14,Magenta,Black,"/ SC");
        }
      SPI3_write(0xFF);				// send dummy clocks
    }

  GPIOC->BSRR = 0x00002000;			// -SD_CS = 1

  TFT_string(13,17,Green,Black,"???? ??? !");
}

void SD_command(U08 command, U32 sector)	/* send SD card command */
{
  unsigned char check;

  SPI3_write(0xFF);				// dummy
  SPI3_write(command | 0x40);			// send command index
  SPI3_write(sector >> 24);			// send 32-bit argument
  SPI3_write(sector >> 16);
  SPI3_write(sector >> 8);
  SPI3_write(sector);

  if(command == CMD0)      check = 0x95;	// CRC for CMD0
  else if(command == CMD8) check = 0x87;	// CRC for CMD8(0x000001AA)
  else                     check = 0xFF;	// no CRC for other CMD

  SPI3_write(check);				// send CRC
}

void SD_read_sector(U32 sector, U08 *buffer)	/* read a sector of SD card */
{
  unsigned int i;

  if((SD_type == VER1X) || (SD_type == VER2X_SC)) // if not SDHC, sector = sector*512
    sector <<= 9;

  GPIOC->BSRR = 0x20000000;			// -SD_CS = 0

  SD_command(CMD17, sector);			// send CMD17(read a block)

  while(SPI3_write(0xFF) != 0x00);		// wait for R1 = 0x00
  while(SPI3_write(0xFF) != 0xFE);		// wait for Start Block Token = 0xFE

  for(i = 0; i < 512; i++)			// receive 512-U08 data
    buffer[i] = SPI3_write(0xFF);

  SPI3_write(0xFF);				// receive CRC(2 U08s)
  SPI3_write(0xFF);
  SPI3_write(0xFF);				// send dummy clocks

  GPIOC->BSRR = 0x00002000;			// -SD_CS = 1
}

void SD_write_sector(U32 sector, const U08 *buffer) /* write a sector to SD card */
{
    unsigned int i;
    unsigned char response;

    if((SD_type == VER1X) || (SD_type == VER2X_SC)) // if not SDHC, sector = sector*512
        sector <<= 9; // convert sector to U08 address if necessary

    GPIOC->BSRR = 0x20000000; // -SD_CS = 0

    SD_command(CMD24, sector); // send CMD24(write a block)

    while(SPI3_write(0xFF) != 0x00); // wait for R1 = 0x00 (acknowledgement)

    SPI3_write(0xFE); // Start Block Token = 0xFE

    // Send 512 U08s data
    for(i = 0; i < 512; i++) {
        SPI3_write(buffer[i]);
    }

    // Wait for the card to finish writing
    response = SPI3_write(0xFF); // Get the response
    if((response & 0x1F) != 0x05) {
        // If response is not 0x05, there was an error writing
        // Handle the error accordingly (e.g., retry or return failure)
    }

    SPI3_write(0xFF); // Receive CRC (2 U08s, discard)
    SPI3_write(0xFF);

    GPIOC->BSRR = 0x00002000; // -SD_CS = 1 (deactivate chip select)
}

// SD ī���� �뷮�� ���ϴ� �Լ�
U16 SD_get_capacity(void) {
    U16 capacity = 0;
    U08 CSD[16]; // CSD �������� ������ �����
    U32 i;

    // SD ī�忡�� CSD �������͸� �б� ���� ��� (CMD9)
    SD_command(CMD9, 0); // CMD9�� CSD �������͸� ��û�ϴ� ���

    // CSD �����͸� ����
    for (i = 0; i < 16; i++) {
        CSD[i] = SPI3_write(0xFF); // CSD ������ �б�
    }

    // CSD���� ī�� �뷮�� ��� (V2.0 �Ǵ� V1.x ī�� ����)
    if (CSD[0] & 0x40) { // SDHC ī������ Ȯ��
        // CSD[8]�� [9]�� ������� �뷮 ���
        capacity = ((U16)(CSD[8] & 0x3F) << 16) | ((U16)(CSD[9]) << 8) | (CSD[10]);
    } else {
        // V1.0, V1.1 ī���� ���
        capacity = ((U16)(CSD[8] & 0x3F) << 16) | ((U16)(CSD[9]) << 8) | (CSD[10]);
    }

    // ���� ũ�⸦ 512����Ʈ�� ����Ͽ� ��ȯ
    return capacity * 1024 / 512; // �뷮(���� ��)�� ��ȯ
}

// SD ī�尡 ���������� �����ϴ��� Ȯ���ϴ� �Լ�. ī�� ���� Ȯ��, �б�/���� �۾� �� �� �ִ��� �˻�
DRESULT SD_check_status(void) {
    U08 status;
    
    // SD ī�� ���¸� Ȯ���ϱ� ���� ��� (CMD13: SEND_STATUS)
    SD_command(CMD13, 0); // ī�� ���¸� ��û�ϴ� ���

    // ���� �� �б�
    status = SPI3_write(0xFF); // ī�� ���� �� �б�

    // ���� Ȯ�� (���⼭�� ���÷� ī�尡 �غ� �������� Ȯ��)
    if (status == 0x00) { // 0x00�� ���� ���¸� ��Ÿ��
        return RES_OK; // ���� ����
    } else {
        return RES_ERROR; // ���� ����
    }
}

// SD ī�忡�� Ư�� ���� ����� ���. 
DRESULT SD_erase_sector(U16 sector) {
    // ���� ���� �÷���
    DRESULT res = RES_ERROR;

    // 1. ��ġ ����
    SD_command(CMD32, sector); // CMD32: ���� ���� ����
    unsigned char spi_result = SPI3_write(0xFF);
    
    if ( spi_result == 0x00 ) {
      res = RES_OK;
    }  
    // ���� ���� ���� ������ �����ϸ� �ٷ� ����
    if ( res == RES_ERROR ) {
      return RES_ERROR;
    }

    // 2. ���� ��ġ ����
    SD_command(CMD33, sector + 1); // CMD33: ���� ���� ����
    spi_result = SPI3_write(0xFF);
    
    // ���� ��ġ ���� ���� ���� Ȯ��
    if ( spi_result != 0x00 ) {
      return RES_ERROR; // ���� ���� ���� ���� �� ���� ��ȯ
    }
    
    // 3. ���� ���� (CMD38)
    SD_command(CMD38, 0); // CMD38: ������ ���� ����

    // ���� �Ϸ� Ȯ��
    while (SPI3_write(0xFF) != 0x00); // ��ٸ�

    return RES_OK; // ���� �Ϸ�
}

