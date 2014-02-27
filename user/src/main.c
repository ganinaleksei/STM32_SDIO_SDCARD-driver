/******************** (C) COPYRIGHT 2012 WildFire Team **************************
 * ???  :main.c
 * ??    :MicroSD?(SDIO??)????,??????????1?????????
 *           ????         
 * ????:??STM32???
 * ???  :ST3.5.0
 *
 * ??    :wildfire team 
 * ??    :http://www.amobbs.com/forum-1008-1.html
 * ??    :http://firestm32.taobao.com
*********************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32_eval_sdio_sd.h"
#include "usart1.h"	


/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/
#define BLOCK_SIZE            512 /* Block Size in Bytes */

#define NUMBER_OF_BLOCKS      32  /* For Multi Blocks operation (Read/Write) */
#define MULTI_BUFFER_SIZE    (BLOCK_SIZE * NUMBER_OF_BLOCKS)   //?????	 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t Buffer_Block_Tx[BLOCK_SIZE], Buffer_Block_Rx[BLOCK_SIZE];
uint8_t readbuff[BLOCK_SIZE];
uint8_t Buffer_MultiBlock_Tx[MULTI_BUFFER_SIZE], Buffer_MultiBlock_Rx[MULTI_BUFFER_SIZE];
volatile TestStatus EraseStatus = FAILED, TransferStatus1 = FAILED, TransferStatus2 = FAILED;
SD_Error Status = SD_OK;
extern SD_CardInfo SDCardInfo;	
int i;


/* Private function prototypes -----------------------------------------------*/  
void SD_EraseTest(void);
void SD_SingleBlockTest(void);
void SD_MultiBlockTest(void);
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength);
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength);

int main(void)
{

	  /* USART1 config */
		USART1_Config();

    /*------------------------------ SD Init ---------------------------------- */
	Status = SD_Init();

	printf( "\r\n this is a microSD test\r\n " );


  if(Status == SD_OK)
	{    
		printf( " \r\n SD_Init succeed \r\n " );		
	 }
  else
  	{
		printf("\r\n SD_Init failed \r\n" );
    	printf("\r\n failed status is: %d \r\n",Status );
	}			  	
	    
 	printf( " \r\n CardType is :%d ", SDCardInfo.CardType );
	printf( " \r\n CardCapacity is :%d ", SDCardInfo.CardCapacity );
	printf( " \r\n CardBlockSize is :%d ", SDCardInfo.CardBlockSize );
	printf( " \r\n RCA is :%d ", SDCardInfo.RCA);
	printf( " \r\n ManufacturerID is :%d \r\n", SDCardInfo.SD_cid.ManufacturerID );

	SD_EraseTest();	   //????  

	SD_SingleBlockTest();  //??????

 	SD_MultiBlockTest();  //??????	

  while (1)
  {}
}


/*
 * ???:SD_EraseTest
 * ??  :??????
 * ??  :?
 * ??  :?
 */
void SD_EraseTest(void)
{
  /*------------------- Block Erase ------------------------------------------*/
  if (Status == SD_OK)
  {
    /* Erase NumberOfBlocks Blocks of WRITE_BL_LEN(512 Bytes) */
    Status = SD_Erase(0x00, (BLOCK_SIZE * NUMBER_OF_BLOCKS));//????????????,????????????
  }

  if (Status == SD_OK)
  {			  //?????????
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);

    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();  //????dma??????

    /* Wait until end of DMA transfer */
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of erased blocks */
  if (Status == SD_OK)
  {				  //??????????
    EraseStatus = eBuffercmp(Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
  }
  
  if(EraseStatus == PASSED)
  	printf("\r\n erase OK! " );
 
  else	  
  	printf("\r\n Erase failed! " );
  
}

/*
 * ???:SD_SingleBlockTest
 * ??  :	?????????
 * ??  :?
 * ??  :?
 */
void SD_SingleBlockTest(void)
{
  /*------------------- Block Read/Write --------------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_Block_Tx, BLOCK_SIZE, 0x320F);

  if (Status == SD_OK)
  {
    /* Write block of 512 bytes on address 0 */
    Status = SD_WriteBlock(Buffer_Block_Tx, 0x00, BLOCK_SIZE);
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();	   //??dma????
    while(SD_GetStatus() != SD_TRANSFER_OK); //??sdio?sd?????
  }

  if (Status == SD_OK)
  {
    /* Read block of 512 bytes from address 0 */
    Status = SD_ReadBlock(Buffer_Block_Rx, 0x00, BLOCK_SIZE);//????
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus1 = Buffercmp(Buffer_Block_Tx, Buffer_Block_Rx, BLOCK_SIZE);	//??
  }
  
  if(TransferStatus1 == PASSED)
    printf("\r\n single OK!" );
 
  else  
  	printf("\r\n single failed! " );  
}

/*
 * ???:SD_MultiBlockTest
 * ??  :	????????
 * ??  :?
 * ??  :?
 */
void SD_MultiBlockTest(void)
{
  /*--------------- Multiple Block Read/Write ---------------------*/
  /* Fill the buffer to send */
  Fill_Buffer(Buffer_MultiBlock_Tx, MULTI_BUFFER_SIZE, 0x0);

  if (Status == SD_OK)
  {
    /* Write multiple block of many bytes on address 0 */
    Status = SD_WriteMultiBlocks(Buffer_MultiBlock_Tx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
    Status = SD_WaitWriteOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  if (Status == SD_OK)
  {
    /* Read block of many bytes from address 0 */
    Status = SD_ReadMultiBlocks(Buffer_MultiBlock_Rx, 0x00, BLOCK_SIZE, NUMBER_OF_BLOCKS);
    /* Check if the Transfer is finished */
    Status = SD_WaitReadOperation();
    while(SD_GetStatus() != SD_TRANSFER_OK);
  }

  /* Check the correctness of written data */
  if (Status == SD_OK)
  {
    TransferStatus2 = Buffercmp(Buffer_MultiBlock_Tx, Buffer_MultiBlock_Rx, MULTI_BUFFER_SIZE);
  }
  
  if(TransferStatus2 == PASSED)	  
  	printf("\r\n multi Ok! " );

  else 
  	printf("\r\n multi failed! " );  

}




/*
 * ???:Buffercmp
 * ??  :???????????????
 * ??  :-pBuffer1, -pBuffer2 : ??????????
 *         -BufferLength ?????
 * ??  :-PASSED ??
 *         -FAILED ??
 */
TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    if (*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }

  return PASSED;
}


/*
 * ???:Fill_Buffer
 * ??  :?????????
 * ??  :-pBuffer ???????
 *         -BufferLength ??????
 *         -Offset ??????????
 * ??  :? 
 */
void Fill_Buffer(uint8_t *pBuffer, uint32_t BufferLength, uint32_t Offset)
{
  uint16_t index = 0;

  /* Put in global buffer same values */
  for (index = 0; index < BufferLength; index++ )
  {
    pBuffer[index] = index + Offset;
  }
}


/*
 * ???:eBuffercmp
 * ??  :???????????0
 * ??  :-pBuffer ???????
 *         -BufferLength ?????        
 * ??  :PASSED ????????0
 *         FAILED ?????????????0 
 */
TestStatus eBuffercmp(uint8_t* pBuffer, uint32_t BufferLength)
{
  while (BufferLength--)
  {
    /* In some SD Cards the erased state is 0xFF, in others it's 0x00 */
    if ((*pBuffer != 0xFF) && (*pBuffer != 0x00))//????0xff?0x00
    {
      return FAILED;
    }

    pBuffer++;
  }

  return PASSED;
}


/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/
