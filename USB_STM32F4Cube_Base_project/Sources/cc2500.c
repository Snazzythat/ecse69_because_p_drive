/**
  ******************************************************************************
  * @file    CC2500.c
  * @author  Ashraf Suyyagh based on the MCD Application Team implementation of the CC2500 driver
             of both the standard peripheral library drivers and the STM32F4 Cube drivers
  * @version V 1.1.0
  * @date    17-January-2016
  * @brief   This file provides a set of functions needed to manage the CC2500
  *          MEMS accelerometer available on STM32F4-Discovery Kit.
  ******************************************************************************
  * NOTES:
  * The accelerometer packaged driver in STM32Cube drivers is highly abstracted and uses 
  * five files (accelero.h, stm32f4_discovery_accelerometer.c/.h and CC2500/lis302dl.c/.h)
  * I have modified the drivers I written in previous years to be STM32F4Cube compatible
  * with minimum modifications to the API/Definitions
  ******************************************************************************
  * LIST OF CHANGES 1.1.0 (17-January-2016)
  * Added an SPI handle declaration "SpiHandle_CC2500"
  * Modified  the following functions to be compatible with the STM32CUBE driver HAL
  * 1. CC2500_Init 
  * 2. HAL_SPI_CC2500_MspInit
  * 3. CC2500_SendByte
  * 4. SPI_CC2500_SendData 
  * 5. SPI_CC2500_ReceiveData
  * (Experimented with porting the new STM32Cube bundeled versions of HAL_SPI_TransmitReceive
     which are more thourough and have added functionality of setting SPI handle states, but
     I ran into several issues. Reverted back to basic SPI read/write from old drivers set for 
     now)
  ******************************************************************************
  * BUGS/FIXES:
  * The new GPIO struct initialisation has new definitions for inputs used as external interrupt
  * sources, that is inputs are explicitly defined as input source with triggering edge. This is 
  * used to abstract the configuration of the pin as an EXTI source. The SPI initialisation for 
  * the driver sets PE0 and PE1 as INT1/INT2 for the CC2500 and configures them accordingly.
  * By default, PE0 is set as EXTI_Line0 source. It conflicts with the Push Button. A work around 
  * is either 
    1. Modify the driver to remove the initialisation of PE0/PE1 (not recommended)
    2. Simply call HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0) AFTER initialsing the Accelerometer, this
  * resets PE0    
  */

/* Includes ------------------------------------------------------------------*/
#include "cc2500.h"
#include "cc2500_settings.h"

/** @addtogroup Utilities
  * @{
  */

/** @addtogroup STM32F4_DISCOVERY
  * @{
  */

/** @addtogroup STM32F4_DISCOVERY_CC2500
  * @{
  */


/** @defgroup STM32F4_DISCOVERY_CC2500_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32F4_DISCOVERY_CC2500_Private_Defines
  * @{
  */
__IO uint32_t  CC2500Timeout = CC2500_FLAG_TIMEOUT;

SPI_HandleTypeDef    SpiHandle_CC2500;

/* Read/Write command */
#define READWRITE_CMD              ((uint8_t)0x80)
/* Multiple byte read/write command */
#define MULTIPLEBYTE_CMD           ((uint8_t)0x40)
/* Dummy Byte Send by the SPI Master device in order to generate the Clock to the Slave device */
#define DUMMY_BYTE                 ((uint8_t)0x00)

#define USE_DEFAULT_TIMEOUT_CALLBACK

/**
  * @}
  */

/** @defgroup STM32F4_DISCOVERY_CC2500_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32F4_DISCOVERY_CC2500_Private_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup STM32F4_DISCOVERY_CC2500_Private_FunctionPrototypes
  * @{
  */
static uint8_t CC2500_SendByte(uint8_t byte);
void SPI_CC2500_SendData(SPI_HandleTypeDef *hspi, uint16_t Data);
uint8_t SPI_CC2500_ReceiveData(SPI_HandleTypeDef *hspi);
void CC2500_LowLevel_Init(void);
/**
  * @}
  */

/** @defgroup STM32F4_DISCOVERY_CC2500_Private_Functions
  * @{
  */

/////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/*************************** START REGISTER MAPPING  **************************/
/******************************************************************************/
static const uint8_t CC2500_SRES = 0x30;
//static const uint8_t CC2500_SNOP = 0x3D;
static const uint8_t CC2500_SFRX = 0x3A;
static const uint8_t CC2500_SFTX = 0x3B;

static const uint8_t CC2500_PARTNUM = 0x30 | 0xC0;
static const uint8_t CC2500_MARCSTATE = 0x35 | 0xC0;
static const uint8_t CC2500_TXBYTES = 0x3A | 0xC0;
static const uint8_t CC2500_RXBYTES = 0x3B | 0xC0;


/////////////////////////////////////////////////////////////////////////////////////////////////

// /**
//   * @brief  Set CC2500 Initialization.
//   * @param  CC2500_Config_Struct: pointer to a CC2500_Config_TypeDef structure
//   *         that contains the configuration setting for the CC2500.
//   * @retval None
//   */
// void CC2500_Init(void)
// {

//   /* Configure the low level interface ---------------------------------------*/
//     /* SPI configuration -------------------------------------------------------*/
//   __HAL_RCC_SPI2_CLK_ENABLE();
  
//   HAL_SPI_DeInit(&SpiHandle_CC2500);
//   SpiHandle_CC2500.Instance                = SPI2;
//   SpiHandle_CC2500.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_4;
//   SpiHandle_CC2500.Init.Direction          = SPI_DIRECTION_2LINES;
//   SpiHandle_CC2500.Init.CLKPhase           = SPI_PHASE_1EDGE;
//   SpiHandle_CC2500.Init.CLKPolarity        = SPI_POLARITY_LOW;
//   SpiHandle_CC2500.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLED;
//   SpiHandle_CC2500.Init.CRCPolynomial      = 7;
//   SpiHandle_CC2500.Init.DataSize           = SPI_DATASIZE_8BIT;
//   SpiHandle_CC2500.Init.FirstBit           = SPI_FIRSTBIT_MSB;
//   SpiHandle_CC2500.Init.NSS                = SPI_NSS_SOFT;
//   SpiHandle_CC2500.Init.TIMode             = SPI_TIMODE_DISABLED;
//   SpiHandle_CC2500.Init.Mode               = SPI_MODE_MASTER;
//   if (HAL_SPI_Init(&SpiHandle_CC2500) != HAL_OK) {printf ("ERROR: Error in initialising SPI1 \n");};
  
//   __HAL_SPI_ENABLE(&SpiHandle_CC2500);
  
// }

/**
  * @brief  Set CC2500 Initialization.
  * @param  CC2500_Config_Struct: pointer to a CC2500_Config_TypeDef structure
  *         that contains the configuration setting for the CC2500.
  * @retval None
  */
void CC2500_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure_CC2500;

  /* Enable the SPI periph */
  __SPI2_CLK_ENABLE();

  /* Enable SCK, MOSI and MISO GPIO clocks */
  __GPIOB_CLK_ENABLE();
  
    /* Configure the low level interface ---------------------------------------*/
    /* SPI configuration -------------------------------------------------------*/
  __HAL_RCC_SPI2_CLK_ENABLE();
  
  HAL_SPI_DeInit(&SpiHandle_CC2500);
  SpiHandle_CC2500.Instance                 = SPI2;
  SpiHandle_CC2500.Init.BaudRatePrescaler   = SPI_BAUDRATEPRESCALER_8;
  SpiHandle_CC2500.Init.Direction           = SPI_DIRECTION_2LINES;
  SpiHandle_CC2500.Init.CLKPhase            = SPI_PHASE_1EDGE;
  SpiHandle_CC2500.Init.CLKPolarity         = SPI_POLARITY_LOW;
  SpiHandle_CC2500.Init.CRCCalculation      = SPI_CRCCALCULATION_DISABLED;
  SpiHandle_CC2500.Init.CRCPolynomial       = 7;
  SpiHandle_CC2500.Init.DataSize            = SPI_DATASIZE_8BIT;
  SpiHandle_CC2500.Init.FirstBit            = SPI_FIRSTBIT_MSB;
  SpiHandle_CC2500.Init.NSS                 = SPI_NSS_SOFT;
  SpiHandle_CC2500.Init.TIMode              = SPI_TIMODE_DISABLED;
  SpiHandle_CC2500.Init.Mode                = SPI_MODE_MASTER;
  if (HAL_SPI_Init(&SpiHandle_CC2500) != HAL_OK) {printf ("ERROR: Error in initialising SPI2 \n");};
  
  __HAL_SPI_ENABLE(&SpiHandle_CC2500);

  GPIO_InitStructure_CC2500.Mode  = GPIO_MODE_AF_PP;
  GPIO_InitStructure_CC2500.Pull  = GPIO_PULLDOWN;
  GPIO_InitStructure_CC2500.Speed = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure_CC2500.Alternate = GPIO_AF5_SPI2;

  /* SPI SCK pin configuration */
  GPIO_InitStructure_CC2500.Pin = CC2500_SPI_SCK_PIN;
  HAL_GPIO_Init(CC2500_SPI_SCK_GPIO_PORT, &GPIO_InitStructure_CC2500);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure_CC2500.Pin =  CC2500_SPI_MOSI_PIN;
  HAL_GPIO_Init(CC2500_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure_CC2500);

  /* SPI MISO pin configuration */
  GPIO_InitStructure_CC2500.Pin = CC2500_SPI_MISO_PIN;
  HAL_GPIO_Init(CC2500_SPI_MISO_GPIO_PORT, &GPIO_InitStructure_CC2500);
  
  GPIO_InitStructure_CC2500.Pin   = CC2500_SPI_CS_PIN;
  GPIO_InitStructure_CC2500.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStructure_CC2500.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(CC2500_SPI_CS_GPIO_PORT, &GPIO_InitStructure_CC2500);

  /* Deselect : Chip Select high */
  HAL_GPIO_WritePin(CC2500_SPI_CS_GPIO_PORT, CC2500_SPI_CS_PIN, GPIO_PIN_SET);
  
}

/**
 * @brief  Read one byte from CC2500.
 * @param  ReadAddr : address to read from
 * @retval byte value read from CC2500
 */
__inline uint8_t CC2500_read_one(uint8_t ReadAddr) {
  uint8_t temp;
  CC2500_Read(&temp, ReadAddr, 1);
  return temp;
}

/**
 * @brief  Write one byte to CC2500.
 * @param  WriteAddr : address to write to
 * @retval void
 */
__inline void CC2500_write_one(uint8_t* value, uint8_t WriteAddr) {
  CC2500_Write(value, WriteAddr, 1);
}

/**
 * @brief  Get SFM state of CC2500.
 * @param void
 * @retval SFM state of CC2500
 */
__inline uint8_t CC2500_get_state(void) {
  return CC2500_read_one(CC2500_MARCSTATE);
}

/**
 * @brief  Get part num of CC2500.
 * @param void
 * @retval part num of CC2500
 */
__inline uint8_t CC2500_get_part_num(void) {
  return CC2500_read_one(CC2500_PARTNUM);
}

/**
 * @brief  Get number of bytes in RX FIFO of CC2500.
 * @param void
 * @retval number of bytes in RX FIFO of CC2500
 */
__inline uint8_t CC2500_get_rxbytes(void) {
  return CC2500_read_one(CC2500_RXBYTES);
}

/**
 * @brief  Get number of bytes in TX FIFO of CC2500.
 * @param void
 * @retval number of bytes in TX FIFO of CC2500
 */
__inline uint8_t CC2500_get_txbytes(void) {
  return CC2500_read_one(CC2500_TXBYTES);
}

/**
 * @brief  Flush RX FIFO of CC2500.
 * @param void
 * @retval byte received from CC2500
 */
__inline uint8_t CC2500_flush_rx(void) {
  return CC2500_read_one(CC2500_SFRX);
}

/**
 * @brief  Flush TX FIFO of CC2500.
 * @param void
 * @retval byte received from CC2500
 */
__inline uint8_t CC2500_flush_tx(void) {
  return CC2500_read_one(CC2500_SFTX);
}

/**
 * @brief  Read one byte from RX FIFO of CC2500.
 * @param void
 * @retval byte received from CC2500
 */
__inline uint8_t CC2500_read_rx_one(void) {
  return CC2500_read_one(CC2500_RX_FIFO);
}

/**
 * @brief  Read multiple bytes from RX FIFO of CC2500.
 * @param buffer buffer to write result to
 * @param NumByteToRead number of bytes to read from RX FIFO
 * @retval void
 */
__inline void CC2500_read_rx(uint8_t* buffer, uint8_t NumByteToRead) {
  if (NumByteToRead == 1) {
    CC2500_Read(buffer, CC2500_RX_FIFO, NumByteToRead);
  } else if (NumByteToRead > 1) {
    CC2500_Read(buffer, CC2500_RX_FIFO_BURST, NumByteToRead);
  }
}

/**
 * @brief  Write one byte to TX FIFO of CC2500.
 * @param value value to write
 * @retval void
 */
__inline void CC2500_write_tx_one(uint8_t value) {
  CC2500_write_one(&value, CC2500_TX_FIFO);
}

/**
 * @brief  Write multiple bytes to TX FIFO of CC2500.
 * @param buffer buffer to read values from
 * @param NumByteToWrite number of bytes to write to TX FIFO
 * @retval void
 */
__inline void CC2500_write_tx(uint8_t* buffer, uint8_t NumByteToWrite) {
  if (NumByteToWrite == 1) {
    CC2500_Write(buffer, CC2500_TX_FIFO, NumByteToWrite);
  } else if (NumByteToWrite > 1) {
    CC2500_Write(buffer, CC2500_TX_FIFO_BURST, NumByteToWrite); 
  }
}

/**
  * @brief  Writes one byte to the CC2500.
  * @param  pBuffer : pointer to the buffer  containing the data to be written to the CC2500.
  * @param  WriteAddr : CC2500's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void CC2500_Write(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
  /* Configure the MS bit:
       - When 0, the address will remain unchanged in multiple read/write commands.
       - When 1, the address will be auto incremented in multiple read/write commands.
  */
  if(NumByteToWrite > 0x01)
  {
    WriteAddr |= (uint8_t)MULTIPLEBYTE_CMD;
  }
  /* Set chip select Low at the start of the transmission */
  CC2500_CS_LOW();

  /* Send the Address of the indexed register */
  CC2500_SendByte(WriteAddr);
  /* Send the data that will be written into the device (MSB First) */
  while(NumByteToWrite >= 0x01)
  {
    CC2500_SendByte(*pBuffer);
    NumByteToWrite--;
    pBuffer++;
  }

  /* Set chip select High at the end of the transmission */
  CC2500_CS_HIGH();
}

/**
  * @brief  Reads a block of data from the CC2500.
  * @param  pBuffer : pointer to the buffer that receives the data read from the CC2500.
  * @param  ReadAddr : CC2500's internal address to read from.
  * @param  NumByteToRead : number of bytes to read from the CC2500.
  * @retval None
  */
void CC2500_Read(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
  if(NumByteToRead > 0x01)
  {
    ReadAddr |= (uint8_t)(READWRITE_CMD | MULTIPLEBYTE_CMD);
  }
  else
  {
    ReadAddr |= (uint8_t)READWRITE_CMD;
  }
  /* Set chip select Low at the start of the transmission */
  CC2500_CS_LOW();

  /* Send the Address of the indexed register */
  CC2500_SendByte(ReadAddr);

  /* Receive the data that will be read from the device (MSB First) */
  while(NumByteToRead > 0x00)
  {
    /* Send dummy byte (0x00) to generate the SPI clock to CC2500 (Slave device) */
    *pBuffer = CC2500_SendByte(DUMMY_BYTE);
    NumByteToRead--;
    pBuffer++;
  }

  /* Set chip select High at the end of the transmission */
  CC2500_CS_HIGH();
}

/**
 * @brief  Reset CC2500 chip.
 * @param  void
 * @retval None
 */
void CC2500_Reset(void) {
  static uint8_t value;
  value = CC2500_read_one(CC2500_SRES);
  do {
    value = CC2500_get_state();
  } while (value != 1);

  for (uint8_t i = 0; i < CC2500_CONFIG_COUNT; i++) {
    CC2500_write_one(&(cc2500_initial_configuration[i][0]), cc2500_initial_configuration[i][1]);
  }

  CC2500_flush_rx();
  CC2500_flush_tx();
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
static uint8_t CC2500_SendByte(uint8_t byte)
{
  /* Loop while DR register in not empty */
  CC2500Timeout = CC2500_FLAG_TIMEOUT;
  while (__HAL_SPI_GET_FLAG(&SpiHandle_CC2500, SPI_FLAG_TXE) == RESET)
  {
    if((CC2500Timeout--) == 0) return CC2500_TIMEOUT_UserCallback();
  }

  /* Send a Byte through the SPI peripheral */
  SPI_CC2500_SendData(&SpiHandle_CC2500,  byte);

  /* Wait to receive a Byte */
  CC2500Timeout = CC2500_FLAG_TIMEOUT;
  while (__HAL_SPI_GET_FLAG(&SpiHandle_CC2500, SPI_FLAG_RXNE) == RESET)
  {
    if((CC2500Timeout--) == 0) {
      return CC2500_TIMEOUT_UserCallback();
    }
  }

  /* Return the Byte read from the SPI bus */ 
  return SPI_CC2500_ReceiveData(&SpiHandle_CC2500);
}


#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t CC2500_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
//  while (1)
 // {
  //}
  return 0;
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

// void HAL_SPI_CC2500_MspInit(SPI_HandleTypeDef *hspi){
//   GPIO_InitTypeDef GPIO_InitStructure;

//   /* Enable the SPI periph */
//   __SPI2_CLK_ENABLE();

//   /* Enable CS, SCK, MOSI and MISO GPIO clocks */
//   __GPIOB_CLK_ENABLE();

//   GPIO_InitStructure.Mode  = GPIO_MODE_AF_PP;
//   GPIO_InitStructure.Pull  = GPIO_PULLDOWN;
//   GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
//   GPIO_InitStructure.Alternate = GPIO_AF5_SPI1;

//   /* SPI SCK pin configuration */
//   GPIO_InitStructure.Pin = CC2500_SPI_SCK_PIN;
//   HAL_GPIO_Init(CC2500_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

//   /* SPI  MOSI pin configuration */
//   GPIO_InitStructure.Pin =  CC2500_SPI_MOSI_PIN;
//   HAL_GPIO_Init(CC2500_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

//   /* SPI MISO pin configuration */
//   GPIO_InitStructure.Pin = CC2500_SPI_MISO_PIN;
//   HAL_GPIO_Init(CC2500_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);
  
//   GPIO_InitStructure.Pin   = CC2500_SPI_CS_PIN;
//   GPIO_InitStructure.Mode  = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_MEDIUM;
//   HAL_GPIO_Init(CC2500_SPI_CS_GPIO_PORT, &GPIO_InitStructure);

//   /* Deselect : Chip Select high */
//   HAL_GPIO_WritePin(CC2500_SPI_CS_GPIO_PORT, CC2500_SPI_CS_PIN, GPIO_PIN_SET);

// }

/**
  * @brief  Transmits a Data through the SPIx/I2Sx peripheral.
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @param  Data: Data to be transmitted.
  * @retval None
  */
void SPI_CC2500_SendData(SPI_HandleTypeDef *hspi, uint16_t Data)
{ 
  /* Write in the DR register the data to be sent */
  hspi->Instance->DR = Data;
}

/**
  * @brief  Returns the most recent received data by the SPIx/I2Sx peripheral. 
  * @param  *hspi: Pointer to the SPI handle. Its member Instance can point to either SPI1, SPI2 or SPI3 
  * @retval The value of the received data.
  */
uint8_t SPI_CC2500_ReceiveData(SPI_HandleTypeDef *hspi)
{
  /* Return the data in the DR register */
  return hspi->Instance->DR;
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
