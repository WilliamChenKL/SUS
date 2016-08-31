#include "S3000.H"
/*******************************************************************

EEPROM  相关程序
********************************************************************/
#define ENABLE_ISP 0x81 //系统工作时钟<24MHz 时，对IAP_CONTR 寄存器设置此值

void IAP_Disable()
{
	//关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
	//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
	IAP_CONTR = 0;      //关闭IAP 功能
	IAP_CMD = 0;      //清命令寄存器,使命令寄存器无命令,此句可不用
	IAP_TRIG = 0;      //清命令触发寄存器,使命令触发寄存器无触发,此句可不用
	IAP_ADDRH = 0;
	IAP_ADDRL = 0;
	_nop_(); _nop_();
}
//读一字节，调用前需打开IAP 功能，入口:DPTR = 字节地址，返回:A = 读出字节
uchar Byte_Read(uchar ADDH, uchar ADDL)//字节地址
{
	IAP_DATA = 0x00;
	IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
	IAP_CMD = 0x01;                 //IAP/ISP/EEPROM 字节读命令

	IAP_ADDRH = ADDH;    //设置目标单元地址的高8 位地址
	IAP_ADDRL = ADDL;    //设置目标单元地址的低8 位地址

	EA = 0;

	IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
	IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
	_nop_();

	EA = 1;
	IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
					//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
	return (IAP_DATA);
}

//字节编程，调用前需打开IAP 功能，入口:DPTR = 字节地址, A= 须编程字节的数据
void Byte_Program(uchar ADDH, uchar ADDL, uchar stdata)//字节地址
{
	IAP_CONTR = ENABLE_ISP;         //打开 IAP 功能, 设置Flash 操作等待时间
	IAP_CMD = 0x02;                 //IAP/ISP/EEPROM 字节编程命令

	IAP_ADDRH = ADDH;    //设置目标单元地址的高8 位地址
	IAP_ADDRL = ADDL;    //设置目标单元地址的低8 位地址址

	IAP_DATA = stdata;                  //要编程的数据先送进IAP_DATA 寄存器
	EA = 0;

	IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
	IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动
	_nop_();

	EA = 1;
	IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
					//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}
//擦除扇区, 入口:DPTR = 扇区地址
void Sector_Erase(uchar ADDH, uchar ADDL) //	扇区的首地址
{
	IAP_CONTR = ENABLE_ISP;         //打开IAP 功能, 设置Flash 操作等待时间
	IAP_CMD = 0x03;                 //IAP/ISP/EEPROM 扇区擦除命令

	IAP_ADDRH = ADDH;    //设置目标单元地址的高8 位地址
	IAP_ADDRL = ADDL;    //设置目标单元地址的低8 位地址址

	EA = 0;

	IAP_TRIG = 0x5A;   //先送 5Ah,再送A5h 到ISP/IAP 触发寄存器,每次都需如此
	IAP_TRIG = 0xA5;   //送完A5h 后，ISP/IAP 命令立即被触发起动

	_nop_(); _nop_();
	EA = 1;
	IAP_Disable();  //关闭IAP 功能, 清相关的特殊功能寄存器,使CPU 处于安全状态,
					//一次连续的IAP 操作完成之后建议关闭IAP 功能,不需要每次都关
}

/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 写一个扇区数据 sector_num是扇区序号
********************************************************************************/
void write_jiemian2_program_data(uchar idata sector_num)
{
	uchar idata  i = 0, b;
	temp_num[0] = shots;
	temp_num[1] = fluence;
	temp_num[2] = pulse_width_num[0];
	temp_num[3] = pulse_width_num[1];
	temp_num[4] = pulse_width_num[2];
	temp_num[5] = delay_width_num[0];
	temp_num[6] = delay_width_num[1];
	temp_num[7] = coolrate;
	temp_num[8] = RFinten;

	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);
	Sector_Erase(ADDRH, ADDRL);

	for (b = 0; b < 9; b++)
	{
		ADDRL = i;
		Byte_Program(ADDRH, ADDRL, temp_num[i]);
		i++;
	}
}
/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 读一个扇区数据 sector_num是扇区序号
********************************************************************************/

void read_jiemian2_program_data(uchar  sector_num)
{
	uchar idata  i = 0, b;
	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);

	for (b = 0; b < 9; b++)
	{
		ADDRL = i;
		temp_num[i] = Byte_Read(ADDRH, ADDRL);
		i++;
	}
	shots = temp_num[0];
	fluence = temp_num[1];
	pulse_width_num[0] = temp_num[2];
	pulse_width_num[1] = temp_num[3];
	pulse_width_num[2] = temp_num[4];
	delay_width_num[0] = temp_num[5];
	delay_width_num[1] = temp_num[6];
	coolrate = temp_num[7];
	RFinten = temp_num[8];
}

/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 写一个扇区数据 sector_num是扇区序号
********************************************************************************/
void write_jiemian1_program_data(uchar  sector_num)
{
	uchar idata  i = 0, b;
	temp_num[0] = fluence;
	temp_num[1] = fre_num;
	temp_num[2] = coolrate;
	temp_num[3] = energy_num / 100000;
	temp_num[4] = energy_num / 10000 % 10;
	temp_num[5] = energy_num / 1000 % 10;
	temp_num[6] = energy_num / 100 % 10;
	temp_num[7] = energy_num / 100 % 10;
	temp_num[8] = energy_num % 10;

	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);
	Sector_Erase(ADDRH, ADDRL);

	for (b = 0; b < 9; b++)
	{
		ADDRL = i;
		Byte_Program(ADDRH, ADDRL, temp_num[i]);
		i++;
	}
}
/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 读一个扇区数据 sector_num是扇区序号
********************************************************************************/

void read_jiemian1_program_data(uchar  sector_num)
{
	uchar idata  i = 0, b;
	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);

	for (b = 0; b < 9; b++)
	{
		ADDRL = i;
		temp_num[i] = Byte_Read(ADDRH, ADDRL);
		i++;
	}
	fluence = temp_num[0];
	fre_num = temp_num[1];
	coolrate = temp_num[2];
	energy_num = temp_num[3] * 100000 + temp_num[4] * 10000 + temp_num[5] * 1000 + temp_num[6] * 100 + temp_num[7] * 10 + temp_num[8];
}

/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 写一个扇区数据 sector_num是扇区序号
********************************************************************************/
void write_zongji_data(uchar  sector_num)
{
	uchar idata  i = 0, b;
	temp_num[0] = zongji / 10000000;
	temp_num[1] = zongji / 1000000 % 10;
	temp_num[2] = zongji / 100000 % 10;
	temp_num[3] = zongji / 10000 % 10;
	temp_num[4] = zongji / 1000 % 10;
	temp_num[5] = zongji / 100 % 10;
	temp_num[6] = zongji / 10 % 10;
	temp_num[7] = zongji % 10;

	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);
	Sector_Erase(ADDRH, ADDRL);

	for (b = 0; b < 8; b++)
	{
		ADDRL = i;
		Byte_Program(ADDRH, ADDRL, temp_num[i]);
		i++;
	}
}
/*******************************************************************************
 存储上次进入界面ARM on或ARM OFF的标志， 读一个扇区数据 sector_num是扇区序号
********************************************************************************/

void read_zongji_data(uchar  sector_num)
{
	uchar idata  i = 0, b;
	ADDRH = ((sector_num * 512) / 256); ADDRL = ((sector_num * 512) % 256);

	for (b = 0; b < 8; b++)
	{
		ADDRL = i;
		temp_num[i] = Byte_Read(ADDRH, ADDRL);
		i++;
	}
	zongji = temp_num[0] * 10000000 + temp_num[1] * 1000000 + temp_num[2] * 100000 + temp_num[3] * 10000 + temp_num[4] * 1000 + temp_num[5] * 100 + temp_num[6] * 10 + temp_num[7];
}

/*****************************************************************

 延时函数
 *******************************************************************/

void delay1ms()
{
	uchar a, b;
	for (a = 9; a > 0; a--)
	for (b = 100; b > 0; b--);
}

void delayms(uint x)
{
	uint i;
	for (i = 0; i <= x; i++) { delay1ms(); }
}
void delay()
{
	;;
}
void iicStart1()
{
	SDA1 = 1;
	delay();
	SCL1 = 1;
	delay();
	SDA1 = 0;
	delay();
	SCL1 = 0;
	delay();
}

void respons1()
{
	uchar i = 0;
	SCL1 = 1;
	delay();
	while ((SDA1 == 1) && (i < 255))
		i++;
	SCL1 = 0;
	delay();
}
//===========================================================
void iicStop1()
{
	SDA1 = 0;
	delay();
	SCL1 = 1;
	delay();
	SDA1 = 1;
	delay();
}

//写入一个数
void writebyte1(uchar date)
{
	uchar i, temp;
	temp = date;
	for (i = 0; i < 8; i++)
	{
		temp = temp << 1;
		SCL1 = 0;
		delay();
		SDA1 = CY;
		delay();
		SCL1 = 1;
		delay();
	}
	SCL1 = 0;
	delay();
	SDA1 = 1;
	delay();
}

//===========================================================
//读出一个数
uchar readbyte1()
{
	uchar i, K;
	SCL1 = 0;
	delay();
	SDA1 = 1;
	for (i = 0; i < 8; i++)
	{
		delay();
		SCL1 = 1;
		delay();
		K = (K << 1);
		if (SDA1) K |= 0x01;
		SCL1 = 0;
	}
	delay();
	return K;
}
void write_add1(uchar address, uchar date)
{
	iicStart1();
	writebyte1(0xa0);
	respons1();
	writebyte1(address);
	respons1();
	writebyte1(date);
	respons1();
	iicStop1();
	delayms(5);
}
uchar read_add1(uchar address)
{
	uchar date;
	iicStart1();
	writebyte1(0xa0);
	respons1();
	writebyte1(address);
	respons1();
	iicStart1();
	writebyte1(0xa1);
	respons1();
	date = readbyte1();
	iicStop1();
	delayms(2);
	return date;
}

void iicStart2()
{
	SDA2 = 1;
	delay();
	SCL2 = 1;
	delay();
	SDA2 = 0;
	delay();
	SCL2 = 0;
	delay();
}

void respons2()
{
	uchar i = 0;
	SCL2 = 1;
	delay();
	while ((SDA2 == 1) && (i < 255))
		i++;
	SCL2 = 0;
	delay();
}
//===========================================================
void iicStop2()
{
	SDA2 = 0;
	delay();
	SCL2 = 1;
	delay();
	SDA2 = 1;
	delay();
}

//写入一个数
void writebyte2(uchar date)
{
	uchar i, temp;
	temp = date;
	for (i = 0; i < 8; i++)
	{
		temp = temp << 1;
		SCL2 = 0;
		delay();
		SDA2 = CY;
		delay();
		SCL2 = 1;
		delay();
	}
	SCL2 = 0;
	delay();
	SDA2 = 1;
	delay();
}

//===========================================================
//读出一个数
uchar readbyte2()
{
	uchar i, K;
	SCL2 = 0;
	delay();
	SDA2 = 1;
	for (i = 0; i < 8; i++)
	{
		delay();
		SCL2 = 1;
		delay();
		K = (K << 1);
		if (SDA2) K |= 0x01;
		SCL2 = 0;
	}
	delay();
	return K;
}
void write_add2(uchar address, uchar date)
{
	iicStart2();
	writebyte2(0xa0);
	respons2();
	writebyte2(address);
	respons2();
	writebyte2(date);
	respons2();
	iicStop2();
	delayms(5);
}
uchar read_add2(uchar address)
{
	uchar date;
	iicStart2();
	writebyte2(0xa0);
	respons2();
	writebyte2(address);
	respons2();
	iicStart2();
	writebyte2(0xa1);
	respons2();
	date = readbyte2();
	iicStop2();
	delayms(2);
	return date;
}

void respons_1()
{
	uchar i = 0;
	SCL1 = 1;
	delay();
	while ((SDA1 == 1) && (i < 255))
		i++;
	if (SDA1 == 0) { hand1_flag = 1; }
	else { hand1_flag = 0; }
	SCL1 = 0;
	delay();
}
void respons_2()
{
	uchar i = 0;
	SCL2 = 1;
	delay();
	while ((SDA2 == 1) && (i < 255))
		i++;
	if (SDA2 == 0) { hand2_flag = 1; }
	else { hand2_flag = 0; }
	SCL2 = 0;
	delay();
}
void jiance_hand1()
{
	iicStart1();
	writebyte1(0xa0);
	respons_1();
	iicStop1();
}
void jiance_hand2()
{
	iicStart2();
	writebyte2(0xa0);
	respons_2();
	iicStop2();
}
void write_total1()
{
	temp10000000bit = total1_count / 10000000;
	temp1000000bit = total1_count / 1000000 % 10;
	temp100000bit = total1_count / 100000 % 10;
	temp10000bit = total1_count / 10000 % 10;
	temp1000bit = total1_count / 1000 % 10;
	temp100bit = total1_count / 100 % 10;
	temp10bit = total1_count / 10 % 10;
	tempbit = total1_count % 10;
	write_add1(1, temp10000000bit);
	write_add1(2, temp1000000bit);
	write_add1(3, temp100000bit);
	write_add1(4, temp10000bit);
	write_add1(5, temp1000bit);
	write_add1(6, temp100bit);
	write_add1(7, temp10bit);
	write_add1(8, tempbit);
}
void read_total1()
{
	temp10000000bit = read_add1(1);
	temp1000000bit = read_add1(2);
	temp100000bit = read_add1(3);
	temp10000bit = read_add1(4);
	temp1000bit = read_add1(5);
	temp100bit = read_add1(6);
	temp10bit = read_add1(7);
	tempbit = read_add1(8);
	total1_count = temp10000000bit * 10000000 + temp1000000bit * 1000000 + temp100000bit * 100000 + temp10000bit * 10000 + temp1000bit * 1000 + temp100bit * 100 + temp10bit * 10 + tempbit;
}
void write_total2()
{
	temp10000000bit = total2_count / 10000000;
	temp1000000bit = total2_count / 1000000 % 10;
	temp100000bit = total2_count / 100000 % 10;
	temp10000bit = total2_count / 10000 % 10;
	temp1000bit = total2_count / 1000 % 10;
	temp100bit = total2_count / 100 % 10;
	temp10bit = total2_count / 10 % 10;
	tempbit = total2_count % 10;
	write_add2(1, temp10000000bit);
	write_add2(2, temp1000000bit);
	write_add2(3, temp100000bit);
	write_add2(4, temp10000bit);
	write_add2(5, temp1000bit);
	write_add2(6, temp100bit);
	write_add2(7, temp10bit);
	write_add2(8, tempbit);
}
void read_total2()
{
	temp10000000bit = read_add2(1);
	temp1000000bit = read_add2(2);
	temp100000bit = read_add2(3);
	temp10000bit = read_add2(4);
	temp1000bit = read_add2(5);
	temp100bit = read_add2(6);
	temp10bit = read_add2(7);
	tempbit = read_add2(8);
	total2_count = temp10000000bit * 10000000 + temp1000000bit * 1000000 + temp100000bit * 100000 + temp10000bit * 10000 + temp1000bit * 1000 + temp100bit * 100 + temp10bit * 10 + tempbit;
}
/**************************************************************************************************************************************************
  LCD通信显示串口
**************************************************************************************************************************************************/

//*************************************************************************************************
//函    数：void Uart_transmit(uchar i)
//功能描述：由串口发送单字节数据
//*************************************************************************************************

void Uart_transmit(uchar i)
{
	ES = 0;
	TI = 0;
	SBUF = i;
	while (TI == 0);
	TI = 0;
	ES = 1;
}

//*************************************************************************************************
//函    数：void send_str(uchar *p)
//功能描述：由串口发送一个固定的数据串，0xFE为数据串结尾。
//*************************************************************************************************

void send_str(uchar *p)
{
	for (; *p != 0xFE; p++)
	{
		Uart_transmit(*p);
	}
}

//*************************************************************************************************
//函    数：void en(void)
//功能描述：发送迪文指令帧尾CC 33 C3 3C
//*************************************************************************************************

void end_fram(void)
{
	send_str(CMD_FRAM_END);
}

//*************************************握手指令****************

void send_ack()
{
	RXFRMOK = 0;
	do 
	{
		send_str(CMD_ACK);
		delayms(100); delayms(100);
	} 
	while (RXFRMOK == 0);
	RXFRMOK = 0;
}

/*******************************************************************************************************
  显示一副图片：pi_id是存储图片的序号 ,最多256张图片
******************************************************************************************************/

void picture_display(uchar pi_id)
{
	Uart_transmit(0xAA);
	Uart_transmit(0x70);
	Uart_transmit(pi_id);
	end_fram();
}

/******************************************************************************************************************
剪切一幅图：Xsh,Xsl,Ysh,Ysl是要剪切图片的左上角，Xeh,Xel,Yeh,Yel是要剪切图片的右下角，
Xh,Xl,Yh,Yl是当前图片的位置;pi_id图片存储位置
********************************************************************************************************************/

void cut_picture(uchar pi_id, uint Xs, uint Ys, uint Xe, uint Ye, uint X, uint Y)
{
	uchar Xsh, Xsl, Ysh, Ysl;
	uchar Xeh, Xel, Yeh, Yel;
	uchar Xh, Xl, Yh, Yl;

	Xsh = (Xs / 256); Xsl = (Xs % 256); Ysh = (Ys / 256); Ysl = (Ys % 256);
	Xeh = (Xe / 256); Xel = (Xe % 256); Yeh = (Ye / 256); Yel = (Ye % 256);
	Xh = (X / 256); Xl = (X % 256); Yh = (Y / 256); Yl = (Y % 256);

	Uart_transmit(0xAA);
	Uart_transmit(0x71);
	Uart_transmit(pi_id);
	Uart_transmit(Xsh);
	Uart_transmit(Xsl);
	Uart_transmit(Ysh);
	Uart_transmit(Ysl);
	Uart_transmit(Xeh);
	Uart_transmit(Xel);
	Uart_transmit(Yeh);
	Uart_transmit(Yel);
	Uart_transmit(Xh);
	Uart_transmit(Xl);
	Uart_transmit(Yh);
	Uart_transmit(Yl);
	end_fram();
}

//************串口通信************************************************************

void UART_ISR(void) interrupt 4
{
	uchar i;

	if (RI)
	{
		i = SBUF;

		RI = 0;
		if (RXFRMOK == 0)
		{
			if (RXAAOK)
			{
				rec[LEN] = i;
				LEN++;
				if (LEN > 4)
				{
					if (rec[LEN - 4] == 0xCC && rec[LEN - 3] == 0x33 && rec[LEN - 2] == 0xC3 && rec[LEN - 1] == 0x3C)
					{
						RXFRMOK = 1;
						RXAAOK = 0;
					}
				}
			}
			else
			{
				if (i == 0xAA)
				{
					RXAAOK = 1;
					LEN = 0;
				}
			}
		}
	}
	else 
	{ 
		TI = 0; 
	}
}

/********************************************************************************************************************
									  DA转换模块结束
*********************************************************************************************************************/

/*----------------------------
ADC interrupt service routine
----------------------------*/

void adc_isr() interrupt 5 using 1
{
	ADC_CONTR = ADC_CONTR & 0xef;         //Clear ADC interrupt flag
	water_temp = ((100 * ADC_RES) / 256);	   //100t=(5*code/256)*1000,t是摄氏温度，water_temp=10t,方便显示小数点后一位	
	if (houtai_flag || interface_flag) { dongtai_flag = 1; }
}

/*****************************************************************
   温度数据处理及显示
******************************************************************/

void tempdata_deal_display(float temp_num)
{
	uchar idata temp100bit, temp10bit, tempbit;
	uint dsnum;
	dsnum = 10 * temp_num;
	temp100bit = dsnum / 100;
	temp10bit = (dsnum / 10) % 10;
	tempbit = dsnum % 10;
	cut_picture(temp100bit + 35, water_temp_100bit_xs, water_temp_100bit_ys, water_temp_100bit_xe, water_temp_100bit_ye, water_temp_100bit_xs, water_temp_100bit_ys);
	cut_picture(temp10bit + 35, water_temp_10bit_xs, water_temp_10bit_ys, water_temp_10bit_xe, water_temp_10bit_ye, water_temp_10bit_xs, water_temp_10bit_ys);
}
//*****************************************************water flow rate display *************************
void flow_rate_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 35, flowrate_10bit_xs, flowrate_10bit_ys, flowrate_10bit_xe, flowrate_10bit_ye, flowrate_10bit_xs, flowrate_10bit_ys);
	cut_picture(b + 35, flowrate_bit_xs, flowrate_bit_ys, flowrate_bit_xe, flowrate_bit_ye, flowrate_bit_xs, flowrate_bit_ys);
}

void RFinten_display(uchar tempnum)
{
	uchar idata temp10bit, tempbit;
	temp10bit = tempnum / 10;
	tempbit = tempnum % 10;

	cut_picture(temp10bit + 11, RFinten10bit_xs, RFinten10bit_ys, RFinten10bit_xe, RFinten10bit_ye, RFinten10bit_xs, RFinten10bit_ys);
	cut_picture(tempbit + 11, RFintenbit_xs, RFintenbit_ys, RFintenbit_xe, RFintenbit_ye, RFintenbit_xs, RFintenbit_ys);
}

void total_display(ulong tempnum)
{
	temp10000000bit = tempnum / 10000000;
	temp1000000bit = (tempnum / 1000000) % 10;
	temp100000bit = (tempnum / 100000) % 10;
	temp10000bit = (tempnum / 10000) % 10;
	temp1000bit = (tempnum / 1000) % 10;
	temp100bit = (tempnum / 100) % 10;
	temp10bit = (tempnum / 10) % 10;
	tempbit = tempnum % 10;
	cut_picture(temp10000000bit + 11, shottotal_10000000bit_xs, shottotal_10000000bit_ys, shottotal_10000000bit_xe, shottotal_10000000bit_ye, shottotal_10000000bit_xs, shottotal_10000000bit_ys);
	cut_picture(temp1000000bit + 11, shottotal_1000000bit_xs, shottotal_1000000bit_ys, shottotal_1000000bit_xe, shottotal_1000000bit_ye, shottotal_1000000bit_xs, shottotal_1000000bit_ys);
	cut_picture(temp100000bit + 11, shottotal_100000bit_xs, shottotal_100000bit_ys, shottotal_100000bit_xe, shottotal_100000bit_ye, shottotal_100000bit_xs, shottotal_100000bit_ys);
	cut_picture(temp10000bit + 11, shottotal_10000bit_xs, shottotal_10000bit_ys, shottotal_10000bit_xe, shottotal_10000bit_ye, shottotal_10000bit_xs, shottotal_10000bit_ys);
	cut_picture(temp1000bit + 11, shottotal_1000bit_xs, shottotal_1000bit_ys, shottotal_1000bit_xe, shottotal_1000bit_ye, shottotal_1000bit_xs, shottotal_1000bit_ys);
	cut_picture(temp100bit + 11, shottotal_100bit_xs, shottotal_100bit_ys, shottotal_100bit_xe, shottotal_100bit_ye, shottotal_100bit_xs, shottotal_100bit_ys);
	cut_picture(temp10bit + 11, shottotal_10bit_xs, shottotal_10bit_ys, shottotal_10bit_xe, shottotal_10bit_ye, shottotal_10bit_xs, shottotal_10bit_ys);
	cut_picture(tempbit + 11, shottotal_bit_xs, shottotal_bit_ys, shottotal_bit_xe, shottotal_bit_ye, shottotal_bit_xs, shottotal_bit_ys);
}

void shotonce_display(ulong tempnum)
{
	temp1000000bit = (tempnum / 1000000) % 10;
	temp100000bit = (tempnum / 100000) % 10;
	temp10000bit = (tempnum / 10000) % 10;
	temp1000bit = (tempnum / 1000) % 10;
	temp100bit = (tempnum / 100) % 10;
	temp10bit = (tempnum / 10) % 10;
	tempbit = tempnum % 10;
	cut_picture(temp1000000bit + 11, shotonce_1000000bit_xs, shotonce_1000000bit_ys, shotonce_1000000bit_xe, shotonce_1000000bit_ye, shotonce_1000000bit_xs, shotonce_1000000bit_ys);
	cut_picture(temp100000bit + 11, shotonce_100000bit_xs, shotonce_100000bit_ys, shotonce_100000bit_xe, shotonce_100000bit_ye, shotonce_100000bit_xs, shotonce_100000bit_ys);
	cut_picture(temp10000bit + 11, shotonce_10000bit_xs, shotonce_10000bit_ys, shotonce_10000bit_xe, shotonce_10000bit_ye, shotonce_10000bit_xs, shotonce_10000bit_ys);
	cut_picture(temp1000bit + 11, shotonce_1000bit_xs, shotonce_1000bit_ys, shotonce_1000bit_xe, shotonce_1000bit_ye, shotonce_1000bit_xs, shotonce_1000bit_ys);
	cut_picture(temp100bit + 11, shotonce_100bit_xs, shotonce_100bit_ys, shotonce_100bit_xe, shotonce_100bit_ye, shotonce_100bit_xs, shotonce_100bit_ys);
	cut_picture(temp10bit + 11, shotonce_10bit_xs, shotonce_10bit_ys, shotonce_10bit_xe, shotonce_10bit_ye, shotonce_10bit_xs, shotonce_10bit_ys);
	cut_picture(tempbit + 11, shotonce_bit_xs, shotonce_bit_ys, shotonce_bit_xe, shotonce_bit_ye, shotonce_bit_xs, shotonce_bit_ys);
}

void houtai_total_display(ulong tempnum)				 //后台脉冲个数记录显示
{
	temp10000000bit = tempnum / 10000000;
	temp1000000bit = (tempnum / 1000000) % 10;
	temp100000bit = (tempnum / 100000) % 10;
	temp10000bit = (tempnum / 10000) % 10;
	temp1000bit = (tempnum / 1000) % 10;
	temp100bit = (tempnum / 100) % 10;
	temp10bit = (tempnum / 10) % 10;
	tempbit = tempnum % 10;
	cut_picture(temp10000000bit + 35, shottotal2_10000000bit_xs, shottotal2_10000000bit_ys, shottotal2_10000000bit_xe, shottotal2_10000000bit_ye, shottotal2_10000000bit_xs, shottotal2_10000000bit_ys);
	cut_picture(temp1000000bit + 35, shottotal2_1000000bit_xs, shottotal2_1000000bit_ys, shottotal2_1000000bit_xe, shottotal2_1000000bit_ye, shottotal2_1000000bit_xs, shottotal2_1000000bit_ys);
	cut_picture(temp100000bit + 35, shottotal2_100000bit_xs, shottotal2_100000bit_ys, shottotal2_100000bit_xe, shottotal2_100000bit_ye, shottotal2_100000bit_xs, shottotal2_100000bit_ys);
	cut_picture(temp10000bit + 35, shottotal2_10000bit_xs, shottotal2_10000bit_ys, shottotal2_10000bit_xe, shottotal2_10000bit_ye, shottotal2_10000bit_xs, shottotal2_10000bit_ys);
	cut_picture(temp1000bit + 35, shottotal2_1000bit_xs, shottotal2_1000bit_ys, shottotal2_1000bit_xe, shottotal2_1000bit_ye, shottotal2_1000bit_xs, shottotal2_1000bit_ys);
	cut_picture(temp100bit + 35, shottotal2_100bit_xs, shottotal2_100bit_ys, shottotal2_100bit_xe, shottotal2_100bit_ye, shottotal2_100bit_xs, shottotal2_100bit_ys);
	cut_picture(temp10bit + 35, shottotal2_10bit_xs, shottotal2_10bit_ys, shottotal2_10bit_xe, shottotal2_10bit_ye, shottotal2_10bit_xs, shottotal2_10bit_ys);
	cut_picture(tempbit + 35, shottotal2_bit_xs, shottotal2_bit_ys, shottotal2_bit_xe, shottotal2_bit_ye, shottotal2_bit_xs, shottotal2_bit_ys);
}

/*****************************************************pulsewidth display *************************/
void pulsewidth1_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, pulsewidth1_10bit_xs, pulsewidth1_10bit_ys, pulsewidth1_10bit_xe, pulsewidth1_10bit_ye, pulsewidth1_10bit_xs, pulsewidth1_10bit_ys);
	cut_picture(b + 11, pulsewidth1_bit_xs, pulsewidth1_bit_ys, pulsewidth1_bit_xe, pulsewidth1_bit_ye, pulsewidth1_bit_xs, pulsewidth1_bit_ys);
}

void pulsewidth2_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, pulsewidth2_10bit_xs, pulsewidth2_10bit_ys, pulsewidth2_10bit_xe, pulsewidth2_10bit_ye, pulsewidth2_10bit_xs, pulsewidth2_10bit_ys);
	cut_picture(b + 11, pulsewidth2_bit_xs, pulsewidth2_bit_ys, pulsewidth2_bit_xe, pulsewidth2_bit_ye, pulsewidth2_bit_xs, pulsewidth2_bit_ys);
}

void pulsewidth3_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, pulsewidth3_10bit_xs, pulsewidth3_10bit_ys, pulsewidth3_10bit_xe, pulsewidth3_10bit_ye, pulsewidth3_10bit_xs, pulsewidth3_10bit_ys);
	cut_picture(b + 11, pulsewidth3_bit_xs, pulsewidth3_bit_ys, pulsewidth3_bit_xe, pulsewidth3_bit_ye, pulsewidth3_bit_xs, pulsewidth3_bit_ys);
}

/*****************************************************delaywidth display *************************/
void delaywidth1_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, delaywidth1_10bit_xs, delaywidth1_10bit_ys, delaywidth1_10bit_xe, delaywidth1_10bit_ye, delaywidth1_10bit_xs, delaywidth1_10bit_ys);
	cut_picture(b + 11, delaywidth1_bit_xs, delaywidth1_bit_ys, delaywidth1_bit_xe, delaywidth1_bit_ye, delaywidth1_bit_xs, delaywidth1_bit_ys);
}

void delaywidth2_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, delaywidth2_10bit_xs, delaywidth2_10bit_ys, delaywidth2_10bit_xe, delaywidth2_10bit_ye, delaywidth2_10bit_xs, delaywidth2_10bit_ys);
	cut_picture(b + 11, delaywidth2_bit_xs, delaywidth2_bit_ys, delaywidth2_bit_xe, delaywidth2_bit_ye, delaywidth2_bit_xs, delaywidth2_bit_ys);
}

/*****************************************************fluence1 display *************************/
void fluence1_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 11, fluence1_10bit_xs, fluence1_10bit_ys, fluence1_10bit_xe, fluence1_10bit_ye, fluence1_10bit_xs, fluence1_10bit_ys);
	cut_picture(b + 11, fluence1_bit_xs, fluence1_bit_ys, fluence1_bit_xe, fluence1_bit_ye, fluence1_bit_xs, fluence1_bit_ys);
}

/*****************************************************fluence2 display *************************/
void fluence2_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 24, fluence2_10bit_xs, fluence2_10bit_ys, fluence2_10bit_xe, fluence2_10bit_ye, fluence2_10bit_xs, fluence2_10bit_ys);
	cut_picture(b + 24, fluence2_bit_xs, fluence2_bit_ys, fluence2_bit_xe, fluence2_bit_ye, fluence2_bit_xs, fluence2_bit_ys);
}

/*****************************************************fre display *************************/
void fre_display(uchar V)
{
	a = V / 10;
	b = V % 10;
	cut_picture(a + 24, fre2_10bit_xs, fre2_10bit_ys, fre2_10bit_xe, fre2_10bit_ye, fre2_10bit_xs, fre2_10bit_ys);
	cut_picture(b + 24, fre2_bit_xs, fre2_bit_ys, fre2_bit_xe, fre2_bit_ye, fre2_bit_xs, fre2_bit_ys);
}

/*****************************************************Energy display *************************/
void Energy_display(ulong tempnum)
{
	temp100000bit = (tempnum / 100000) % 10;
	temp10000bit = (tempnum / 10000) % 10;
	temp1000bit = (tempnum / 1000) % 10;
	temp100bit = (tempnum / 100) % 10;
	temp10bit = (tempnum / 10) % 10;
	tempbit = tempnum % 10;
	cut_picture(temp100000bit + 24, Energy_100000bit_xs, Energy_100000bit_ys, Energy_100000bit_xe, Energy_100000bit_ye, Energy_100000bit_xs, Energy_100000bit_ys);
	cut_picture(temp10000bit + 24, Energy_10000bit_xs, Energy_10000bit_ys, Energy_10000bit_xe, Energy_10000bit_ye, Energy_10000bit_xs, Energy_10000bit_ys);
	cut_picture(temp1000bit + 24, Energy_1000bit_xs, Energy_1000bit_ys, Energy_1000bit_xe, Energy_1000bit_ye, Energy_1000bit_xs, Energy_1000bit_ys);
	cut_picture(temp100bit + 24, Energy_100bit_xs, Energy_100bit_ys, Energy_100bit_xe, Energy_100bit_ye, Energy_100bit_xs, Energy_100bit_ys);
	cut_picture(temp10bit + 24, Energy_10bit_xs, Energy_10bit_ys, Energy_10bit_xe, Energy_10bit_ye, Energy_10bit_xs, Energy_10bit_ys);
	cut_picture(tempbit + 24, Energy_bit_xs, Energy_bit_ys, Energy_bit_xe, Energy_bit_ye, Energy_bit_xs, Energy_bit_ys);
}

/*****************************************************fluence1 quang display *************************/
void fluence1_quang_display(uchar V)
{
	cut_picture(V + 11, 340, 210, 380, 360, 340, 210);
	cut_picture(V + 11, 380, 210, 485, 265, 380, 210);
	cut_picture(V + 11, 380, 315, 485, 360, 380, 315);
	cut_picture(V + 11, 445, 265, 485, 315, 445, 265);
}

/*****************************************************fluence2 quang display *************************/
void fluence2_quang_display(uchar V)
{
	cut_picture(V + 24, 70, 210, 115, 360, 70, 210);
	cut_picture(V + 24, 115, 210, 225, 265, 115, 210);
	cut_picture(V + 24, 115, 315, 225, 360, 115, 315);
	cut_picture(V + 24, 180, 265, 225, 315, 180, 265);
}

/*****************************************************fre quang display *************************/
void fre_quang_display(uchar V)
{
	cut_picture(V + 24, 325, 210, 365, 360, 325, 210);
	cut_picture(V + 24, 365, 210, 475, 265, 365, 210);
	cut_picture(V + 24, 365, 315, 475, 360, 365, 315);
	cut_picture(V + 24, 435, 265, 475, 315, 435, 265);
}

/*****************************************************cooling quang display *************************/
void cooling_quang_display(uchar V)
{
	cut_picture(V + 11, 565, 210, 620, 360, 565, 210);
	cut_picture(V + 11, 620, 210, 715, 265, 620, 210);
	cut_picture(V + 11, 620, 315, 715, 360, 620, 315);
	cut_picture(V + 11, 660, 265, 715, 315, 660, 265);
}

/***********************************************************************************************************************************************
											  初始化结束
************************************************************************************************************************************************/

/**********************************************************************************************************
   手柄控制
*****************************************************************************************************************/
void key_scan()
{
	if (ready_flag == 1 && YRFKCTRL == 0)
	{
		if (HANDLKEY == 0 && key_flag == 0)
		{
			delayms(30);     
			if (HANDLKEY == 0)
			{
			  if(onlight_flag==0)
			  {
				if(bijiao_flag==0)
				{
					delayms(1000); b = 0; shaomiao_flag = 1;onlight_flag=1;
					key_flag = 1; open_flag = 1; sss_flag = 1;
					PULSEcount = 0;
					PULSE_PWM = 1;
					ppcount = 0;
					if (RFstart_flag)
					{
						RFonoff = 0;
					}
					temp_count = 0;
					TH1 = 0X00;//计数器
					TL1 = 0X00;
					pulseout_count = 0; BEEP = 0; pulseout_beep_flag = 1;
					return;
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			 }
			}
		}
		else if (HANDLKEY == 1 && key_flag == 1)
		{
			delayms(30);
			if (HANDLKEY == 1)
			{
				key_flag = 0; open_flag = 0; 
				if (interface_flag == 1)
				{
				onlight_flag=0;
				sss_flag = 0;
				PULSE_PWM = 1;
				PULSEcount = 0;
				ppcount = 0;
				yy_flag = 0;
				write_zongji_data(6);
				delayms(100);				
				write_total2();
				temp_count = 0;
				TH1 = 0X00;//计数器
				TL1 = 0X00;
				flowrate = 15;
				shaomiao_flag = 0;
				miao_flag = 0;
				hand1_flag = 1;
				hand2_flag = 1;
			    ttbeep_count = 0; BEEP = 0; ttbeep_flag = 1; return;
			   }
			}
		}
	}
	else {
		YRXHCTRL = 1;//待机

	}
	if (HANDLKEY == 0 && key_flag == 0 && YRFKCTRL == 1)
	{
		delayms(20);
		if (HANDLKEY == 0)
		{
			key_flag = 1;
			if (RFstart_flag)
			{
				RFonoff = 0;
			}
			beep_count = 0; BEEP = 0; beep_flag = 1; return;
		}
	}
	else if (HANDLKEY == 1 && key_flag == 1 && YRFKCTRL == 1)
	{
		delayms(20);
		if (HANDLKEY == 1)
		{
			key_flag = 0;
			RFonoff = 1;
			beep_count = 0; BEEP = 0; beep_flag = 1; return;
		}
	}
}

//*********************************RFinten_choice*************************************

void RFinten_choice(uchar num)
{
	switch (num)
	{
	case 0: {RP1 = 0; RP2 = 0; RP3 = 0; RP4 = 0; break; }
	case 1: {RP1 = 0; RP2 = 1; RP3 = 0; RP4 = 0; break; }
	case 2: {RP1 = 0; RP2 = 0; RP3 = 1; RP4 = 0; break; }
	case 3: {RP1 = 0; RP2 = 0; RP3 = 0; RP4 = 1; break; }
	case 4: {RP1 = 1; RP2 = 0; RP3 = 0; RP4 = 1; break; }
	case 5: {RP1 = 0; RP2 = 1; RP3 = 0; RP4 = 1; break; }
	case 6: {RP1 = 0; RP2 = 0; RP3 = 1; RP4 = 1; break; }
	default: {break; }
	}
}

void light_time_choice(uchar num)							 //调快速出光的对应档位的快慢	设置多少就是停顿充电的时间
{
	switch (num)
	{
	case 1: {light_time = 500; break; }
	case 2: {light_time = 450; break; }
	case 3: {light_time = 400; break; }
	case 4: {light_time = 350; break; }
	case 5: {light_time = 300; break; }
	case 6: {light_time = 250; break; }
	case 7: {light_time = 200; break; }
	case 8: {light_time = 150; break; }
	case 9: {light_time = 100; break; }
	case 10: {light_time = 80; break; }
	default: {break; }
	}
}
void fluence_num_max_choice(uchar num)							 //调频率限制能量的可调的大小
{
	switch (num)
	{
	case 1: {fluence_num_max= 60; break; }				 //频率1 能量最大可调60
	case 2: {fluence_num_max = 50; break; }
	case 3: {fluence_num_max = 45; break; }
	case 4: {fluence_num_max = 40; break; }
	case 5: {fluence_num_max = 35; break; }
	case 6: {fluence_num_max = 25; break; }
	case 7: {fluence_num_max = 20; break; }
	case 8: {fluence_num_max = 15; break; }
	case 9: {fluence_num_max = 10; break; }
	case 10: {fluence_num_max = 5; break; }			   //频率10 能量最大可调5
	default: {break; }
	}
}
/***********************************************************************************************************************************************
											  key check start
**************************************************************************************************************************************************/
void keyport()
{
	key_scan();
	//LCD KEY SCAN
	if (RXFRMOK)
	{
		RXFRMOK = 0;
		if (rec[0] == 0x79)
		{

			if ((rec[1] == 0x0b) && (rec[2] == 0x01))//进入SHR主界面
			{
				if (ready_flag == 0)
				{
					IPL = 1; SHR = 0; set_flag = 0; progarm = 0; qingling = 0; yuran_flag = 0;
					picture_display(22);//SHR主界面
					jiemian2_progarm_display_variable(3);
					delayms(100); delayms(100); delayms(100);
					jiemian2_progarm_display_variable(3);
					delayms(100); delayms(100); delayms(100);
					BEEP = 0; delayms(300); BEEP = 1;
					interface_flag = 1; return;
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			}
			else if ((rec[1] == 0x0b) && (rec[2] == 0x05))//进入BURST主界面
			{
				if (ready_flag == 0)
				{
					IPL = 0; SHR = 1; set_flag = 0; qingling = 0;
					progarm = 0; yuran_flag = 0;
					add_reduce_flag = 0;
					bijiao_flag = 0;
					energy_woke = 0;
					picture_display(9);//BURST主界面
					jiemian1_progarm_display_variable(0);
					delayms(100); delayms(100); delayms(100);
					jiemian1_progarm_display_variable(0);
					delayms(300);
					BEEP = 0; delayms(300); BEEP = 1;
					interface_flag = 2; return;
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			}
			else if ((rec[1] == 0x0b) && (rec[2] == 0x07))//BACK
			{
				if (ready_flag == 0)
				{
					IPL = 1; SHR = 1; yuran_flag = 0;
					add_reduce_flag = 0;
					bijiao_flag = 0; qingling = 0;
					energy_woke = 0;
					picture_display(8);
					interface_flag = 0;
					RFstart_flag = 0;
					RFonoff = 1;
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0xA5) && (rec[2] == 0xA5)) //Standby Or Ready
			{
				if (set_flag == 0)
				{
					ready_flag = !ready_flag;
					if (ready_flag)
					{
						qingling = 0;
						yuran_flag = 1;
						cool_count = 0;
						YRXHCTRL = 0;//预燃
						delayms(500);
						YRXHCTRL = 0;//预燃
						delayms(500);
						YRXHCTRL = 0;//预燃
						delayms(500);
						if (interface_flag == 2)
						{
							CCAP1H = CCAP1L = cap_num; //BURST充电	
						}
						else if (interface_flag == 1)
						{
							CCAP1H = CCAP1L = cap_num;//SHR充电
						}

						cut_picture(10, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
						cut_picture(10, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
						cut_picture(10, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);

						beep_count = 0; BEEP = 0; beep_flag = 1;
						delayms(500);
						temp_count = 0;
						TH1 = 0X00;//计数器
						TL1 = 0X00;
						return;
					}
					else 
					{
						if (yuran_flag == 1 && interface_flag == 1)
						{
							yuran_flag = 0;
							add_reduce_flag = 0;
							energy_woke = 0;
							bijiao_flag = 0; qingling = 0;
							Energy_display(energy_woke);
						}
						key_flag = 0; open_flag = 0; PULSEcount = 0;
						YRXHCTRL = 1;//待机				
						delayms(500);
						YRXHCTRL = 1;//待机				
						delayms(500);
						YRXHCTRL = 1;//待机				
						delayms(500);

						PULSE_PWM = 1;
						CCAP1H = CCAP1L = 0;//停止充电	

						cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
						cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
						cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
						write_zongji_data(6);
		                write_total1();				
		                write_total2();
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}

			//***********************************************************************profession  E light*****************************************
			else if ((rec[1] == 0x04) && (rec[2] == 0x05))//SHR And BURST Fluence +
			{
				if (set_flag == 0)
				{
					if (fluence<fluence_num_max)
					{
						fluence++;
						qingling = 0;
						quang = (fluence - 1) / 6;
						if (interface_flag == 2)
						{
							cap_num = 44 + 2 * fluence;
							fluence1_quang_display(quang);
							fluence1_display(fluence);
						}
						else if (interface_flag == 1)
						{
							cap_num = 44 + 2 * fluence;
							fluence2_quang_display(quang);
							fluence2_display(fluence);
							energy_fa = fluence;
						}
						if (ready_flag)
						{
							CCAP1H = CCAP1L = cap_num;
						}
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}

			}
			else if ((rec[1] == 0x04) && (rec[2] == 0x02))//SHR And BURST Fluence -
			{
				if (set_flag == 0)
				{
					if (fluence > 1)
					{
						fluence--; qingling = 0;
						quang = (fluence - 1) / 6;
						if (interface_flag == 2)
						{
							cap_num = 44 + 2 * fluence;
							fluence1_quang_display(quang);
							fluence1_display(fluence);
						}
						else if (interface_flag == 1)
						{
							cap_num = 44 + 2 * fluence;
							fluence2_quang_display(quang);
							fluence2_display(fluence);
							energy_fa = fluence;
						}
						if (ready_flag)
						{
							CCAP1H = CCAP1L = cap_num;
						}
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}

			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x04))//SHR Frequency +
			{
				if (set_flag == 0)
				{
					if (fre_num < 10)
					{
						fre_num++; qingling = 0;
						light_time_choice(fre_num);
						quang = fre_num - 1;
						fre_quang_display(quang);
						fre_display(fre_num);
						fluence_num_max_choice(fre_num);
						if(fluence>fluence_num_max)
						{
						  fluence=fluence_num_max;
						  cap_num = 44 + 2 * fluence;
						  quang = (fluence - 1) / 6;
						  fluence2_quang_display(quang);
						  fluence2_display(fluence);
						  energy_fa = fluence;
						  if (ready_flag)
						  {
							CCAP1H = CCAP1L = cap_num;
						  }
						}
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x05))//SHR Frequency -
			{
				if (set_flag == 0)
				{
					if (fre_num > 1)
					{
						fre_num--; qingling = 0;
						light_time_choice(fre_num);
						quang = fre_num - 1;
						fre_quang_display(quang);
						fre_display(fre_num);
						fluence_num_max_choice(fre_num);
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}

			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x08))//SHR Energy +
			{
				if (set_flag)
				{
					if (energy_num < 999999)
					{
						energy_num = energy_num + 10;
						energy_woke = energy_num;
						if (add_reduce_flag)
						{
							if (energy_woke > 0) { bijiao_flag = 0; }
						}
						Energy_display(energy_num); qingling = 0;
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x07))//SHR Energy -
			{
				if (set_flag)
				{
					if (energy_num > 0)
					{
						energy_num = energy_num - 10;
						energy_woke = energy_num; qingling = 0;
						if (add_reduce_flag)
						{
							if (energy_woke == 0) { bijiao_flag = 1; }
						}
						Energy_display(energy_num);
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x06))//SHR Set Key
			{

				set_flag = !set_flag;
				add_reduce_flag = 1; qingling = 0;
				energy_woke = energy_num;
				if (energy_woke == 0) { bijiao_flag = 1; }
				Energy_display(energy_num);
				if (set_flag)
				{
					cut_picture(23, setdraw_xs, setdraw_ys, setdraw_xe, setdraw_ye, setdraw_xs, setdraw_ys);
					cut_picture(23, setadd_xs, setadd_ys, setadd_xe, setadd_ye, setadd_xs, setadd_ys);

				}
				else 
				{
					cut_picture(22, setdraw_xs, setdraw_ys, setdraw_xe, setdraw_ye, setdraw_xs, setdraw_ys);
					cut_picture(22, setadd_xs, setadd_ys, setadd_xe, setadd_ye, setadd_xs, setadd_ys);
				}
				beep_count = 0; BEEP = 0; beep_flag = 1; return;

			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x01))    //PULSE 1
			{
				shots = 0;
				two = 0; qingling = 0;
				shots_flag = 1;
				cut_picture(10, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
				cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
				cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
				if (xuanze > 1)
				{
					xuanze = 1;
					cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
					cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
					cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
					cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
					cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				}
				if (shots > 0)
				{
					light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
				}
				else 
				{ 
					light_width_num = (pulse_width_num[0] + delay_width_num[0]); 
				}
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x02))    //PULSE 2
			{
				shots = 1;
				two = 1; qingling = 0;
				shots_flag = 1;
				cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
				cut_picture(10, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
				cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
				if (xuanze > 1)
				{
					xuanze = 1;
					cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
					cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
					cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
					cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
					cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
					cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				}
				if (shots > 0)
				{
					light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
				}
				else 
				{ 
					light_width_num = (pulse_width_num[0] + delay_width_num[0]); 
				}
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x03))    //PULSE 3
			{
				two = 1;
				if (shots_flag)
				{
					shots_flag = 0;
					shots = 2;
				}
				xuanze = 6; qingling = 0;
				cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
				cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
				cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
				if (shots > 0)
				{
					light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
				}
				else 
				{ 
					light_width_num = (pulse_width_num[0] + delay_width_num[0]); 
				}
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x04))    //高脉宽1
			{
				xuanze = 1; qingling = 0;
				cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
				cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
				cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
				cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
				cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x05))    //高脉宽2
			{
				xuanze = 2; qingling = 0;
				cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
				cut_picture(20, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
				cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
				cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
				cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x06))    //高脉宽3
			{
				xuanze = 3; qingling = 0;
				cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
				cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
				cut_picture(20, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
				cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
				cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x07))    //低脉宽4
			{
				xuanze = 4; qingling = 0;
				cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
				cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
				cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
				cut_picture(20, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
				cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x08))    //低脉宽5
			{
				xuanze = 5; qingling = 0;
				cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
				cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
				cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
				cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
				cut_picture(20, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x09))    //公共加
			{
				if (xuanze == 1)
				{
					if (pulse_width_num[0] < 50)
					{
						pulse_width_num[0]++;
						pulsewidth1_display(pulse_width_num[0]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 2)
				{
					if (pulse_width_num[1] < 50)
					{
						pulse_width_num[1]++;
						pulsewidth2_display(pulse_width_num[1]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 3)
				{
					if (pulse_width_num[2] < 50)
					{
						pulse_width_num[2]++;
						pulsewidth3_display(pulse_width_num[2]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 4)
				{
					if (delay_width_num[0] < 50)
					{
						delay_width_num[0]++;
						delaywidth1_display(delay_width_num[0]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 5)
				{
					if (delay_width_num[1] < 50)
					{
						delay_width_num[1]++;
						delaywidth2_display(delay_width_num[1]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 6)
				{
					if (shots < 8)
					{
						shots++;
						cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (shots > 0)
				{
					light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
				}
				else 
				{ 
					light_width_num = (pulse_width_num[0] + delay_width_num[0]); 
				}
				for (a = 2; a < 9; a++)
				{
					pulse_width_num[a] = pulse_width_num[2];
					delay_width_num[a] = delay_width_num[1];
				}
				qingling = 0; return;
			}
			else if ((rec[1] == 0x01) && (rec[2] == 0x0A))    //公共减
			{
				if (xuanze == 1)
				{
					if (pulse_width_num[0] > 5)
					{
						pulse_width_num[0]--;
						pulsewidth1_display(pulse_width_num[0]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 2)
				{
					if (pulse_width_num[1] > 5)
					{
						pulse_width_num[1]--;
						pulsewidth2_display(pulse_width_num[1]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 3)
				{
					if (pulse_width_num[2] > 5)
					{
						pulse_width_num[2]--;
						pulsewidth3_display(pulse_width_num[2]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 4)
				{
					if (delay_width_num[0] > 5)
					{
						delay_width_num[0]--;
						delaywidth1_display(delay_width_num[0]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 5)
				{
					if (delay_width_num[1] > 5)
					{
						delay_width_num[1]--;
						delaywidth2_display(delay_width_num[1]);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (xuanze == 6)
				{
					if (shots > 2)
					{
						shots--;
						cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
						beep_count = 0; BEEP = 0; beep_flag = 1;
					}
					else 
					{ 
						BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; delayms(50); BEEP = 0; delayms(50); BEEP = 1; return; 
					}
				}
				if (shots > 0)
				{
					light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
				}
				else 
				{ 
					light_width_num = (pulse_width_num[0] + delay_width_num[0]); 
				}
				for (a = 2; a < 9; a++)
				{
					pulse_width_num[a] = pulse_width_num[2];
					delay_width_num[a] = delay_width_num[1];
				}
				qingling = 0; return;
			}
			else if ((rec[1] == 0x03) && (rec[2] == 0x06))//Cooling +
			{
				if (set_flag == 0)
				{
					if (coolrate < 4)
					{
						coolrate++; qingling = 0;
						cooling_quang_display(coolrate);
						cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}

			}
			else if ((rec[1] == 0x03) && (rec[2] == 0x05))//Cooling -
			{
				if (set_flag == 0)
				{
					if (coolrate > 1)
					{
						coolrate--; qingling = 0;
						cooling_quang_display(coolrate);
						cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
						beep_count = 0; BEEP = 0; beep_flag = 1; return;
					}
					else 
					{
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
					}
				}
				else 
				{ BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; }

			}
			else if ((rec[1] == 0x05) && (rec[2] == 0x08))//RFinten +
			{

				if (RFinten < 100)
				{
					RFinten = RFinten + 10;	qingling = 0;
					RFinten_display(RFinten);
					RFinten_choice(RFinten);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}

			}
			else if ((rec[1] == 0x05) && (rec[2] == 0x07))//RFinten -
			{
				if (RFinten > 10)
				{
					RFinten = RFinten - 10; qingling = 0;
					RFinten_display(RFinten);
					RFinten_choice(RFinten);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0x0A) && (rec[2] == 0x01))//BURST Program1
			{
				if (ready_flag == 0)
				{
					progarm = 0; qingling = 0;
					jiemian1_progarm_display_variable(0);
					cut_picture(11, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(11, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(11, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}

			}
			else if ((rec[1] == 0x0A) && (rec[2] == 0x02))//BURST Program2
			{
				if (ready_flag == 0)
				{
					progarm = 1; qingling = 0;
					jiemian1_progarm_display_variable(1);
					cut_picture(12, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(12, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(12, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0x0A) && (rec[2] == 0x03))//BURST Program3
			{
				if (ready_flag == 0)
				{
					progarm = 2; qingling = 0;
					jiemian1_progarm_display_variable(2);
					cut_picture(13, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(13, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(13, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x01))//SHR Program1
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					progarm = 0; qingling = 0;
					jiemian2_progarm_display_variable(3);
					cut_picture(11, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(11, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(11, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}

			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x02))//SHR Program2
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					progarm = 1; qingling = 0;
					jiemian2_progarm_display_variable(4);
					cut_picture(12, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(12, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(12, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0x0C) && (rec[2] == 0x03))//SHR Program3
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					progarm = 2; qingling = 0;
					jiemian2_progarm_display_variable(5);
					cut_picture(13, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
					cut_picture(13, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
					cut_picture(13, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return;
				}
			}
			else if ((rec[1] == 0xA1) && (rec[2] == 0xA2)) //SAVE
			{
				if (interface_flag == 2)
				{
					if (progarm == 0) { write_jiemian2_program_data(0); }
					else if (progarm == 1) { write_jiemian2_program_data(1); }
					else if (progarm == 2) { write_jiemian2_program_data(2); }
				}
				else if (interface_flag == 1)
				{
					if (set_flag == 0)
					{
						if (progarm == 0) { write_jiemian1_program_data(3); }
						else if (progarm == 1) { write_jiemian1_program_data(4); }
						else if (progarm == 2) { write_jiemian1_program_data(5); }
					}
					else 
					{ 
						BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
					}
				}
				qingling = 0; beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0xA2) && (rec[2] == 0xA1)) //Enter Background
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					houtai_flag = 1; qingling = 0;
					picture_display(34);
					flow_rate_display(flowrate);
					tempdata_deal_display(water_temp);
					houtai_total_display(zongji);
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			}
			else if ((rec[1] == 0xA2) && (rec[2] == 0xA3)) //Return To SHR Or BURST				 A2 A3
			{
				xianshi_flag = 0;
				houtai_flag = 0;
				qingling = 0;
				if (interface_flag == 2)
				{
					picture_display(9);
					fanhui_IPL_display();
				}
				else if (interface_flag == 1)
				{
					picture_display(22);
					fanhui_OPT_display();
				}
				beep_count = 0; BEEP = 0; beep_flag = 1; return;
			}
			else if ((rec[1] == 0x0B) && (rec[2] == 0x0C)) //once_num清零
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					if (interface_flag == 2)
					{
						once_num1 = 0;
						shotonce_display(once_num1);
					}
					else if (interface_flag == 1)
					{
						once_num2 = 0;
						shotonce_display(once_num2);
					}
					beep_count = 0; BEEP = 0; beep_flag = 1; return;
				}
				else 
				{ 
					BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; 
				}
			}
			else if ((rec[1] == 0xA2) && (rec[2] == 0xA4)) //后台清零
			{
				qingling++;
				if (qingling >= 50)
				{
					qingling = 0;
					zongji = 0;
					houtai_total_display(zongji);
					write_zongji_data(6);
				}
			}
			else if ((rec[1] == 0x0A) && (rec[2] == 0x0D)) //手柄清零
			{
				qingling++;
				if (qingling >= 80)
				{
					qingling = 0;
					if (interface_flag == 2)
					{
						total1_count = 0;
						total_display(total1_count);
						write_total1();
					}
					else if (interface_flag == 1)
					{
						total2_count = 0;
						total_display(total2_count);
						write_total2();
					}
				}
			}
		}
	}
}
void baojing_keyport()
{
	if (RXFRMOK)
	{
		RXFRMOK = 0;
		if (rec[0] == 0x79)
		{

			if ((rec[1] == 0xA2) && (rec[2] == 0xA1)) //后台
			{
				if (ready_flag == 0 && set_flag == 0)
				{
					houtai_flag = 1; qingling = 0;
					picture_display(34);
					flow_rate_display(flowrate);
					tempdata_deal_display(water_temp);
					houtai_total_display(zongji);
					return;
				}
				else { BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; delayms(80); BEEP = 0; delayms(80); BEEP = 1; return; }
			}
			else if ((rec[1] == 0xA2) && (rec[2] == 0xA3)) //后台返回
			{
				xianshi_flag = 0;
				houtai_flag = 0; yitou_flag = 0; ertou_flag = 0;
				qingling = 0; jiance1_flag = 0; jiance2_flag = 0;
				flowrate_flag = 0; warm_count = 0; ready_enter_flag = 0; water_temp_flag = 0; temp_sensor_flag = 0;
				if (interface_flag == 2)
				{
					picture_display(9);
					fanhui_IPL_display();
				}
				else if (interface_flag == 1)
				{
					picture_display(22);
					fanhui_OPT_display();
				}
				return;
			}
			else if ((rec[1] == 0x0A) && (rec[2] == 0x0D)) //手柄清零
			{
				qingling++;
				if (qingling >= 80)
				{
					qingling = 0;
					if (interface_flag == 2)
					{
						total1_count = 0; guoqi1_flag = 0;
						total_display(total1_count);
						write_total1();
					}
					else if (interface_flag == 1)
					{
						total2_count = 0; guoqi2_flag = 0;
						total_display(total2_count);
						write_total2();
					}
				}
			}
		}
	}
}

//**************************TIME INTERRUPT************************************************************************
// time0 interrupt 1ms

void time0(void) interrupt 1
{
	TH0 = 0xF8;//1ms中断
	TL0 = 0xCC;
	TF0 = 0;
	//Cooling Part
	if (coolrate < 9)
	{
		cool_count++;
		if (cool_count == coolrate)
		{
			COOL_PWM = 1;
		}
		else if (cool_count >= 9)
		{
			COOL_PWM = 0;
			cool_count = 0;
		}
	}
	else { COOL_PWM = 0; cool_count = 0; }
	//////////////////////////////
	if (dong_flag)
	{
		sudu_count++;
		if (sudu_count >= 400)		  //图片切换的速度
		{
			jishu++;
			xianshi_flag = 1;
			sudu_count = 0;
			if (jishu >= 8)			 //图片
			{
				dong_flag = 0;
			}
		}
	}
    if(onlight_flag)
	{
	 if (interface_flag == 2)//BURST 主界面
		{
			ppcount++;
			if (ppcount <= light_width_num)
			{
				PULSEcount++; 
				if (PULSEcount <= pulse_width_num[b])
				{
					PULSE_PWM = 0;//出光
					ff_flag = 1;
				}
				else if ((PULSEcount > pulse_width_num[b]) && (PULSEcount <= (pulse_width_num[b] + delay_width_num[b])))
				{
					PULSE_PWM = 1;//不出光
					if (PULSEcount == (pulse_width_num[b] + delay_width_num[b]))
					{
						PULSEcount = 0; b++;
					}
					if (ff_flag)
					{
						once_num1++;
						total1_count++;
						zongji++;
						if (zongji >= 99999999) { zongji = 99999999; }
						if (total1_count >= 60000000)
						{
							guoqi1_flag = 1; open_flag = 0; riqi1_flag = 1;
						}
						ff_flag = 0;
						xx_flag = 1;
					}
				}
			}
			else if ((ppcount > light_width_num) && (ppcount < light_width_num + 1000))
			{
				PULSE_PWM = 1;//不出光				 

			}
			else if (ppcount >= light_width_num + 1000)
			{
				pulseout_count = 0; BEEP = 0; pulseout_beep_flag = 1;
				PULSEcount = 0; ppcount = 0; b = 0;onlight_flag=0;baocun_flag=1;
			}
		}
	}
	if (open_flag)
	{
		
		if (interface_flag == 1)//SHR 主界面
		{
			PULSEcount++; sss_flag = 1; ff_flag = 1;
			if (PULSEcount == light_time)
			{
				PULSE_PWM = 0;//出光
			}
			else if (PULSEcount == light_time + 20)	         //改出光的脉宽  20对应出光20ms
			{
				PULSE_PWM = 1;//不出光
				PULSEcount = 0;
				if (ff_flag)
				{
					once_num2++;
					total2_count++;
					if (total2_count >= 60000000)
					{
						guoqi2_flag = 1; open_flag = 0; riqi2_flag = 1;
					}
					zongji++;
					if (zongji >= 99999999) { zongji = 99999999; }
					if (add_reduce_flag)
					{
						energy_woke = energy_woke - energy_fa;
						energy_num = energy_woke;
						if (energy_woke <= energy_fa)
						{
							energy_woke = 0; open_flag = 0; bijiao_flag = 1;
							wang_flag = 1; energy_num = energy_woke;
						}
					}
					else 
					{
						energy_woke = energy_woke + energy_fa;
						if (energy_woke >= 999999)
						{
							energy_woke = 999999;
						}
					}
					ff_flag = 0;
					sss_flag = 0;
				}
			}
		}
	}

	//pulse out beep 
	if (pulseout_beep_flag)
	{
		pulseout_count++;
		if (pulseout_count >= 40)
		{
			pulseout_count = 0; BEEP = 1; pulseout_beep_flag = 0;
		}
	}

	//蜂鸣器
	if (beep_flag)
	{
		beep_count++;
		if (beep_count >= 80)
		{
			beep_count = 0; BEEP = 1; beep_flag = 0;
		}
	}

	if (ttbeep_flag)
	{
		ttbeep_count++;
		if (ttbeep_count >= 600)
		{
			ttbeep_count = 0; BEEP = 1; ttbeep_flag = 0;
		}
	}
	//报警
	if (shut_key_flag == 0)
	{
		warm_count++;
		if (warm_count == 100) { BEEP = 0; }
		else if (warm_count >= 300) { BEEP = 1; warm_count = 0; }
	}

	//脉冲信号水流传感器频率检测	 
	temp_count++;
	if (temp_count >= 1000)
	{
		TR1 = 0;
		miao_flag = 1;
		temp_count = 0;
		flowrate_frenum = TL1;
		TH1 = 0X00;//计数器
		TL1 = 0X00;
		flowrate = (flowrate_frenum * 10) / 8;//waterlevel_frenum/8 实际是水流量，10为小数点显示位置
		yy_flag = 0;
		//AD启动转换	   	
		ADC_CONTR = ADC_CONTR | 0x08;
		_nop_(); _nop_(); _nop_(); _nop_();
		TR1 = 1;
	}
}

/***********************************************************************************************************************************************

											  key check end
**************************************************************************************************************************************************/

void running_funtion()
{
	//     
	if (open_flag)
	{
		if (sss_flag == 0)
		{
			sss_flag = 1;
		    if (interface_flag == 1)
			{
				total_display(total2_count);
				shotonce_display(once_num2);
				Energy_display(energy_woke);
			}

		}
	}
	if(xx_flag)
	{
	  xx_flag=0;
	  if (interface_flag == 2)
	  {
		 total_display(total1_count);
		 shotonce_display(once_num1);
	  }
	}
	if(baocun_flag)
	{
		  baocun_flag=0;
	      sss_flag = 0;
		  PULSE_PWM = 1;
		  PULSEcount = 0;
		  ppcount = 0;
		  yy_flag = 0;
		  write_zongji_data(6);
		  delayms(100);				
		  write_total1();
		  temp_count = 0;
		  TH1 = 0X00;//计数器
		  TL1 = 0X00;
		  flowrate = 15;
		  shaomiao_flag = 0;
		  miao_flag = 0;
		  hand1_flag = 1;
		  hand2_flag = 1;
	}
	if (dongtai_flag)
	{
		dongtai_flag = 0;
		if(shaomiao_flag==0){flow_rate_display(flowrate);}//显示水流量		      			
		tempdata_deal_display(water_temp);//显示温度数据
	}
	if (wang_flag) { wang_flag = 0; Energy_display(energy_woke); }
	if (riqi2_flag) { riqi2_flag = 0; total_display(total2_count); write_total2(); }
	if (riqi1_flag) { riqi1_flag = 0; total_display(total1_count); write_total1(); }
	if (miao_flag == 1 && shaomiao_flag == 0) { miao_flag = 0; jiance_hand1(); jiance_hand2(); }
	//报警检测
	if (shaomiao_flag == 0 || water_temp > 65)
	{
		if (interface_flag == 2)
		{
			if (hand1_flag)
			{
				if (jiance1_flag == 1)
				{
					if (ready_flag == 0)
					{
						woking_flag = 1;
						RXFRMOK = 0; flowrate_flag = 0; warm_count = 0; water_temp_flag = 0; temp_sensor_flag = 0;
						shut_key_flag = 1;	BEEP = 1;
						jiance1_flag = 0; cut_picture(9, 310, 380, 800, 440, 310, 380);
					}

				}
			}
			else 
			{
				if (jiance1_flag == 0)
				{
					key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
					shut_key_flag = 0; ready_flag = 0;
					YRXHCTRL = 1;//待机
					PULSE_PWM = 1;
					CCAP1H = CCAP1L = 0X00;//停止充电					
					cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
					cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
					cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
					jiance1_flag = 1; cut_picture(45, 165, 70, 650, 115, 310, 380);
				}
			}
			if (guoqi1_flag)
			{
				if (yitou_flag == 0)
				{
					yitou_flag = 1;
					cut_picture(45, 165, 395, 650, 445, 310, 380);
					key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
					shut_key_flag = 0;
					ready_flag = 0;
					YRXHCTRL = 1;//待机
					PULSE_PWM = 1;
					CCAP1H = CCAP1L = 0X00;//停止充电					
					cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
					cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
					cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
				}
			}
			else
			{
				if (yitou_flag)
				{
					woking_flag = 1; jiance1_flag = 0; ertou_flag = 0;
					RXFRMOK = 0; flowrate_flag = 0; warm_count = 0; water_temp_flag = 0; temp_sensor_flag = 0;
					shut_key_flag = 1;	BEEP = 1;
					jiance1_flag = 0; cut_picture(9, 310, 380, 800, 440, 310, 380);
				}
			}
		}
		if (interface_flag == 1)
		{
			if (hand2_flag)
			{
				if (jiance1_flag == 1)
				{
					if (ready_flag == 0)
					{
						woking_flag = 1; flowrate_flag = 0; warm_count = 0; water_temp_flag = 0; temp_sensor_flag = 0;
						RXFRMOK = 0; shut_key_flag = 1;	BEEP = 1;
						jiance1_flag = 0; cut_picture(9, 310, 380, 800, 440, 310, 380);
					}
				}
			}
			else 
			{
				if (jiance1_flag == 0)
				{
					key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
					shut_key_flag = 0; ready_flag = 0;
					YRXHCTRL = 1;//待机
					PULSE_PWM = 1;
					CCAP1H = CCAP1L = 0X00;//停止充电					
					cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
					cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
					cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
					jiance1_flag = 1; cut_picture(45, 165, 70, 650, 115, 310, 380);
				}
			}
			if (guoqi2_flag)
			{
				if (ertou_flag == 0)
				{
					key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
					shut_key_flag = 0; ready_flag = 0;
					YRXHCTRL = 1;//待机
					PULSE_PWM = 1;
					CCAP1H = CCAP1L = 0X00;//停止充电					
					cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
					cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
					cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
					ertou_flag = 1; cut_picture(45, 165, 395, 650, 445, 310, 380);
				}
			}
			else
			{
				if (ertou_flag)
				{
					woking_flag = 1; jiance1_flag = 0; yitou_flag = 0;
					RXFRMOK = 0; flowrate_flag = 0; warm_count = 0; water_temp_flag = 0; temp_sensor_flag = 0;
					shut_key_flag = 1;	BEEP = 1;
					jiance1_flag = 0; cut_picture(9, 310, 380, 800, 440, 310, 380);
				}
			}
		}
		if (flowrate >= 10)
		{
			flowrate_flag = 0;
			if (ADC_RES > 50)	//检测出此时温度传感器连接良好
			{
				temp_sensor_flag = 0;
				if (water_temp <= 65)
				{
					water_temp_flag = 0;
					if (ready_flag == 0 && ready_enter_flag == 0)
					{
						woking_flag = 1;
						RXFRMOK = 0; jiance1_flag = 0; jiance2_flag = 0; yitou_flag = 0; ertou_flag = 0;
						ready_enter_flag = 1; shut_key_flag = 1;	BEEP = 1;
						cut_picture(9, 240, 380, 750, 440, 240, 380);
					}
				}
				else
				{
					if (water_temp_flag == 0)
					{
						key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
						shut_key_flag = 0; ready_flag = 0; water_temp_flag = 1; warm_count = 0; ready_enter_flag = 0;
						YRXHCTRL = 1;//待机
						PULSE_PWM = 1;
						CCAP1H = CCAP1L = 0X00;//停止充电
						if (houtai_flag == 0)
						{
							cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
							cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
							cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
						}
						cut_picture(45, 165, 130, 650, 188, 270, 380);
					}
				}
			}
			else//检测出此时断开了温度传感器
			{
				if (temp_sensor_flag == 0)
				{
					key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
					shut_key_flag = 0; ready_flag = 0; warm_count = 0; ready_enter_flag = 0; water_temp_flag = 0; temp_sensor_flag = 1;
					YRXHCTRL = 1;//待机
					PULSE_PWM = 1;
					CCAP1H = CCAP1L = 0X00;//停止充电					
					if (houtai_flag == 0)
					{
						cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
						cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
						cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
					}
					cut_picture(45, 165, 335, 650, 388, 270, 380);
				}
			}
		}
		else
		{
			if (flowrate_flag == 0)
			{
				key_flag = 0; open_flag = 0; PULSEcount = 0; sss_flag = 0; woking_flag = 0;
				shut_key_flag = 0; ready_flag = 0; flowrate_flag = 1; warm_count = 0; ready_enter_flag = 0; water_temp_flag = 0; temp_sensor_flag = 0;
				YRXHCTRL = 1;//待机
				PULSE_PWM = 1;
				CCAP1H = CCAP1L = 0X00;//停止充电					
				if (houtai_flag == 0)
				{
					cut_picture(9, readydraw_xs, readydraw_ys, readydraw_xe, readydraw_ye, readydraw_xs, readydraw_ys);
					cut_picture(9, standbydraw_xs, standbydraw_ys, standbydraw_xe, standbydraw_ye, standbydraw_xs, standbydraw_ys);
					cut_picture(9, statusdraw_light_xs, statusdraw_light_ys, statusdraw_light_xe, statusdraw_light_ye, statusdraw_light_xs, statusdraw_light_ys);
				}
				cut_picture(45, 165, 195, 650, 245, 270, 380);
			}
		}
	}
}

/***********************************************************************************************************************************************
											 初始化开始
**************************************************************************************************************************************************/
void PCA_initial()
{
	CCON = 0;								   //PCA控制寄存器 清零	  可位寻址
	CL = CH = 0;
	CMOD = 0; //选择SOC/12
	CCAP1H = CCAP1L = 0X00;//
	PCA_PWM1 = 0X00;
	CCAPM1 = 0X42;//8BIT 无中断PWM	 	 
	CR = 1;									   //PCA控制寄存器里的启动位 1为启动
}

/**************************************************************************
 AD转换初始化
***************************************************************************/

void AD_initial()
{
	EADC = 1;
	P1ASF = P1ASF | 0X01;//确定模拟端口,P1_0做AD转换接口
	ADC_CONTR = 0XE0;	//启动电源//
	delayms(5);
}

/*****************************串口初始化***************************************/

void serial_port_initial()
{
	SCON = 0X50;//串口方式1
	PCON = (PCON | 0X80);//SOMD=1
	BRT = 0XF4;//重载数据
	AUXR = 0X15;//独立波特率发生器设置
	ES = 1;
	EA = 1;
}
/***************************定时器初始化*******************************************/

void timeq_initial()
{
	TMOD = 0X51;
	TH0 = 0XF8;//1ms中断
	TL0 = 0XCC;
	ET0 = 1;
	TR0 = 1;

	TH1 = 0X00;//计数器
	TL1 = 0X00;
	TR1 = 1;
}

/*****************************初始化变量*******************************************/
void fanhui_IPL_display()
{
	if (progarm == 0)
	{
		cut_picture(11, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(11, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(11, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	if (progarm == 1)
	{
		cut_picture(12, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(12, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(12, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	if (progarm == 2)
	{
		cut_picture(13, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(13, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(13, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	quang = (fluence - 1) / 6;
	fluence1_quang_display(quang);
	fluence1_display(fluence);
	pulsewidth1_display(pulse_width_num[0]);
	pulsewidth2_display(pulse_width_num[1]);
	pulsewidth3_display(pulse_width_num[2]);
	delaywidth1_display(delay_width_num[0]);
	delaywidth2_display(delay_width_num[1]);
	cooling_quang_display(coolrate);
	cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
	//	RFinten_display(RFinten);
	if (shots == 0)
	{
		two = 0;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(10, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else if (shots == 1)
	{
		two = 1;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(10, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else
	{
		two = 1;
		xuanze = 6;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	total_display(total1_count);
	shotonce_display(once_num1);
	delayms(100);
	total_display(total1_count);
	shotonce_display(once_num1);
	if (shots == 0)
	{
		two = 0;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(10, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else if (shots == 1)
	{
		two = 1;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(10, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else
	{
		two = 1;
		xuanze = 6;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
}
void fanhui_OPT_display()
{
	if (progarm == 0)
	{
		cut_picture(11, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(11, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(11, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	if (progarm == 1)
	{
		cut_picture(12, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(12, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(12, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	if (progarm == 2)
	{
		cut_picture(13, progarm1_xs, progarm1_ys, progarm1_xe, progarm1_ye, progarm1_xs, progarm1_ys);
		cut_picture(13, progarm2_xs, progarm2_ys, progarm2_xe, progarm2_ye, progarm2_xs, progarm2_ys);
		cut_picture(13, progarm3_xs, progarm3_ys, progarm3_xe, progarm3_ye, progarm3_xs, progarm3_ys);
	}
	quang = (fluence - 1) / 6;
	fluence2_quang_display(quang);
	fluence2_display(fluence);
	cooling_quang_display(coolrate);
	cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
	Energy_display(energy_woke);
	quang = fre_num - 1;
	fre_quang_display(quang);
	fre_display(fre_num);
	total_display(total2_count);
	shotonce_display(once_num2);
	delayms(100); delayms(100); delayms(100);
	total_display(total2_count);
	shotonce_display(once_num2);
}
void jiemian2_progarm_display_variable(uchar a)
{
	read_jiemian1_program_data(a);
	if (fre_num > 10)
	{
		fluence = 30;
		fre_num = 1;
		coolrate = 2;
		energy_num = 0;

	}
	delayms(100); delayms(100); delayms(100); delayms(100); delayms(100);
	fluence_num_max_choice(fre_num);
	energy_fa = fluence;
	cap_num = 44 + 2 * fluence;
	quang = (fluence - 1) / 6;
	fluence2_quang_display(quang);
	fluence2_display(fluence);
	cooling_quang_display(coolrate);
	cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
	Energy_display(energy_woke);
	light_time_choice(fre_num);
	quang = fre_num - 1;
	fre_quang_display(quang);
	fre_display(fre_num);
	total_display(total2_count);
	shotonce_display(once_num2);
}

void jiemian1_progarm_display_variable(uchar a)
{
	read_jiemian2_program_data(a);
	if (shots >= 0xff)
	{
		shots = 0;
		fluence = 30;
		pulse_width_num[0] = 20;
		pulse_width_num[1] = 20;
		pulse_width_num[2] = 20;
		delay_width_num[0] = 20;
		delay_width_num[1] = 20;
		coolrate = 2;
		RFinten = 30;
	}
	delayms(100); delayms(100); delayms(100); delayms(100); delayms(100);
	cap_num = 44 + 2 * fluence;
	quang = (fluence - 1) / 6;
	fluence1_quang_display(quang);
	fluence1_display(fluence);
	pulsewidth1_display(pulse_width_num[0]);
	pulsewidth2_display(pulse_width_num[1]);
	pulsewidth3_display(pulse_width_num[2]);
	delaywidth1_display(delay_width_num[0]);
	delaywidth2_display(delay_width_num[1]);
	cooling_quang_display(coolrate);
	cut_picture(coolrate + 11, cooling_rate_xs, cooling_rate_ys, cooling_rate_xe, cooling_rate_ye, cooling_rate_xs, cooling_rate_ys);
	total_display(total1_count);
	shotonce_display(once_num1);
	//	RFinten_display(RFinten);     用射频时要编译
	if (shots == 0)
	{
		two = 0;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(10, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else if (shots == 1)
	{
		two = 1;
		shots_flag = 1;
		xuanze = 1;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(10, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(11, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(20, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	else {
		two = 1;
		xuanze = 6;
		cut_picture(11, maichong1_xs, maichong1_ys, maichong1_xe, maichong1_ye, maichong1_xs, maichong1_ys);
		cut_picture(11, maichong2_xs, maichong2_ys, maichong2_xe, maichong2_ye, maichong2_xs, maichong2_ys);
		cut_picture(12 + shots, maichong3_xs, maichong3_ys, maichong3_xe, maichong3_ye, maichong3_xs, maichong3_ys);
		cut_picture(21, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys, pulsewidth1_trigon_xe, pulsewidth1_trigon_ye, pulsewidth1_trigon_xs, pulsewidth1_trigon_ys);
		cut_picture(21, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys, pulsewidth2_trigon_xe, pulsewidth2_trigon_ye, pulsewidth2_trigon_xs, pulsewidth2_trigon_ys);
		cut_picture(21, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys, pulsewidth3_trigon_xe, pulsewidth3_trigon_ye, pulsewidth3_trigon_xs, pulsewidth3_trigon_ys);
		cut_picture(21, delaywidth1_trigon_xs, delaywidth1_trigon_ys, delaywidth1_trigon_xe, delaywidth1_trigon_ye, delaywidth1_trigon_xs, delaywidth1_trigon_ys);
		cut_picture(21, delaywidth2_trigon_xs, delaywidth2_trigon_ys, delaywidth2_trigon_xe, delaywidth2_trigon_ye, delaywidth2_trigon_xs, delaywidth2_trigon_ys);
	}
	if (shots > 0)
	{
		light_width_num = (pulse_width_num[0] + delay_width_num[0]) + two*(pulse_width_num[1] + delay_width_num[1]) + (shots - 1)*(pulse_width_num[2] + delay_width_num[1]);
	}
	else
	{
		light_width_num = (pulse_width_num[0] + delay_width_num[0]);
	}
	for (a = 2; a < 9; a++)
	{
		pulse_width_num[a] = pulse_width_num[2];
		delay_width_num[a] = delay_width_num[1];
	}
}

void variable_initial()
{
	once_num1 = 0;
	once_num2 = 0;
	shut_key_flag = 1;
	flowrate = 35;
	water_temp = 25;
	COOL_PWM = 1;
	interface_flag = 0;

	CCAP1H = CCAP1L = 0X00;
	beep_flag = 0;
	woking_flag = 1;
}
/**************************系统初始化******************************************/

void system_initial()
{
	AD_initial();
	timeq_initial();
	serial_port_initial();
	PCA_initial();
	variable_initial();

	RXAAOK = 0;
	RXFRMOK = 0;
}

void main()
{
	system_initial();
	delayms(100); delayms(100); delayms(100); delayms(100);
	jiance_hand1(); jiance_hand2();
	if (hand1_flag == 1 || hand2_flag == 1) { PUMP = 0; }
	read_zongji_data(6);
	if (zongji >= 0x5f5e0ff)
	{
		zongji = 0;
	}
	read_total1();
	if (total1_count >= 0x5f5e0ff)
	{
		total1_count = 0;
	}
	if (total1_count >= 60000000)
	{
		guoqi1_flag = 1;
	}
	read_total2();
	if (total2_count >= 0x5f5e0ff)
	{
		total2_count = 0;
	}
	if (total2_count >= 60000000)
	{
		guoqi2_flag = 1;
	}
	send_ack();//握手 
	BEEP = 0;
	delayms(100); delayms(100); delayms(100); delayms(100);
	BEEP = 1;
	picture_display(0);//display first interface
	dong_flag = 1;
	while (1)
	{
		if (xianshi_flag)
		{
			picture_display(jishu);
			xianshi_flag = 0;
			if (jishu == 8)
			{
				beep_count = 0; BEEP = 0; beep_flag = 1;
			}
		}
		if (woking_flag) { keyport(); }
		else
		{
			baojing_keyport();
		}
		if (interface_flag > 0)
		{
			running_funtion();
		}
	}
}