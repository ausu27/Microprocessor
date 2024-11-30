#ifndef SD_DISKIO_H
#define SD_DISKIO_H

#include "OK-STM767.h"
#include "diskio.h"

void SPI3_low_speed(void);			/* initialize SPI3(210.9375kHz) */
void SPI3_medium_speed(void);			/* initialize SPI3(1.6875MHz) */
void SPI3_high_speed(void);			/* initialize SPI3(6.75MHz) */
unsigned char SPI3_write(U08 data);		/* send a U08 to SPI3 and receive */
U08 Initialize_SD(void);			/* initialize SD/SDHC card */
void SD_command(U08 command, U32 sector);	/* send SD card command */
void SD_read_sector(U32 sector, U08 *buffer);	/* read a sector of SD card */
void SD_write_sector(U32 sector, const U08 *buffer); /* write a sector to SD card */

U16 SD_get_capacity(void);                      // SD 카드의 용량을 구하는 함수
DRESULT SD_check_status(void);                   // SD 카드가 정상적으로 동작하는지 확인하는 함수. 카드 상태 확인, 읽기/쓰기 작업 할 수 있는지 검사
DRESULT SD_erase_sector(U16 sector);              // SD 카드에서 특정 섹터 지우는 기능. 

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


#endif // SD_DISKIO_H