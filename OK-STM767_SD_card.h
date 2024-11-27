/* ============================================================================ */
/*	   OK-STM746 V1.0 ŰƮ  (STM32F767VGT6) SD ī           		*/
/* ============================================================================ */
/*					Programmed by Duck-Yong Yoon in 2016.   */

// -------------------------------------------------------------------------------
//                OK-STM767 ŰƮ V1.0           Դϴ .  ̰     Ʒ       ǿ     
//   ùٸ              մϴ .
// -------------------------------------------------------------------------------
//	(1)  ý    Ŭ   : 216MHz
//	(2) C      Ϸ   : IAR EWARM V7.80.2
//	(3)     ȭ  ɼ  : High/Size
//	(4) CSTACK ũ   : 0x1000
//      (5)   Ŭ       :  ⺻          OK-STM767.h    ڿ    Ŭ        .
// -------------------------------------------------------------------------------

/* ============================================================================ */
/*				SD ī         ⺻  Լ 				*/
/* ============================================================================ */
#include "OK-STM767.h"

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
#define CMD24           24                      // Write a block
#define CMD9            9                       // CSD 레지스터 요청
#define CMD13           13                      // Send Status, 카드 상태 요청
#define CMD32           32                      // 시작 섹터 지정
#define CMD33           33                      // 종료 섹터 지정
#define CMD38           38                      // 에리어 삭제 시작

#define ACMD41		41			// SD_SEND_OP_COND

#define VER1X		10			// Ver1.X SD Memory Card
#define VER2X		20			// Ver2.X SD Memory Card
#define VER2X_SC	21			// Ver2.X Standard Capacity SD Memory Card
#define VER2X_HC	22			// Ver2.X High Capacity SD Memory Card(SDHC)




/* ---------------------------------------------------------------------------- */

#define MAX_FILE		250		// maximum file number

#define BYTES_PER_SECTOR	512		// bytes per sector
#define CLUST_FIRST		2		// first legal cluster number
#define CLUST_EOFE		0xFFFFFFFF	// end of eof cluster range
#define FAT16_MASK		0x0000FFFF	// mask for 16 bit cluster numbers
#define FAT32_MASK		0x0FFFFFFF	// mask for FAT32 cluster numbers

/* ---------------------------------------------------------------------------- */

#pragma pack(1)					// byte(8 bit) align


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
