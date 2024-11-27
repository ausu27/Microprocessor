#include "stm32f767xx.h"
// #include "OK-STM767.h"
#include "ff.h"
// #include "diskio.h"
// #include "sd_diskio.h"


#define DIRECTORY_NAME "recordings"
#define FILENAME_PREFIX "record_"
#define FILENAME_EXTENSION ".txt"

void EXTI0_IRQHandler(void); /* EXTI0 interrupt function */
void EXTI1_IRQHandler(void); /* EXTI1 interrupt function */
void create_new_file(void);
FRESULT create_directory(void);
unsigned int get_next_file_index(void);

short ADC1_data_buffer[256];
volatile unsigned int interrupt_flag_key1, interrupt_flag_key2;
unsigned int i = 0;
unsigned int ADC_count, bytesWritten;
static int error_count = 0;
FIL file;
FATFS FatFs; // 파일시스템 초기화할때 쓸 거임. ff.h의 구조체임.

//FRESULT res; 이걸 굳이 전역변수로?
//FRESULT res_open, res_write;

void EXTI0_IRQHandler(void) /* EXTI0 interrupt function */
{
  RCC->APB2ENR |= 0x00000001; // enable TIM1 clock  
 
  EXTI->PR = 0x00000001; // clear pending bit of EXTI0
  interrupt_flag_key1 = 1;
}

void EXTI1_IRQHandler(void) /* EXTI1 interrupt function */
{
  RCC->APB2ENR &= 0xFFFFFFFE; // disable TIM1 clock
 
  EXTI->PR = 0x00000002; // clear pending bit of EXTI1
  interrupt_flag_key2 = 1;
}


void TIM1_UP_TIM10_IRQHandler(void) /* TIM1 interrupt function (27MHz)*/
{
  TIM1->SR = 0x0000; // clear pending bit of TIM1 interrupt
 
  ADC_count++;
  if( ADC_count >= 612 ) // 1초에 44000번 ADC 샘플링하기 위해서.
  {
   ADC_count = 0;
   if ( i >= 255 )
    {
      i = 0;
      if ( f_write(&file, ADC1_data_buffer, sizeof(ADC1_data_buffer), &bytesWritten) != FR_OK ) {
        error_count++;
        TFT_string( 10, 10, White, Black, "파일 쓰기 실패" );
        if ( error_count >= 3){
            TFT_string( 10, 10, White, Black, "에러 3번 이상 발생. 리셋하시오." );
            error_count = 0;
        }
      } ; // 데이터 기록
      memset(ADC1_data_buffer, 0, sizeof(ADC1_data_buffer)); //ADC1_data_buffer[ 255 ] = { 0, };
    }
    else
    {  
      ADC1->CR2 |= 0x40000000; // start conversion by software
      while(!(ADC1->SR & 0x00000002)); // wait for end of conversion ///////// 480+12clk = 492clk .걸림
      ADC1_data_buffer[ i++ ] = ADC1->DR;
     }
    }
}




int main( void )
{
  //////KEY_INTERRUPT/////////////////
  GPIOC->MODER &= 0xFFFFFF00; // PC3,2,1,0 = input mode
  RCC->APB2ENR |= 0x00000001; // enable SYSCFG
  SYSCFG->EXTICR[0] = 0x00002222; // EXTI3,2,1,0 = PC3,2,1,0(KEY4,3,2,1)
  EXTI->IMR = 0x0000000F; // enable EXTI3,2,1,0 interrupt
  EXTI->RTSR = 0x00000000; // disable EXTI3,2,1,0 rising edge
  EXTI->FTSR = 0x0000000F; // enable EXTI3,2,1,0 falling edge
  NVIC->ISER[0] = 0x000003C0; // enable (9)EXTI3 ~ (6)EXTI0 interrupt
  interrupt_flag_key1 = 0;
  interrupt_flag_key2 = 0;

  //////TIMER1///////////////////
  //RCC->APB2ENR |= 0x00000001; // enable TIM1 clock
  TIM1->PSC = 1; // 108MHz/( 1+1 ) = 54MHz
  TIM1->ARR = 1; // 54MHz/( 1+1 ) = 27MHz
  TIM1->CNT = 0; // clear counter
  TIM1->DIER = 0x0001; // enable update interrupt
  TIM1->CR1 = 0x0005; // enable TIM1 and update event
  NVIC->ISER[0] |= 0x02000000; // enable (25)TIM1 interrupt
 
  //////ADC1//////////////////////
  GPIOA->MODER |= 0x00003000; // PA6 = analog mode(ADC12_IN6-)
  RCC->APB2ENR |= 0x00000100; // enable ADC1 clock
  ADC->CCR = 0x00000000; // ADCCLK = 54MHz/2 = 27MHz
  ADC1->SMPR2 = 0x001C0000; // sampling time of channel 6 = 480 cycle
  ADC1->CR1 = 0x00000000; // 12-bit resolution
  ADC1->CR2 = 0x00000001; // right alignment, single conversion, ADON = 1
  ADC1->SQR1 = 0x00000000; // total regular channel number = 1
  ADC1->SQR3 = 0x00000006; // channel 6
  
  
  //  1. SD 초기화, FAT 초기화
  // 1-1. SD 카드 초기화
    if (disk_initialize( 1 ) != 0) {
        TFT_string(10, 10, White, Black, "SD 초기화 실패");
        while (1); // SD 카드 초기화 실패 시 대기
    }

    // 1-2. FAT 파일 시스템 마운트
    if (f_mount(&FatFs, "", 1) != FR_OK) {
        TFT_string(10, 10, White, Black, "파일 시스템 마운트 실패");
        while (1); // 파일 시스템 마운트 실패 시 대기
    }
  
  
  
  //////2. 새로운 directory 만들기//
  FRESULT res0 = create_directory();
    if (res0 == FR_OK) {
        TFT_string(10, 10, White, Black, "디렉토리 생성됨");
    } else {
        TFT_string(10, 10, White, Black, "디렉토리 생성 실패");
    }


  while(1)
  {
    if ( interrupt_flag_key1 == 1 )
     { interrupt_flag_key1 = 0;
       create_new_file();
     }
    
    if ( interrupt_flag_key2 == 1 )
    { interrupt_flag_key2 = 0;
      if ( f_close( &file ) != FR_OK ) {
        TFT_string( 10, 10, White, Black, "파일 닫기 실패" );
        
          if ( error_count >= 3){
              TFT_string( 10, 10, White, Black, "에러 3번 이상 발생. 리셋하시오." );
              error_count = 0;
          }
      } else {
          RCC->APB2ENR &= 0xFFFFFFFE; // TIM1 disable
          TFT_string( 10, 10, White, Black, "파일 닫기 성공");
      }   
    }  
  }
}
//res_close = f_close( &file );

/////////////사용자 설정 함수//////////////////////////////////
#define DIRECTORY_NAME "recordings"           // 디렉토리 이름
#define FILENAME_PREFIX "record_"               // 파일 이름 접두어
#define FILENAME_EXTENSION ".txt"             // 파일 확장자
//extern FIL file;

unsigned int get_next_file_index(void) {
    DIR dir;
    FILINFO fno;
    //FIL file; main.c 에서 전역변수로 쓸거고, 여기서도 extern FIL file;로 전역변수로 설정해서 가져다 쓸 수 있게 할것임. 물론 main.c에서 잘 관리해야돼.순차적으로.
    
    unsigned int max_index = 0;
    char *filename;

    // 디렉토리 열기
    if (f_opendir(&dir, DIRECTORY_NAME) == FR_OK) {
        while (1) {
            // 파일 정보 읽기
            if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) {
                break; // 디렉토리 끝
            }

            // 파일 이름 분석
            filename = fno.fname;
            if (strncmp(filename, FILENAME_PREFIX, strlen(FILENAME_PREFIX)) == 0) {
                // "record_" 이후의 숫자를 추출
                unsigned int current_index = atoi(&filename[strlen(FILENAME_PREFIX)]);
                if (current_index > max_index) {
                    max_index = current_index; // 최대 인덱스 갱신
                }
            }
        }
        f_closedir(&dir); // 디렉토리 닫기
    }

    return max_index + 1; // 다음 파일 인덱스 반환
}

void create_new_file(void) {
    char filename[64]; // 파일 이름 저장 버퍼
    unsigned int next_index = get_next_file_index();
    unsigned int error_count = 0;
    // 새로운 파일 이름 생성
    sprintf(filename, DIRECTORY_NAME "/" FILENAME_PREFIX "%u" FILENAME_EXTENSION, next_index);

    // 파일 생성
    if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
            TFT_string( 10, 10, White, Black, "파일 생성 실패" );
            if ( error_count >= 3){
            TFT_string( 10, 10, White, Black, "에러 3번 이상 발생. 리셋하시오." );
            error_count = 0;
        }
    }
}        
        //static volatile int error_flag = 0;

      //  if ( !error_flag ) {
        //    error_flag = 1; // 오류 상태 설정
          //  RCC->APB2ENR &= 0xFFFFFFFF; 
            //RCC->APB2ENR |= 0x00000001;
            //error_flag = 0; // 오류 상태 해제
           // if ( error_count > 3 ) {
             //   return;
            //error_count++;
    //}
      //  }


FRESULT create_directory(void) {
    DIR dir;
    FRESULT res1 = f_opendir(&dir, "recordings");  // "recordings" 디렉토리 열기
    if (res1 == FR_OK) {
        f_closedir(&dir);  // 디렉토리가 존재하면 정상적으로 닫음
        return FR_OK;  // 이미 디렉토리가 존재함
    }
    // 디렉토리가 존재하지 않으면 생성
    res1 = f_mkdir("recordings");  
    return res1;
}


///////////////////////////////////////////////


/*
에러 처리는 TFT_LCD에 띄우는거 아니면 답 없는데..
res_open, res_close, res_write가 뭔지. 형태는 뭔지
sprintf가 뭔지
f_mkdir
f_write
f_open
f_close

FRESULT f_mkdir (
  const TCHAR* path / [IN] Directory name 
);
FRESULT f_write (
  FIL* fp,          /   [IN] Pointer to the file object structure 
  const void* buff, / [IN] Pointer to the data to be written 
  UINT btw,         / [IN] Number of bytes to write 
  UINT* bw          / [OUT] Pointer to the variable to return number of bytes written 
);
FRESULT f_open (
  FIL* fp,           / [OUT] Pointer to the file object structure 
  const TCHAR* path, / [IN] File name 
  BYTE mode          / [IN] Mode flags 
);
FRESULT f_close (
  FIL* fp     / [IN] Pointer to the file object 
);

ADC1_data_buffer[ i ] = ADC1->DR; 여기서 배열 하나의 크기는 16비트이고, 12비트씩 저장되는데 하위비트부터 채워지는지, 나머지는 0으로 채워지는지?



================================
특히 긴 문자열 표시 시 ADC 데이터 손실을 방지하려면 메시지 출력이 비동기적으로 실행되거나 기록 중단 상태를 유지해야 합니다.
이건 어떻게 해야 하는거야

&file 에서 file이 생성할 때, FIL file; 이렇게 한거 보면, 만든 파일의 객체라고 보면 되겠네
그래서 f_write(&file, ) 하면 FatFS 정의의 FIL*fp 파일 객체 포인터, 즉 SD카드 안의 파일의 첫 block의 시작 주소를 나타내는거고?

기록 중단 상태 유지를 하고 싶은데, 에러가 떴을때, KEY1 핸들러안의 내용을 실행시킬 순 없지 않나.
if (f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
        RCC->APB2ENR &= 0xFFFFFFFE; // TIM1 disable
        TFT_string( 10, 10, White, Black, "파일 생성 실패" );
    }
이렇게 하면 충돌이나 문제가 생길 수 있나?

static int가 뭐지? */