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

U16 SD_get_capacity(void);                      // SD ī���� �뷮�� ���ϴ� �Լ�
DRESULT SD_check_status(void);                   // SD ī�尡 ���������� �����ϴ��� Ȯ���ϴ� �Լ�. ī�� ���� Ȯ��, �б�/���� �۾� �� �� �ִ��� �˻�
DRESULT SD_erase_sector(U16 sector);              // SD ī�忡�� Ư�� ���� ����� ���. 

#define CMD0		0			// GO_IDLE_STATE
#define CMD8		8			// SEND_IF_COND

#define CMD17		17			// READ_SINGLE_BLOCK
#define CMD55		55			// APP_CMD
#define CMD58		58			// READ_OCR
#define CMD24           24                      // Write a block
#define CMD9            9                       // CSD �������� ��û
#define CMD13           13                      // Send Status, ī�� ���� ��û
#define CMD32           32                      // ���� ���� ����
#define CMD33           33                      // ���� ���� ����
#define CMD38           38                      // ������ ���� ����

#define ACMD41		41			// SD_SEND_OP_COND

#define VER1X		10			// Ver1.X SD Memory Card
#define VER2X		20			// Ver2.X SD Memory Card
#define VER2X_SC	21			// Ver2.X Standard Capacity SD Memory Card
#define VER2X_HC	22			// Ver2.X High Capacity SD Memory Card(SDHC)


#endif // SD_DISKIO_H