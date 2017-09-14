#include "ili.h"

void TFT_sendByte(uint8_t data)
{
    while(__HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_TXE) == RESET);
    HAL_SPI_Transmit(TFT_SPI, &data, 1, 100);
    int o = 0;
    while (o < 50) {
	o++;
    }
    
/*
  uint8_t i;
  for(i=0; i<8; i++)
  {
  GPIO_WriteBit(TFT_PORT, PIN_SCK, Bit_RESET);
  if ((data & 0x80) != 0) {
  GPIO_WriteBit(TFT_PORT, PIN_MOSI, Bit_SET);
  } else {
  GPIO_WriteBit(TFT_PORT, PIN_MOSI, Bit_RESET);
  }
  data <<= 1;
  GPIO_WriteBit(TFT_PORT, PIN_SCK, Bit_SET);
  }
  GPIO_WriteBit(TFT_PORT, PIN_SCK, Bit_RESET);
*/
}

void TFT_sendCMD(int index)
{
// ждем чтобы старые данные ушли до того как мы поменяем состояние линий управления дисплеем
    while( __HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_BSY) != RESET);
    
    TFT_CS_HIGH;
    TFT_DC_LOW;
    TFT_CS_LOW;

    TFT_sendByte(index);
}

void TFT_sendDATA(int data)
{
    while(__HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_BSY) != RESET);
    TFT_DC_HIGH;
    TFT_sendByte(data);
}

void TFT_sendWord(int data)
{
    while(__HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_BSY) != RESET);
    TFT_DC_HIGH;

    TFT_sendByte(data >> 8);
    TFT_sendByte(data & 0x00ff);
}

int TFT_Read_Register(int Addr, int xParameter)
{
    uint8_t data = 0;

    TFT_sendCMD(0xD9);                                                     // ext command
    TFT_sendByte(0x10+xParameter);                           // 0x11 is the first Parameter
    TFT_DC_LOW;
    TFT_CS_LOW;
    TFT_sendByte(Addr);
    TFT_DC_HIGH;
    while(HAL_SPI_GetState(TFT_SPI)==HAL_SPI_STATE_BUSY_RX);
    HAL_SPI_Receive(TFT_SPI, &data, 1, 10);
//    data = SPI_I2S_ReceiveData(TFT_SPI);
    TFT_CS_HIGH;

    return data;
}

int TFT_readID(void)
{
    int i = 0;
    int byte;
    int data[3] = {0x00, 0x00, 0x00};
    int ID[3] = {0x00, 0x93, 0x41};
    int ToF = 1;

    for(i=0; i<3; i++)
    {
	byte = TFT_Read_Register(0xD3, i+1);
        data[i] = byte;

        if(data[i] != ID[i])
        {
            ToF = 0;
        }
    }

    return ToF;
}

void TFT_init()
{
    int i;
    /* GPIO_InitTypeDef gpio; */

    /* RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); */

    /* gpio.GPIO_Pin = PIN_LED | PIN_DC | PIN_RST | PIN_CS; */
    /* gpio.GPIO_Speed = GPIO_Speed_50MHz; */
    /* gpio.GPIO_Mode = GPIO_Mode_Out_PP; */
    /* GPIO_Init(TFT_PORT, &gpio); */

    /* RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); */

    /* gpio.GPIO_Pin = PIN_SCK | PIN_MOSI; */
    /* gpio.GPIO_Speed = GPIO_Speed_50MHz; */
    /* gpio.GPIO_Mode = GPIO_Mode_AF_PP; */
    /* GPIO_Init(TFT_PORT, &gpio); */

    /* gpio.GPIO_Pin = PIN_MISO; */
    /* gpio.GPIO_Mode = GPIO_Mode_IPU; */
    /* GPIO_Init(TFT_PORT, &gpio); */

    /* //SPI_StructInit(&spi); */
    /* SPI_InitTypeDef spi; */
    /* spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex; */
    /* spi.SPI_Mode = SPI_Mode_Master; */
    /* spi.SPI_DataSize = SPI_DataSize_8b; */
    /* spi.SPI_CPOL = SPI_CPOL_Low; */
    /* spi.SPI_CPHA = SPI_CPHA_1Edge; */
    /* spi.SPI_NSS = SPI_NSS_Soft; */
    /* spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; */
    /* spi.SPI_FirstBit = SPI_FirstBit_MSB; */
    /* SPI_Init(TFT_SPI, &spi); */
    /* SPI_Cmd(TFT_SPI, ENABLE); */
    /* SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set); */

    TFT_RST_LOW;
    for(i=0; i<0x0FFFF; i++);
    TFT_RST_HIGH;

    TFT_sendCMD(0xCB);
    TFT_sendDATA(0x39);
    TFT_sendDATA(0x2C);
    TFT_sendDATA(0x00);
    TFT_sendDATA(0x34);
    TFT_sendDATA(0x02);

    TFT_sendCMD(0xCF);
    TFT_sendDATA(0x00);
    TFT_sendDATA(0xC1);
    TFT_sendDATA(0x30);

    TFT_sendCMD(0xE8);
    TFT_sendDATA(0x85);
    TFT_sendDATA(0x00);
    TFT_sendDATA(0x78);

    TFT_sendCMD(0xEA);
    TFT_sendDATA(0x00);
    TFT_sendDATA(0x00);

    TFT_sendCMD(0xED);
    TFT_sendDATA(0x64);
    TFT_sendDATA(0x03);
    TFT_sendDATA(0x12);
    TFT_sendDATA(0x81);

    TFT_sendCMD(0xF7);
    TFT_sendDATA(0x20);

    TFT_sendCMD(0xC0);    	//Power control
    TFT_sendDATA(0x23);   	//VRH[5:0]

    TFT_sendCMD(0xC1);    	//Power control
    TFT_sendDATA(0x10);   	//SAP[2:0];BT[3:0]

    TFT_sendCMD(0xC5);    	//VCM control
    TFT_sendDATA(0x3e);   	//Contrast
    TFT_sendDATA(0x28);

    TFT_sendCMD(0xC7);    	//VCM control2
    TFT_sendDATA(0x86);  	 //--

    TFT_sendCMD(0x36);    	// Memory Access Control
    TFT_sendDATA(0x48);  	//C8	   //48 68ç»æ §ç//28 E8 åŠ¯îç

    TFT_sendCMD(0x3A);
    TFT_sendDATA(0x55);

    TFT_sendCMD(0xB1);
    TFT_sendDATA(0x00);
    TFT_sendDATA(0x18);

    TFT_sendCMD(0xB6);    	// Display Function Control
    TFT_sendDATA(0x08);
    TFT_sendDATA(0x82);
    TFT_sendDATA(0x27);

    TFT_sendCMD(0xF2);    	// 3Gamma Function Disable
    TFT_sendDATA(0x00);

    TFT_sendCMD(0x26);    	//Gamma curve selected
    TFT_sendDATA(0x01);

    TFT_sendCMD(0xE0);    	//Set Gamma
    TFT_sendDATA(0x0F);
    TFT_sendDATA(0x31);
    TFT_sendDATA(0x2B);
    TFT_sendDATA(0x0C);
    TFT_sendDATA(0x0E);
    TFT_sendDATA(0x08);
    TFT_sendDATA(0x4E);
    TFT_sendDATA(0xF1);
    TFT_sendDATA(0x37);
    TFT_sendDATA(0x07);
    TFT_sendDATA(0x10);
    TFT_sendDATA(0x03);
    TFT_sendDATA(0x0E);
    TFT_sendDATA(0x09);
    TFT_sendDATA(0x00);

    TFT_sendCMD(0xE1);    	//Set Gamma
    TFT_sendDATA(0x00);
    TFT_sendDATA(0x0E);
    TFT_sendDATA(0x14);
    TFT_sendDATA(0x03);
    TFT_sendDATA(0x11);
    TFT_sendDATA(0x07);
    TFT_sendDATA(0x31);
    TFT_sendDATA(0xC1);
    TFT_sendDATA(0x48);
    TFT_sendDATA(0x08);
    TFT_sendDATA(0x0F);
    TFT_sendDATA(0x0C);
    TFT_sendDATA(0x31);
    TFT_sendDATA(0x36);
    TFT_sendDATA(0x0F);

    TFT_sendCMD(0x11);    	//Exit Sleep
    for(i=0; i<0x00FFF; i++);

    TFT_sendCMD(0x29);    //Display on
    TFT_sendCMD(0x2c);
}

void TFT_led(int state)
{
    if (state)
	HAL_GPIO_WritePin(TFT_PORT, PIN_LED, GPIO_PIN_SET);
    else
	HAL_GPIO_WritePin(TFT_PORT, PIN_LED, GPIO_PIN_RESET);
}

void TFT_setCol(int StartCol, int EndCol)
{
    TFT_sendCMD(0x2A);                                                      // Column Command address
    TFT_sendWord(StartCol);
    TFT_sendWord(EndCol);
}

void TFT_setPage(int StartPage, int EndPage)
{
    TFT_sendCMD(0x2B);                                                      // Column Command address
    TFT_sendWord(StartPage);
    TFT_sendWord(EndPage);
}

void TFT_setXY(int poX, int poY)
{
    TFT_setCol(poX, poX);
    TFT_setPage(poY, poY);
    TFT_sendCMD(0x2c);
}

void TFT_setPixel(int poX, int poY, int color)
{
    TFT_setXY(poX, poY);
    TFT_sendWord(color);
}
