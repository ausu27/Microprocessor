/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#ifndef __DISKIO_H
#define __DISKIO_H

#include "types.h"
// #include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "OK-STM767.h"
#include "sd_diskio.h"


/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	
	if (pdrv == DEV_MMC) {  // DEV_MMC는 SD 카드를 나타냅니다.
          result = SD_check_status();  // SD 카드 상태 확인
          if (result == RES_OK) {
              stat = 0x00;  // 정상 상태
          } else {
              stat = STA_NOINIT;  // 초기화되지 않음
          }
          return stat;
        }
        return STA_NOINIT;
}
	/*
	switch (pdrv) 
	case DEV_RAM :
		result = RAM_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_MMC :
		result = MMC_disk_status();

		// translate the reslut code here

		return stat;

	case DEV_USB :
		result = USB_disk_status();

		// translate the reslut code here

		return stat;
	
	return STA_NOINIT;
	*/




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	//int result;

	switch (pdrv) {
		case DEV_MMC :  
			Initialize_SD();  // SD 카드 초기화 함수 호출

			// SD 카드 초기화가 성공적으로 완료되면 정상적인 상태 변환
			if ( SD_type != 0 ) {   // SD_type이 0이 아니면 초기화 성공
				stat = 0;           // 성공 상태 
			} else {
				stat = STA_NOINIT;  // 실패 상태
			}
			return stat;
		/*
		case DEV_USB :
			result = USB_disk_initialize();

			// translate the reslut code here

			return stat;
		}
		case DEV_RAM :
			result = RAM_disk_initialize();

			// translate the reslut code here

			return stat;
		*/
	}
	return STA_NOINIT; // 0x01
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	//int result;

	switch (pdrv) {
	case DEV_MMC :
        for ( UINT i = 0; i < count; i++ ) {
            SD_read_sector( sector + i, buff + ( i * 512 ) ); // 512byte씩 읽어서 buffer에 저장
        }
        res = RES_OK; // 읽기 성공 시 RES_OK 반환
        break;

    default:
        res = RES_PARERR;
        break;
    }

		return res;
    /*
	case DEV_USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
    case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
    */
}




/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	//int result;
    UINT i;

	switch (pdrv) {
	case DEV_MMC :
        for ( i = 0; i < count; i++ ) {
            SD_write_sector( sector + i, (const U08*)(buff + ( i * 512 ) ) );
        }
        res = RES_OK; //성공
		return res;    
    /*
	case DEV_USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}
    case DEV_RAM :
		// translate the arguments here

		result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
    */
    }
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
  BYTE pdrv,     /* [IN] 드라이브 번호 */
  BYTE cmd,      /* [IN] 제어 명령 코드 */
  void* buff     /* [I/O] 파라미터와 데이터 버퍼 */
)
{
  DRESULT res = RES_ERROR;  // 기본적으로 오류 반환

  // SD 카드에 대해서만 처리
  if (pdrv == DEV_MMC) {
    switch (cmd) {
      case CTRL_SYNC:
        // 데이터 버퍼를 SD 카드에 동기화하는 명령
        // 실제 구현은 SD 카드에 데이터를 플러시하는 과정이 필요할 수 있음
        res = RES_OK;  // 일단은 성공 처리로 가정
        break;
        
      case GET_SECTOR_COUNT:
        // SD 카드의 총 섹터 수를 구하는 명령
        *(DWORD*)buff = SD_get_capacity();  // SD 카드 용량 함수 호출
        res = RES_OK;
        break;
        
      case GET_SECTOR_SIZE:
        // SD 카드의 섹터 크기를 구하는 명령 (512 바이트 고정)
        *(WORD*)buff = 512;  // SD 카드의 섹터 크기
        res = RES_OK;
        break;
        
      case GET_BLOCK_SIZE:
        // SD 카드의 블록 크기 구하는 명령 (일반적으로 1 블록은 512 바이트)
        *(DWORD*)buff = 1;  // SD 카드 블록 크기
        res = RES_OK;
        break;
        
      case CTRL_ERASE_SECTOR:
        // SD 카드에서 섹터 지우기
        // buff는 지울 섹터 번호를 담고 있는 포인터로 가정
        res = SD_erase_sector(*(DWORD*)buff);  // SD 카드의 특정 섹터 지우는 함수 *(DWORD*)buff는 (DWORD*)buff buff 포인터가 가리키는 주소를 역참조, 데이터 가져옴
        break;
        
      case SD_CARD_STATUS:
        // SD 카드의 상태 확인 (읽기/쓰기 가능한지 확인)
        res = SD_check_status();  // SD 카드의 상태 확인 함수
        break;
      
      default:
        res = RES_PARERR;  // 잘못된 명령 코드
        break;
    }
  } else {
    res = RES_PARERR;  // 잘못된 드라이브 번호
  }
  
  return res;
}
#endif
