/*
 * ds18b20.c
 *
 *  Created on: Sep 18, 2021
 *      Author: Ben Yuen
 */

#include "gpio.h"
#include "ds18b20.h"

/* Set 1-Wire bus */
#define	DQ_In		HAL_GPIO_ReadPin(_18B20_PIN_GPIO_Port,_18B20_PIN_Pin)  //Read bus status
#define	DQ_Out_0	HAL_GPIO_WritePin(_18B20_PIN_GPIO_Port,_18B20_PIN_Pin,GPIO_PIN_RESET) //Pull low
#define	DQ_Out_1	HAL_GPIO_WritePin(_18B20_PIN_GPIO_Port,_18B20_PIN_Pin,GPIO_PIN_SET)  //Pull up

int32_t Temper_Var;  //Temperature variable
float temper_var;  //Temperature variable
DS18B20_RomCode_t RomID[10];  //Maximum 5 ROM ID stored in array, [0] for single ROM attached
DS18B20_RomCode_t AlarmID[10];

typedef enum _DS18B20_Command
{
	/* ROM Commands */
	ROM_READ         = 0x33,
	ROM_MATCH        = 0x55,
	ROM_SEARCH       = 0xF0,
	ROM_SKIP         = 0xCC,
	ALARM_SEARCH     = 0xEC,
	/* Function Commands */
	TEMPER_CONVERT   = 0x44,
	RAM_READ         = 0xBE,
	RAM_WRITE        = 0x4E,
	RAM_TO_EE        = 0x48,
	EE_TO_RAM        = 0xB8,
	POWER_READ       = 0xB4,
}_DS18B20_Command_t;

/* CRC8 data table */
static const uint8_t CRC8_TAB[]= {
		0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
		157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
		35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
		190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
		70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
		219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
		101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
		248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
		140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
		17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
		175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
		50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
		202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
		87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
		233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
		116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

uint8_t DS18B20_CRC(const void *pBuf, uint32_t len)
{
	uint8_t	crc = 0;
	const uint8_t *pBuf_ = pBuf;
	while(len--)
	{
		crc = CRC8_TAB[*pBuf_++ ^ crc];
	}
	return crc;
}

void delay_1us(void)
{
	uint16_t n;
	for (n=0; n<2; n++) //	for (n=0; n<176; n++)
	{
		__NOP();
	}
}

/* Different method to make microsecond delay */
void delay_us(uint32_t t)
{
	uint16_t n;
	for (n=0; n<t; n++)  //n=1 1us
	{
		/* Operation for 1 microsecond delay */
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();  //Add this line for release build
	}
}
//void delay_us(uint32_t t)
//{
//	t = (t << 3)+t;
//	while (t--);
//}
//void delay_us(uint32_t t)
//{
//	t = (t << 3) - (t << 1);
//	while (t--);
//}

void delay_ms(uint16_t t)
{
	t *= 10;
	while (t--)
	{
		delay_us(100);
	}
}

void DS18B20_check_us_timming(uint8_t _set)
{
	/* Make one-wire pull up and down at certain period so that it can be detected by oscilloscope */
	switch(_set)
	{
	case 1:  //Generate 10ms by delay_us call
		for(uint8_t i=0;i<2;i++){
			DQ_Out_0;
			delay_us(10000);
			DQ_Out_1;
			delay_us(10000);
		}
	case 2:  //Generate 10ms via delay_ms call
		for(uint8_t i=0;i<2;i++){
			DQ_Out_0;
			delay_ms(10);
			DQ_Out_1;
			delay_ms(10);
		}
	default:
		break;
	}
}

void init_onewire_in(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = _18B20_PIN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_18B20_PIN_GPIO_Port, &GPIO_InitStruct);
}

void init_onewire_out(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = _18B20_PIN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(_18B20_PIN_GPIO_Port, &GPIO_InitStruct);
}

uint8_t DS18B20_Reset(void)
{
	uint8_t timeout=0;
	init_onewire_out();
	DQ_Out_0;
	delay_us(500);  //Minimum delay 480us ***
	DQ_Out_1;
	delay_us(60);  //Minimum delay 15us, 15us-60us
	init_onewire_in();
	/* Waiting for DS18B20 response */
	while(DQ_In)  //Waiting to pull down
	{
		delay_us(1);
		if(++timeout > 200)  //Time out
		 goto Exit;
	}
	timeout = 0;
	while(!DQ_In)  //Waiting to pull up
	{
		delay_us(1);
		if(++timeout > 240)  //Overtime
		 goto Exit;
	}
	return DS18B20_NO_ERR;  //Success

	Exit:
	return 0xFF;
}

inline static uint8_t _DS18B20_ReadBit(void)
{
	uint8_t bit = 0;
	init_onewire_out();  //The data line is configured as output
	DQ_Out_0;  //Pull down
	delay_us(2);  //The delay time is 2us, and it is required to be greater than 1 US
	DQ_Out_1;	  //Pull up
	init_onewire_in();  //The data line is configured as input
	delay_us(12);  //Delay at least 10us to start sampling bus data
	if(DQ_In)
		bit = 1;  //If high level, set 1
    delay_us(60);
	return bit;
}

static uint8_t _DS18B20_ReadByte(void)
{
	uint8_t byte_ = 0;
	for(uint8_t i=0;i<8;i++)
	{
		byte_ |= _DS18B20_ReadBit() << i;  //Read low bit first
	}
	return byte_;
}

inline static void _DS18B20_WriteBit(uint8_t bit)
{
  	init_onewire_out();  //The data line is configured as output
	if(bit)  //Write 1
	{
		DQ_Out_0;  //Pull down
		delay_us(15);  //Delay 15us, minimum 1us, send data start bit
		DQ_Out_1;  //Pull up and release the bus
		delay_us(60);  //The delay time is 60us, the minimum is 60us and the maximum is 120us
	}
	else  //Write 0
	{
		DQ_Out_0;  //Pull down
		delay_us(70);  //Minimum 60us ~ 120us
		DQ_Out_1;  //Pull up delay 2us
		delay_us(2);
	}
}

static void _DS18B20_WriteByte(uint8_t byte_)
{
	for(uint8_t i=0;i<8;i++)
	{
		_DS18B20_WriteBit(byte_ & 0x01);  //Mask LSB and write this bit
		byte_ >>= 1;  //There are 8 places in total, starting from the low position and then moving to the right
	}
}

static void _DS18B20_Write(const void* pBuf, uint8_t len)
{
	const char* rpBuf = pBuf;
	while(len--)  //Write bytes sequence according to its length
	{
		_DS18B20_WriteByte(*rpBuf++);
	}
}

uint8_t DS18B20_if_parasite(void)
{
	uint8_t power_mode;
	power_mode = 0;
    DS18B20_Reset();
    _DS18B20_WriteByte(ROM_SKIP);  //Skip ROM 0xCC
    _DS18B20_WriteByte(POWER_READ);  //Check power supply
    power_mode = _DS18B20_ReadByte();  //Return 0 if parasite power presence, else 0xFF
    return power_mode;
}

void DS18B20_when_parasite(uint8_t *power_mode, uint16_t t)
{
	if(*power_mode == 0xFF)  //If external power
    {
    	uint8_t done;
    	done = 0;
//    	while(!done)  //device transmit 1 when conversion done
//    	{
//    		delay_ms(10);
//    		done=_DS18B20_ReadByte();
//    	}
    	do  //device transmit 1 when conversion done, delay at least once
    	{
    		delay_ms(10);
    		done = _DS18B20_ReadByte();
    	}while(done == 0);
    }
    else
    {
    	if(*power_mode == 0)  //If parasite power
    	{
    		delay_ms(t);  //Set the period held high enough for the completion of temperature conversion or EEPROM copy
    	}
    	else
    	{
    		delay_ms(750);  //In case the power mode is not expected.
    	}
    }
}

int8_t DS18B20_SetConfig(const DS18B20_RomCode_t* pRom, const DS18B20_ConfigInfo_t* configInfo)
{
	uint8_t power_mode;
	power_mode = 0;  //Value 0 for parasite power
	power_mode = DS18B20_if_parasite();

	/* Write configuration to scratchpad for device of specific romCode */
	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;
	_DS18B20_WriteByte(ROM_MATCH);  //Match ROM 0x55
	_DS18B20_Write(pRom,sizeof(DS18B20_RomCode_t));
	_DS18B20_WriteByte(RAM_WRITE);
	_DS18B20_Write(configInfo,sizeof(DS18B20_ConfigInfo_t));

	DS18B20_Reset();
    _DS18B20_WriteByte(ROM_MATCH);  //Match ROM 0x55
    _DS18B20_Write(pRom,sizeof(DS18B20_RomCode_t));
    _DS18B20_WriteByte(RAM_TO_EE);  //Save the above 3 bytes value from scratchpad to eeprom
    DS18B20_when_parasite(&power_mode, 20);
    return DS18B20_NO_ERR;
}

int8_t DS18B20_GetConfig(const DS18B20_RomCode_t* pRom, DS18B20_ConfigInfo_t* configInfo)
{
	uint8_t power_mode;
	power_mode = 0;  //Value 0 for parasite power
	power_mode = DS18B20_if_parasite();

	/* For checking if this DS18B20 is with configuration feature */
	DS18B20_Reset();
	_DS18B20_WriteByte(ROM_MATCH);  //Match ROM 0x55
	_DS18B20_Write(pRom,sizeof(DS18B20_RomCode_t));
	_DS18B20_WriteByte(EE_TO_RAM);  //Copy EEPROM 3 bytes data to RAM (0xB8)
	DS18B20_when_parasite(&power_mode, 20);

	/* Fetch scratchpad data for device of specific romCode */
	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;
	_DS18B20_WriteByte(ROM_MATCH);  //Match ROM 0x55
	_DS18B20_Write(pRom,sizeof(DS18B20_RomCode_t));
	_DS18B20_WriteByte(RAM_READ);
	uint8_t buf[9];
	for(int i=0;i<9;i++)
	{
		buf[i] = _DS18B20_ReadByte();
	}
#if do_CRC == 1
		if(DS18B20_CRC(buf,8) != buf[8])
	{
		return DS18B20_ERR_CRC_VERIFY_FAILED;
	}
#endif

	configInfo->alarmTH = buf[2];
	configInfo->alarmTL = buf[3];
	configInfo->resolution = buf[4];
	return DS18B20_NO_ERR;
}

int8_t DS18B20_Read_romcode(DS18B20_RomCode_t* pRom)
{
	/* Fetch RomCode for single device attached */
	pRom->romCode = 0;
    DS18B20_Reset();
    _DS18B20_WriteByte(ROM_READ);  //Read ROM 0x33
    pRom->familyCode = _DS18B20_ReadByte();
    for(uint8_t i=0;i<6;i++)
    {
    	uint64_t data = _DS18B20_ReadByte();
    	pRom->serialNumber |= data <<= (i*8);
    }
    pRom->crcCode = _DS18B20_ReadByte();

    /* Check CRC of the RomCode */
	if(DS18B20_CRC(&pRom->romCode,7) == pRom->crcCode){
		return DS18B20_NO_ERR;
	}
	else{
		return DS18B20_ERR_CRC_VERIFY_FAILED;
	}
}

int8_t DS18B20_ConvertTemperatureAll(void)
{
	uint8_t power_mode;
	power_mode = 0;  //Value 0 for parasite power
	power_mode = DS18B20_if_parasite();

	/* Command all devices for temperature conversion */
	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;
	_DS18B20_WriteByte(ROM_SKIP);  //Skip ROM 0xCC
	_DS18B20_WriteByte(TEMPER_CONVERT);
	DS18B20_when_parasite(&power_mode, 750);  //Set longer delay for original DS18B20, when in parasite power
	return DS18B20_NO_ERR;
}

/* This method passing float data of actual temperature value.
 * Some adjustment for thermometer routine is needed.
 * Example:
 *   DS18B20_ReadTemperature(RomID, &temper_var);
 *   Temper_Var = ((int32_t)(temper_var*10000))/100;
 */
int8_t DS18B20_ReadTemperature(DS18B20_RomCode_t* pRomCode, float* pTemperature)
{
	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;

	/* Read temperature for device of specific RomCode */
	_DS18B20_WriteByte(ROM_MATCH);
	_DS18B20_Write(pRomCode,sizeof(DS18B20_RomCode_t));
	_DS18B20_WriteByte(RAM_READ);
	uint8_t buf[9];
	for(int i=0;i<9;i++)
	{
		buf[i] = _DS18B20_ReadByte();
	}
#if do_CRC == 1
	if(DS18B20_CRC(buf,8) != buf[8])
	{
		return DS18B20_ERR_CRC_VERIFY_FAILED;
	}
#endif

	*pTemperature = *((int16_t*)buf) / 16.F;

	return DS18B20_NO_ERR;
}

/* This method passing singed integer of temperature value which is done by shift bit conversion.
 * Some adjustment for thermometer routine is needed. Output is already *10000.
 * Example:
 *   DS18B20_ReadTemper(&RomID[0], &temper_var);
 *   Temper_Var = temper_var/100;
 */
int8_t DS18B20_ReadTemper(DS18B20_RomCode_t* pRomCode, int32_t* pTemperature)
{
    uint8_t sign,TH,TL,resolution;
    int32_t temper;

	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;

	/* Read temperature for device of specific RomCode */
	_DS18B20_WriteByte(ROM_MATCH);
	_DS18B20_Write(pRomCode,sizeof(DS18B20_RomCode_t));
	_DS18B20_WriteByte(RAM_READ);
	uint8_t buf[9];  //buf[0]=TL, buf[1]=TH, buf[4]=configuration
	for(int i=0;i<9;i++)
	{
		buf[i] = _DS18B20_ReadByte();
	}
#if do_CRC == 1
	if(DS18B20_CRC(buf,8) != buf[8])
	{
		return DS18B20_ERR_CRC_VERIFY_FAILED;
	}
#endif

    TL = buf[0];
	TH = buf[1];
    resolution = buf[4];

    temper = (TH << 8) + TL;
    if(TH > 7)  //Check if negative temperature, which is bit 11-15 of MS byte is 1
    {
        sign = 0;  //Which is negative temperature
        temper = (~temper) + 1;  //Find the complement, 2's complement method with singed integer
    }
    else sign = 1;  //Which is positive temperature

    switch(resolution & 0x60 >> 5)  //Extract bit6&5 of configuration register
    {
	/*------------------------------------
	Set resolution according to CONF value
	bit 4-11 represent whole number of temperature, bit 0-3 represent fractional value
	*10000 make the fractional value become whole number for calculation
	>>4 = 1/16 = *0.0625
	------------------------------------*/
    	case 0:
    		temper = ((temper * 10) >> 4) * 1000;  //9bits, 0.5C, get decimal point of 1 digit
            break;
        case 1:
        	temper = ((temper * 100) >> 4) * 100;  //10bits, 0.25C, get decimal point of 2 digits
            break;
        case 2:
        	temper = ((temper * 1000) >> 4) * 10;  //11bits, 0.125C, get decimal point of 3 digits
            break;
        case 3:
        	temper = ((temper * 10000) >> 4) * 1;  //12bits, 0.0625C, get decimal point of 4 digits
            break;
        default:
            break;
    }

    if(sign){  //Return value according to sign
        *pTemperature = temper;
    }
    else{
        *pTemperature = -temper;
    }

	return DS18B20_NO_ERR;
}

int8_t DS18b20_single_temper(int32_t* pTemperature)
{
    uint8_t sign,TH,TL;
    int32_t temper;

	if(!DS18B20_SUCCESSED(DS18B20_Reset()))
		return DS18B20_ERR_NO_PRESENCE;

	/* Read temperature for device of specific RomCode */
	_DS18B20_WriteByte(ROM_SKIP);
	_DS18B20_WriteByte(RAM_READ);
	uint8_t buf[9];
	for(int i=0;i<9;i++)
	{
		buf[i] = _DS18B20_ReadByte();
	}
#if do_CRC == 1
	if(DS18B20_CRC(buf,8) != buf[8])
	{
		return DS18B20_ERR_CRC_VERIFY_FAILED;
	}
#endif

    TL = buf[0];
	TH = buf[1];

    temper = (TH << 8) + TL;
    if(TH > 7)  //Check if negative temperature, which is bit 11-15 of MS byte is 1
    {
        sign = 0;  //Which is negative temperature
        temper = (~temper) + 1;  //Find the complement, 2's complement method with singed integer
    }
    else sign = 1;  //Which is positive temperature

    temper = ((temper * 10000) >> 4) * 1;

    if(sign){  //Return value according to sign
        *pTemperature = temper;
    }
    else{
        *pTemperature = -temper;
    }

	return DS18B20_NO_ERR;
}

static int8_t _DS18B20_Search(DS18B20_RomCode_t* pRomList, uint8_t* numberOfRom, uint8_t cmd)
{
	uint8_t count = 0;  //表示搜索到的数量
	uint64_t LastDiscrepancy = 0;  //有差异的位标记
	uint64_t LastZero = 0;  //轉向寫1的位標記

	if(!numberOfRom || !*numberOfRom ){  //If parameter is 0 or wrong pointer, then end ???
		*numberOfRom = count;
		return DS18B20_ERR_NO_DEVICES;
	}

	do{
		uint64_t romCode = 0;  //复位
		if(!DS18B20_SUCCESSED(DS18B20_Reset())){
			*numberOfRom = count;
			return DS18B20_ERR_NO_PRESENCE;
		}
		//发送搜索 ROM 命令
		_DS18B20_WriteByte(cmd);

		for(uint64_t bitMask =1; bitMask; bitMask <<=1)
		{
			//读两次位
			switch(_DS18B20_ReadBit() | (_DS18B20_ReadBit() << 1))
			{
				case 0x00:  //两次读的都是 0, 表示当前的位是存在差异的位
					{
						//判断是不是标记过
						if((LastDiscrepancy & bitMask) || (LastDiscrepancy > bitMask))  //Same effect as below??
//						if((LastDiscrepancy & bitMask) || ((LastDiscrepancy & ~((bitMask << 1) - 1))))
						{
							//标记过的, 说明不是第一次检测到这个差异位
							//判断当前的位是不是有差异的位中最高的那位
							if(LastDiscrepancy & ~((bitMask << 1) - 1))
							{
								if(LastZero & bitMask){  //到之前轉向位時, 寫入1
									romCode |= bitMask;
									_DS18B20_WriteBit(1);
								}
								else{
									//如果不是最高的差异位, 仍然先写入0, 仅跳过这个差异的位
									_DS18B20_WriteBit(0);
								}
							}
							else{
								//如果是最高的差异位, 因为第一次的时候写0, 所以这次写1, 并清除标记
								romCode |= bitMask;
								LastZero |= bitMask ;  //設定最後的轉向位
								LastDiscrepancy ^= bitMask;
								_DS18B20_WriteBit(1);
							}
						}
						else{
							//没有标记过, 第一次读到这个差异位
							//进行标记, 并先写入0
							LastDiscrepancy |= bitMask;
							_DS18B20_WriteBit(0);
						}
					}
					break;
				case 0x01:  //第一次读1, 第二次读0, 所以逻辑1
					romCode |= bitMask;
					_DS18B20_WriteBit(1);
					break;
				case 0x02:  //第一次读0, 第二次读1, 所以逻辑0
					_DS18B20_WriteBit(0);
					break;
				default:  // 0x03 这其实是通信异常
					*numberOfRom = count;
					return DS18B20_ERR_WAIT_TIMEOUT;
				}
		}
			//CRC 校验
			if(DS18B20_CRC(&romCode,7) == ((uint8_t*)&romCode)[7]){
				//校验通过, 保存结果, 计数加1
				pRomList[count].romCode = romCode;
				count++;
			}
			else{
				//校验失败, 错误 ROM 不进行处理
				//这行是为了count不加1 也不会影响循环, *numberOfRom 最后肯定是要被 count 赋值
				(*numberOfRom)--;
			}
	} while(count < *numberOfRom && LastDiscrepancy);

	*numberOfRom = count;
	return DS18B20_NO_ERR;
}

int8_t DS18B20_SearchRom(DS18B20_RomCode_t* pRomList, uint8_t* numberOfRom)
{
	return _DS18B20_Search(pRomList, numberOfRom, ROM_SEARCH);
}

int8_t DS18B20_SearchAlarm(DS18B20_RomCode_t* pRomList, uint8_t* numberOfRom)
{
	return _DS18B20_Search(pRomList, numberOfRom, ALARM_SEARCH);
}


/* Old code */
//int32_t ds18b20_get_temper(uint8_t _S_M, uint8_t *sensor)
//{
//    uint8_t sign,TH,TL;
//    int32_t temper;
//    power=ds18b20_if_parasite();
//
//    init_ds18b20();
//    ds18b20_write_byte(ROM_SKIP);  //Skip ROM 0xCC
//    ds18b20_write_byte(TEMPER_CONVERT);  //Temperature conversion
//    ds18b20_when_parastite(750);  //Longer delay for original DS18B20
//
//    ds18b20_RAM_rw(_S_M,RAM_READ,sensor);
//    TL=ds18b20_read_byte();  //LS byte
//    TH=ds18b20_read_byte();  //MS byte
//    ds18b20_data.TH=ds18b20_read_byte();
//    ds18b20_data.TL=ds18b20_read_byte();
//    ds18b20_data.CONF=ds18b20_read_byte();
//
//    temper = (TH << 8) + TL;
//    if(TH > 7)  //Check if negative temperature, which is bit 11-15 of MS byte is 1
//    {
//        sign = 0;  //Which is negative temperature
//        temper = (~temper) + 1;  //Find the complement, 2's complement method with singed integer
//    }
//    else sign = 1;  //Which is positive temperature
//
//    switch (ds18b20_data.CONF &0x60 >> 5)  //Extract bit6&5 of configuration register
//    {
//	/*------------------------------------
//	Set resolution according to CONF value
//	bit 4-11 represent whole number of temperature, bit 0-3 represent fractional value
//	*10000 make the fractional value become whole number for calculation
//	>>4 = 1/16 = *0.0625
//	------------------------------------*/
//    	case 0:
//    		temper = ((temper * 10) >> 4) * 1000;  //9bits, 0.5C, get decimal point of 1 digit
//            break;
//        case 1:
//        	temper = ((temper * 100) >> 4) * 100;  //10bits, 0.25C, get decimal point of 2 digits
//            break;
//        case 2:
//        	temper = ((temper * 1000) >> 4) * 10;  //11bits, 0.125C, get decimal point of 3 digits
//            break;
//        case 3:
//        	temper = ((temper * 10000) >> 4) * 1;  //12bits, 0.0625C, get decimal point of 4 digits
//            break;
//        default:
//            break;
//    }
//
//    if(sign)  //Return value according to sign
//        return temper;
//    else
//        return -temper;
//}
/* Old code end */



