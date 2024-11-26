/* ============================================================================ */
/*	   OK-STM746 V1.0 ŰƮ��(STM32F767VGT6) SD ī�� ��� ����		*/
/* ============================================================================ */
/*					Programmed by Duck-Yong Yoon in 2016.   */

// -------------------------------------------------------------------------------
//   �� ��������� OK-STM767 ŰƮ V1.0�� ���� ���Դϴ�. �̰��� �Ʒ��� ���ǿ�����
//  �ùٸ� ������ �����մϴ�.
// -------------------------------------------------------------------------------
//	(1) �ý��� Ŭ�� : 216MHz
//	(2) C �����Ϸ�  : IAR EWARM V7.80.2
//	(3) ����ȭ �ɼ� : High/Size
//	(4) CSTACK ũ�� : 0x1000
//      (5) ��Ŭ���    : �⺻ ��� ���� OK-STM767.h�� �ڿ� ��Ŭ����� ��.
// -------------------------------------------------------------------------------

/* ============================================================================ */
/*				SD ī�� ���� �⺻ �Լ�				*/
/* ============================================================================ */

void SPI3_low_speed(void);			// initialize SPI3(210.9375kHz)
void SPI3_medium_speed(void);			// initialize SPI3(1.6875MHz)
void SPI3_high_speed(void);			// initialize SPI3(6.75MHz)
unsigned char SPI3_write(U08 data);		// send a byte to SPI3 and receive

void Initialize_SD(void);			// initialize SD card
void SD_command(U08 command, U32 sector);	// send SD card command
void SD_read_sector(U32 sector, U08* buffer);	// read a sector of SD card
void SD_write_sector(U32 sector, const U08 *buffer); // SD 카드 쓰기 함수!!

/* ---------------------------------------------------------------------------- */

#define CMD0		0			// GO_IDLE_STATE
#define CMD8		8			// SEND_IF_COND

#define CMD17		17			// READ_SINGLE_BLOCK
#define CMD55		55			// APP_CMD
#define CMD58		58			// READ_OCR
#define ACMD41		41			// SD_SEND_OP_COND

#define VER1X		10			// Ver1.X SD Memory Card
#define VER2X		20			// Ver2.X SD Memory Card
#define VER2X_SC	21			// Ver2.X Standard Capacity SD Memory Card
#define VER2X_HC	22			// Ver2.X High Capacity SD Memory Card(SDHC)

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

unsigned char SPI3_write(U08 data)		/* send a byte to SPI3 and receive */
{
  unsigned char byte = 0;
  unsigned int address;

  byte |= SPI3->DR;				// clear RXNE flag

  address = (uint32_t)SPI3 + 0x0C;
  *(__IO uint8_t *)address = data;

  while((SPI3->SR & 0x0003) != 0x0003);

  return SPI3->DR;
}

/* ---------------------------------------------------------------------------- */

void Initialize_SD(void)			/* initialize SD/SDHC card */
{
  unsigned char i, j;
  unsigned char R1, CMD_flag, R3[5], R7[5];

  TFT_string(0,2,White,Magenta,"           (1) SD ī�� �ʱ�ȭ           ");
  Delay_ms(100);

  if(GPIOC->IDR & 0x00004000)
    { TFT_string(10,20,Red,Black,"SD ī�尡 �����ϴ� !");
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
      TFT_string(6,20,Red,Black,"SD ī�尡 �������� �ʽ��ϴ� !");
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
          if(R1 == 0x00)
            CMD_flag = 1;
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

  TFT_string(13,17,Green,Black,"�ʱ�ȭ �Ϸ� !");
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

  for(i = 0; i < 512; i++)			// receive 512-byte data
    buffer[i] = SPI3_write(0xFF);

  SPI3_write(0xFF);				// receive CRC(2 bytes)
  SPI3_write(0xFF);
  SPI3_write(0xFF);				// send dummy clocks

  GPIOC->BSRR = 0x00002000;			// -SD_CS = 1
}

void SD_write_sector(U32 sector, const U08 *buffer) /* write a sector to SD card */
{
    unsigned int i;
    unsigned char response;

    if((SD_type == VER1X) || (SD_type == VER2X_SC)) // if not SDHC, sector = sector*512
        sector <<= 9; // convert sector to byte address if necessary

    GPIOC->BSRR = 0x20000000; // -SD_CS = 0

    SD_command(CMD24, sector); // send CMD24(write a block)

    while(SPI3_write(0xFF) != 0x00); // wait for R1 = 0x00 (acknowledgement)

    SPI3_write(0xFE); // Start Block Token = 0xFE

    // Send 512 bytes data
    for(i = 0; i < 512; i++) {
        SPI3_write(buffer[i]);
    }

    // Wait for the card to finish writing
    response = SPI3_write(0xFF); // Get the response
    if((response & 0x1F) != 0x05) {
        // If response is not 0x05, there was an error writing
        // Handle the error accordingly (e.g., retry or return failure)
    }

    SPI3_write(0xFF); // Receive CRC (2 bytes, discard)
    SPI3_write(0xFF);

    GPIOC->BSRR = 0x00002000; // -SD_CS = 1 (deactivate chip select)
}

// SD 카드의 용량을 구하는 함수
DWORD SD_get_capacity(void) {
    DWORD capacity = 0;
    BYTE CSD[16]; // CSD 레지스터 데이터 저장용
    UINT i;

    // SD 카드에서 CSD 레지스터를 읽기 위한 명령 (CMD9)
    SD_command(CMD9, 0); // CMD9는 CSD 레지스터를 요청하는 명령

    // CSD 데이터를 수신
    for (i = 0; i < 16; i++) {
        CSD[i] = SPI3_write(0xFF); // CSD 데이터 읽기
    }

    // CSD에서 카드 용량을 계산 (V2.0 또는 V1.x 카드 구분)
    if (CSD[0] & 0x40) { // SDHC 카드인지 확인
        // CSD[8]과 [9]를 기반으로 용량 계산
        capacity = ((DWORD)(CSD[8] & 0x3F) << 16) | ((DWORD)(CSD[9]) << 8) | (CSD[10]);
    } else {
        // V1.0, V1.1 카드의 경우
        capacity = ((DWORD)(CSD[8] & 0x3F) << 16) | ((DWORD)(CSD[9]) << 8) | (CSD[10]);
    }

    // 섹터 크기를 512바이트로 계산하여 반환
    return capacity * 1024 / 512; // 용량(섹터 수)을 반환
}

// SD 카드가 정상적으로 동작하는지 확인하는 함수. 카드 상태 확인, 읽기/쓰기 작업 할 수 있는지 검사
DRESULT SD_check_status(void) {
    BYTE status;
    
    // SD 카드 상태를 확인하기 위한 명령 (CMD13: SEND_STATUS)
    SD_command(CMD13, 0); // 카드 상태를 요청하는 명령

    // 상태 값 읽기
    status = SPI3_write(0xFF); // 카드 상태 값 읽기

    // 상태 확인 (여기서는 예시로 카드가 준비 상태인지 확인)
    if (status == 0x00) { // 0x00은 정상 상태를 나타냄
        return RES_OK; // 정상 상태
    } else {
        return RES_ERROR; // 오류 상태
    }
}

// SD 카드에서 특정 섹터 지우는 기능. 
DRESULT SD_erase_sector(DWORD sector) {
    // 에러 상태 플래그
    DRESULT res = RES_ERROR;

    // 1. 위치 지정
    SD_command(CMD32, sector); // CMD32: 시작 섹터 지정
    res = SPI3_write(0xFF); // 상태 확인

    if (res != 0x00) {
        return RES_ERROR; // 오류 발생
    }

    // 2. 종료 위치 지정
    SD_command(CMD33, sector + 1); // CMD33: 종료 섹터 지정
    res = SPI3_write(0xFF); // 상태 확인

    if (res != 0x00) {
        return RES_ERROR; // 오류 발생
    }

    // 3. 삭제 시작 (CMD38)
    SD_command(CMD38, 0); // CMD38: 에리어 삭제 시작

    // 삭제 완료 확인
    while (SPI3_write(0xFF) != 0x00); // 기다림

    return RES_OK; // 삭제 완료
}




/* ============================================================================ */
/*			SD ī���� FAT32 ���� �Լ�				*/
/* ============================================================================ */

void Initialize_FAT32(void);			// initialize FAT32
unsigned char fatGetDirEntry(U32 cluster);	// get directory entry
unsigned char *fatDir(U32 cluster, U32 offset);	// get directory entry sector
unsigned int  fatNextCluster(U32 cluster);	// get next cluster
unsigned int  fatClustToSect(U32 cluster);	// convert cluster to sector

unsigned char Get_long_filename(U08 file_number); // check long file name
void TFT_short_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback);
void TFT_long_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback);
unsigned short Unicode_to_KS(U16 unicode);	// convert Unicode(�����ڵ�) to KS(������)
void Filename_arrange(U16 new, U16 old, U08 *FileNameBuffer); // arrange file name

/* ---------------------------------------------------------------------------- */

#define MAX_FILE		250		// maximum file number

#define BYTES_PER_SECTOR	512		// bytes per sector
#define CLUST_FIRST		2		// first legal cluster number
#define CLUST_EOFE		0xFFFFFFFF	// end of eof cluster range
#define FAT16_MASK		0x0000FFFF	// mask for 16 bit cluster numbers
#define FAT32_MASK		0x0FFFFFFF	// mask for FAT32 cluster numbers

unsigned int   file_start_cluster[MAX_FILE];	// file start cluster
unsigned int   file_start_sector[MAX_FILE];	// file start sector number 
unsigned char  file_start_entry[MAX_FILE];	// file directory entry
unsigned int   file_size[MAX_FILE];		// file size

unsigned char  SectorBuffer[512];		// sector buffer
unsigned char  EntryBuffer[512];		// directory entry buffer

unsigned int   sector_cluster = 0;
unsigned int   extension = 0;			// filename extension string(3 character)

unsigned char  Fat32Enabled;			// indicate if is FAT32 or FAT16
unsigned int   FirstDataSector;			// first data sector address
unsigned short SectorsPerCluster;		// number of sectors per cluster
unsigned int   FirstFATSector;			// first FAT sector address
unsigned int   FirstFAT2Sector;			// first FAT2 sector address
unsigned int   FirstDirCluster;			// first directory(data) cluster address

/* ---------------------------------------------------------------------------- */

#pragma pack(1)					// byte(8 bit) align

struct MBR					/* MBR(Master Boot Record) */
{
  unsigned char	 mbrBootCode[512-64-2];		// boot code
  unsigned char	 mbrPartition[64];		// four partition records (64 = 16*4)
  unsigned char	 mbrSignature0;			// signature byte 0 (0x55)
  unsigned char	 mbrSignature1;			// signature byte 1 (0xAA)
};

struct partition				/* partition table(16 byte) of MBR */
{
  unsigned char	 partBootable;			// 0x80 indicates active bootable partition
  unsigned char	 partStartHead;			// starting head for partition
  unsigned short partStartCylSect;		// starting cylinder and sector
  unsigned char	 partPartType;			// partition type
  unsigned char	 partEndHead;			// ending head for this partition
  unsigned short partEndCylSect;		// ending cylinder and sector
  unsigned int	 partStartLBA;			// first LBA sector for this partition
  unsigned int	 partSize;			// size of this partition(bytes or sectors)
};

#define PART_TYPE_FAT16		0x04		// partition type
#define PART_TYPE_FAT16BIG	0x06
#define PART_TYPE_FAT32		0x0B
#define PART_TYPE_FAT32LBA	0x0C
#define PART_TYPE_FAT16LBA	0x0E

struct bootrecord				/* PBR(Partition Boot Record) */
{
  unsigned char	brJumpBoot[3];			// jump instruction E9xxxx or EBxx90
  char		brOEMName[8];			// OEM name and version
  char		brBPB[53];			// volume ID(= BIOS parameter block)
  char		brExt[26];			// Bootsector Extension
  char		brBootCode[418];		// pad so structure is 512 bytes
  unsigned char	brSignature2;			// 2 & 3 are only defined for FAT32
  unsigned char	brSignature3;
  unsigned char	brSignature0;			// boot sector signature byte 0(0x55)
  unsigned char	brSignature1;			// boot sector signature byte 1(0xAA)
};

struct volumeID					/* BPB for FAT16 and FAT32 */
{
					// same at FAT16 and FAT32
  unsigned short bpbBytesPerSec;		// bytes per sector
  unsigned char	 bpbSecPerClust;		// sectors per cluster
  unsigned short bpbResSectors;			// number of reserved sectors
  unsigned char	 bpbFATs;			// number of FATs
  unsigned short bpbRootDirEnts;		// number of root directory entries(0 = FAT32)
  unsigned short bpbSectors;			// total number of sectors(0 = FAT32)
  unsigned char	 bpbMedia;			// media descriptor
  unsigned short bpbFATsecs;			// number of sectors per FAT16(0 = FAT32)
  unsigned short bpbSecPerTrack;		// sectors per track
  unsigned short bpbHeads;			// number of heads
  unsigned int	 bpbHiddenSecs;			// number of hidden sectors
  unsigned int	 bpbHugeSectors;		// number of sectors for FAT32
					// only at FAT32
  unsigned int	 bpbBigFATsecs;			// number of sectors per FAT32
  unsigned short bpbExtFlags;			// extended flags
  unsigned short bpbFSVers;			// file system version
  unsigned int	 bpbRootClust;			// start cluster for root directory
  unsigned short bpbFSInfo;			// file system info structure sector
  unsigned short bpbBackup;			// backup boot sector
  unsigned char	 bpbReserved[12];
					// same at FAT16 and FAT32
  unsigned char	 bpbDriveNum;			// INT 0x13 drive number
  unsigned char	 bpbReserved1;
  unsigned char	 bpbBootSig;			// extended boot signature(0x29)
  unsigned char	 bpbVolID[4];			// extended
  unsigned char	 bpbVolLabel[11];		// extended
  unsigned char	 bpbFileSystemType[8];		// extended
};

typedef struct Dir_entry			/* structure of DOS directory entry */
{
  unsigned char	 dirName[8];      		// filename, blank filled
  unsigned char	 dirExtension[3]; 		// extension, blank filled
  unsigned char	 dirAttributes;   		// file attributes
  unsigned char	 dirLowerCase;    		// NT VFAT lower case flags
  unsigned char	 dirCHundredth;			// hundredth of seconds in CTime
  unsigned char	 dirCTime[2];			// create time
  unsigned char	 dirCDate[2];			// create date
  unsigned char	 dirADate[2];			// access date
  unsigned short dirHighClust;			// high bytes of cluster number
  unsigned char	 dirMTime[2];			// last update time
  unsigned char	 dirMDate[2];			// last update date
  unsigned short dirStartCluster;		// starting cluster of file
  unsigned int	 dirFileSize;			// size of file in bytes
} Dir_entry;

#define 	SLOT_EMPTY      	0x00	// slot has never been used
#define 	SLOT_DELETED    	0xE5	// file in this slot deleted

#define 	ATTR_NORMAL     	0x00	// normal file
#define 	ATTR_READONLY   	0x01	// file is read-only
#define 	ATTR_HIDDEN     	0x02	// file is hidden
#define 	ATTR_SYSTEM     	0x04	// file is a system file
#define 	ATTR_VOLUME     	0x08	// entry is a volume label
#define 	ATTR_LONG_FILENAME	0x0F	// this is a long filename entry
#define 	ATTR_DIRECTORY  	0x10	// entry is a directory name
#define 	ATTR_ARCHIVE    	0x20	// file is new or modified

#define 	LCASE_BASE      	0x08	// filename base in lower case
#define 	LCASE_EXT       	0x10	// filename extension in lower case

typedef struct Long_dir_entry			/* structure of long directory entry */
{
  unsigned char  Longdir_Ord;			// 00(0x00)
  unsigned char	 Longdir_Name1[10];		// 01(0x01)
  unsigned char	 Longdir_Attr;			// 11(0x0B)
  unsigned char	 Longdir_Type;			// 12(0x0C)
  unsigned char	 Longdir_Chksum;		// 13(0x0D)
  unsigned char	 Longdir_Name2[12];		// 14(0x0E)
  unsigned short Longdir_FstClusLO;		// 26(0x1A)
  unsigned char	 Longdir_Name3[4];		// 28(0x1C)
} Long_dir_entry;

#pragma pack()					// word(32 bit) align

/* ---------------------------------------------------------------------------- */
/*			FAT32 ������ Ȯ���ϰ� �ʱ�ȭ				*/
/* ---------------------------------------------------------------------------- */

void Initialize_FAT32(void)			/* initialize FAT32 */
{
  struct volumeID *BPB;
  struct partition PartInfo;

  TFT_string(0,21,White,Magenta,"            (2) FAT32 �ʱ�ȭ            ");

  SD_read_sector(0, SectorBuffer);		// read partition table
  PartInfo = *((struct partition *) ((struct MBR *) SectorBuffer)->mbrPartition);

  SD_read_sector(PartInfo.partStartLBA, SectorBuffer);	// read start LBA sector
  BPB = (struct volumeID *) ((struct bootrecord *) SectorBuffer)->brBPB;
  FirstDataSector = PartInfo.partStartLBA;	// setup global disk constants

  if(BPB->bpbFATsecs)
    FirstDataSector += BPB->bpbResSectors + BPB->bpbFATs * BPB->bpbFATsecs;
  else
    FirstDataSector += BPB->bpbResSectors + BPB->bpbFATs * BPB->bpbBigFATsecs;

  SectorsPerCluster = BPB->bpbSecPerClust;
  FirstFATSector = BPB->bpbResSectors + PartInfo.partStartLBA;

  switch(PartInfo.partPartType)
    { case PART_TYPE_FAT32:			// FAT32
      case PART_TYPE_FAT32LBA:
			FirstDirCluster = BPB->bpbRootClust;
			Fat32Enabled = 1;
                        TFT_string(13,24,Green,Black,"�ʱ�ȭ �Ϸ� !");
			break;

      case PART_TYPE_FAT16:			// FAT16
      case PART_TYPE_FAT16BIG:
      case PART_TYPE_FAT16LBA:
			FirstDirCluster	= CLUST_FIRST;
			Fat32Enabled = 0;
			TFT_string(2,25,Red,Black,"�� FAT32 ���� �ý����� FAT16 ������");
			TFT_string(5,27,Red,Black,"SD ī�带 �������� �ʽ��ϴ� !");
			Beep_3times();
			while(1);

      default:		TFT_string(9,25,Red,Black,"�� FAT32 ���� �ý�����");
			TFT_string(0,27,Red,Black,"�� SD ī���� ������ �������� �ʽ��ϴ� !");
			Beep_3times();
			while(1);
    }

  if(Fat32Enabled == 1)
    FirstFAT2Sector = FirstFATSector + BPB->bpbBigFATsecs;
  else
    FirstFAT2Sector = FirstFATSector + BPB->bpbFATsecs;
}

/* ---------------------------------------------------------------------------- */
/*	�� ���丮 ��Ʈ������ Ŭ������ �ѹ��� �����ϰ� �� ���ϼ� �ľ�		*/
/* ---------------------------------------------------------------------------- */

unsigned char fatGetDirEntry(U32 cluster)	/* get directory entry */
{
  unsigned char index, files = 0;
  unsigned int  cluster_number, offset = 0;

  Dir_entry      *pDirentry;
  Long_dir_entry *pLDirentry;

  for(offset = 0; ; offset++)
    { pDirentry = (Dir_entry *)fatDir(cluster, offset);

      if(pDirentry == 0)
	return files;
      for(index = 0; index < 16; index++)	// 16 direntries
	{ if(*pDirentry->dirName == SLOT_EMPTY)
	    break;
	  else
	    { if((*pDirentry->dirName != SLOT_DELETED) && (pDirentry->dirAttributes == ATTR_LONG_FILENAME))
		{ pLDirentry = (Long_dir_entry *)pDirentry;
		  if(((pLDirentry->Longdir_Ord & 0x40) == 0x40) || ((pLDirentry->Longdir_Ord & 0x50) == 0x50))
		    { file_start_sector[files] = sector_cluster;
		      file_start_entry[files] = index;
		    }
		}

	      if((*pDirentry->dirName != SLOT_DELETED) && (pDirentry->dirAttributes != ATTR_LONG_FILENAME) && (pDirentry->dirAttributes != ATTR_VOLUME) && (pDirentry->dirAttributes != ATTR_DIRECTORY))
		{ if(((pDirentry->dirLowerCase & 0x18) == 0x18) || ((pDirentry->dirLowerCase & 0x18) == 0x10))
		    { file_start_sector[files] = sector_cluster;
		      file_start_entry[files] = index;	
		    }

		  cluster_number = (unsigned int)(pDirentry->dirHighClust);
		  cluster_number <<= 16;
		  cluster_number |= (unsigned int)(pDirentry->dirStartCluster);

		  file_size[files] = pDirentry->dirFileSize;

		  file_start_cluster[files++] = cluster_number;

		  if(files > MAX_FILE)
		    { TFT_string(9,25,Red,Black,"�� FAT32 ���� �ý�����");
		      TFT_string(3,27,Red,Black,"200�� ������ ���ϸ��� �����մϴ� !");
		      Beep_3times();
		      while(1);
		    }
		}
	      pDirentry++;
  	    }
	}					// end of sector
    }						// end of cluster
}

/* ---------------------------------------------------------------------------- */
/*	Ŭ�����Ϳ��� ���丮 ��Ʈ�� ������ �������� ���� ���͸� ����		*/
/* ---------------------------------------------------------------------------- */

unsigned char *fatDir(U32 cluster, U32 offset)	/* get directory entry sector */
{
  unsigned int index;

  for(index = 0; index < offset/SectorsPerCluster; index++)
    cluster = fatNextCluster(cluster);

  if(cluster == 0)
    return 0;

  sector_cluster =  fatClustToSect(cluster) + offset % SectorsPerCluster;
  SD_read_sector(sector_cluster, SectorBuffer);

  return SectorBuffer;
}

/* ---------------------------------------------------------------------------- */
/*		FAT ü������ ���� Ŭ�����͸� ã��				*/
/* ---------------------------------------------------------------------------- */

unsigned int fatNextCluster(U32 cluster)	/* get next cluster */
{
  unsigned int   nextCluster, fatOffset, fatMask, sector;
  unsigned short offset;
  unsigned char  FAT_cache[512];

  if(Fat32Enabled == 1)
    { fatOffset = cluster << 2;			// four FAT bytes(32 bits) for every cluster
      fatMask = FAT32_MASK;
    }
  else
    { fatOffset = cluster << 1;			// two FAT bytes(16 bits) for every cluster
      fatMask = FAT16_MASK;
    }

  sector = FirstFATSector + (fatOffset/BYTES_PER_SECTOR); // calculate the FAT sector

  offset = fatOffset % BYTES_PER_SECTOR;	// calculate offset of entry in FAT sector

  SD_read_sector(sector,(unsigned char *)FAT_cache);

  nextCluster = (*((unsigned int *) &((char *)FAT_cache)[offset])) & fatMask;

  if(nextCluster == (CLUST_EOFE & fatMask))	// check the end of the chain
    nextCluster = 0;

  return nextCluster;
}

/* ---------------------------------------------------------------------------- */
/*		Ŭ������ ��ȣ�� ���� ��ȣ�� ��ȯ				*/
/* ---------------------------------------------------------------------------- */

unsigned int fatClustToSect(U32 cluster)	/* convert cluster to sector */
{
  if(cluster == 0)
    cluster = 2;

  return ((cluster-2)*SectorsPerCluster) + FirstDataSector;
}

/* ---------------------------------------------------------------------------- */
/*		�� ���ϸ��� ����(13*15=195�ڱ��� ���)				*/
/* ---------------------------------------------------------------------------- */

unsigned char Get_long_filename(U08 file_number) /* check long file name */
{
  unsigned short i, n = 0;
  unsigned short dir_address, dir_number = 0;
  unsigned int	 dir_sector = 0;

  unsigned char ShortFileName[11];		// short file name buffer

  SD_read_sector(file_start_sector[file_number], SectorBuffer);	// read directory entry
  dir_address = file_start_entry[file_number]*32;

  if(SectorBuffer[dir_address+11] == 0x0F)	// *** long file name entry
    { dir_number = (SectorBuffer[dir_address] & 0x1F) + 1; // total number of directory entry

      if(dir_number > 16)			// if file name character > 13*15, return 2
        return 2;

      for( ; dir_address < 512; dir_address++)
	EntryBuffer[n++] = SectorBuffer[dir_address];

      if((dir_number*32) >= n)			// directory entry use 2 sectors
	{ dir_sector = file_start_sector[file_number];
	  SD_read_sector(++dir_sector, SectorBuffer);

	  i = 0;
	  for( ; n < 512; n++)
	    EntryBuffer[n] = SectorBuffer[i++];
	}

      dir_number = (EntryBuffer[0] & 0x1F)*32;	// last entry is short file name entry
      for(n = 0; n < 11; n++)
        ShortFileName[n] = EntryBuffer[dir_number++];

      extension = ShortFileName[8];		// get filename extension character
      extension <<= 8;
      extension += ShortFileName[9];
      extension <<= 8;
      extension += ShortFileName[10];
      return 1;					// if long file name, return 1
    }

  else if(SectorBuffer[dir_address+11] == 0x20)	// *** short file name entry
    { for( ; dir_address < 512; dir_address++)
	EntryBuffer[n++] = SectorBuffer[dir_address];

      for(n = 0; n < 11; n++)
        ShortFileName[n] = EntryBuffer[dir_number++];

      extension = ShortFileName[8];		// get filename extension character
      extension <<= 8;
      extension += ShortFileName[9];
      extension <<= 8;
      extension += ShortFileName[10];
      return 0;					// if short file name, return 0
    }

  else						// if file name error, return 3
    return 3;
}

/* ---------------------------------------------------------------------------- */
/*		ª�� ���ϸ� ���						*/
/* ---------------------------------------------------------------------------- */

void TFT_short_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback) /* display short filename */
{
  unsigned char  ch1, i;
  unsigned short ch2;

  Xcharacter = xchar;				// start position
  Ycharacter = ychar;

  foreground = colorfore;			// foreground color and background color
  background = colorback;

  for(i = 0; i < 11; i++ )			// convert upper case to lower case
    { if((EntryBuffer[i] >= 'A') && (EntryBuffer[i] <= 'Z'))
        EntryBuffer[i] += 0x20;	
    }

  for(i = 0; i < 11; i++)			// display 8.3 format
    { if(i == 8)
        TFT_English('.');

      ch1= EntryBuffer[i];
      if(ch1 < 0x80)				// English ASCII character
        {if(ch1 != 0x20)
	   TFT_English(ch1);
	}
      else					// Korean character
	{ ch2 = (ch1 << 8) + EntryBuffer[++i];
	  ch2 = KS_code_conversion(ch2);
	  TFT_Korean(ch2);
	}
    }
}

/* ---------------------------------------------------------------------------- */
/*		�� ���ϸ� ���(40�ڱ��� ǥ��)					*/
/* ---------------------------------------------------------------------------- */

void TFT_long_filename(U08 xchar,U08 ychar, U16 colorfore,U16 colorback) /* display long filename */
{
  unsigned char  ch1;
  unsigned short ch2, i, entrynumber, charnumber, newindex, oldindex;
  unsigned char  FileNameBuffer[512];

  Xcharacter = xchar;				// start position
  Ycharacter = ychar;

  foreground = colorfore;			// foreground color and background color
  background = colorback;

  entrynumber = EntryBuffer[0] & 0x1F;		// long entry number
  charnumber = (EntryBuffer[0] & 0x1F)*13;	// character number of file name
  oldindex = ((EntryBuffer[0] & 0x1F) - 0x01)*32;
  newindex = 0;

  for(i = 0; i < entrynumber; i++)		// arrange file name
    { Filename_arrange(newindex,oldindex,FileNameBuffer);
      newindex += 26;
      oldindex -= 32;
    }

  for(i = 0; i < charnumber*2; i++)		// display long file name format
    { if(i >= 40*2)				// limit length(40 character)
	{ TFT_English('~');
          TFT_English('1');
          return;
	}

      ch1 = FileNameBuffer[i];
      if(ch1 == 0xFF)
	return;

      if(ch1 < 0x80)				// English ASCII character
	TFT_English(FileNameBuffer[++i]);
      else					// Korean character
	{ ch2 = (ch1 << 8) + FileNameBuffer[++i];
	  ch2 = Unicode_to_KS(ch2);
	  TFT_Korean(ch2);
	}
    }
}

/* ---------------------------------------------------------------------------- */
/*		���ϸ� ���ڵ��� ������� �����Ͽ� ��迭			*/
/* ---------------------------------------------------------------------------- */

void Filename_arrange(U16 newx, U16 oldx, U08 *FileNameBuffer) /* arrange file name */
{
  FileNameBuffer[newx+0]  = EntryBuffer[oldx+2];  // 1
  FileNameBuffer[newx+1]  = EntryBuffer[oldx+1];
  FileNameBuffer[newx+2]  = EntryBuffer[oldx+4];  // 2
  FileNameBuffer[newx+3]  = EntryBuffer[oldx+3];
  FileNameBuffer[newx+4]  = EntryBuffer[oldx+6];  // 3
  FileNameBuffer[newx+5]  = EntryBuffer[oldx+5];
  FileNameBuffer[newx+6]  = EntryBuffer[oldx+8];  // 4
  FileNameBuffer[newx+7]  = EntryBuffer[oldx+7];
  FileNameBuffer[newx+8]  = EntryBuffer[oldx+10]; // 5
  FileNameBuffer[newx+9]  = EntryBuffer[oldx+9];
  FileNameBuffer[newx+10] = EntryBuffer[oldx+15]; // 6
  FileNameBuffer[newx+11] = EntryBuffer[oldx+14];
  FileNameBuffer[newx+12] = EntryBuffer[oldx+17]; // 7
  FileNameBuffer[newx+13] = EntryBuffer[oldx+16];
  FileNameBuffer[newx+14] = EntryBuffer[oldx+19]; // 8
  FileNameBuffer[newx+15] = EntryBuffer[oldx+18];
  FileNameBuffer[newx+16] = EntryBuffer[oldx+21]; // 9
  FileNameBuffer[newx+17] = EntryBuffer[oldx+20];
  FileNameBuffer[newx+18] = EntryBuffer[oldx+23]; // 10
  FileNameBuffer[newx+19] = EntryBuffer[oldx+22];
  FileNameBuffer[newx+20] = EntryBuffer[oldx+25]; // 11
  FileNameBuffer[newx+21] = EntryBuffer[oldx+24];
  FileNameBuffer[newx+22] = EntryBuffer[oldx+29]; // 12
  FileNameBuffer[newx+23] = EntryBuffer[oldx+28];
  FileNameBuffer[newx+24] = EntryBuffer[oldx+31]; // 13
  FileNameBuffer[newx+25] = EntryBuffer[oldx+30];
}

/* ---------------------------------------------------------------------------- */
/*		�ѱ� �����ڵ带 KS ������ �ڵ�� ��ȯ				*/
/* ---------------------------------------------------------------------------- */

unsigned short Unicode_to_KS(U16 unicode)	/* convert Unicode(�����ڵ�) to KS(������) */
{
  unsigned char  cho = 0, joong = 0, jong = 0;
  unsigned short value;

  value = unicode - 0xAC00;                	// �����ڵ忡�� '��'�� �ش��ϴ� ���� ����.

  jong  = value % 28;				// �����ڵ带 �ʼ�, �߼�, �������� �и�
  joong = ((value - jong) / 28 ) % 21;
  cho   = ((value - jong) / 28 ) / 21;

  cho += 2;                           	 	// �ʼ� + ������

  if(joong < 5)       joong += 3;		// �߼� + ������
  else if(joong < 11) joong += 5;
  else if(joong < 17) joong += 7;
  else                joong += 9;

  if(jong < 17) jong++;				// ���� + ������
  else          jong += 2;

  return 0x8000 | (cho << 10) | ( joong << 5) | jong; // ������ �ڵ�
}
