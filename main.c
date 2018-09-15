#include<reg52.h>
#include<intrins.h>
#include<stdlib.h>

#define uchar unsigned char
#define uint unsigned int

sbit RS=P2^5;		//H表示数据，L表示指令
sbit RW=P2^6;		//H表示读，L表示写
sbit EN=P2^7;		//使能端
sbit PSB=P2^4;		//H并口方式，L串口方式。采用并口

sbit key_up=P2^3;
sbit key_left=P2^2;
sbit key_down=P2^1;
sbit key_right=P2^0;
sbit key_sp=P3^2;		//开始，暂停
sbit key_speed=P1^1;		//加速键

uchar win_score=0;		//打印分数做下标用
uchar num[]={48,49,50,51,52,53,54,55,56,57,48};		//0~9

/******************************************
LCD12864部分
******************************************/
/********************************
延时函数，延时x (ms)
********************************/
void delay(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)		//i=x即延时x毫秒
		for(j=110;j>0;j--);
}

void delay_nop()
{
	_nop_();
	_nop_();
}

/********************************
读忙函数
********************************/
uchar R_busy()
{
	uchar result;
	RS=0;
	RW=1;
	EN=1;
	delay_nop();
	result=(P0&0x80);
	EN=0;
	return result;
}

/********************************
写指令
********************************/
void W_com(uchar com)
{
	while(R_busy());		//每次操作前都应该先读忙
	RS=0;
	RW=0;
	EN=0;

	P0=com;
	delay_nop();

	EN=1;
	delay_nop();		//延迟一下给个高脉冲
	EN=0;

}

/********************************
写数据
********************************/
void W_data(uchar dat)
{
	while(R_busy());
	delay_nop();
	RS=1;
	RW=0;
	EN=0;

	P0=dat;
	delay_nop();

	EN=1;
	delay_nop();
	EN=0;
}

/********************************
读数据	   
********************************/
uchar R_data()
{
	uchar value;
//	delay20us();
	while(R_busy());
/*	P0=0xFF;		//先进行一次空读操作
	RS=1;
	RW=1;
	EN=1;
	_nop_();
	_nop_();
	EN=0;			*/

/*	P0=0xFF;		//读取真正的数据
	RS=1;
	RW=1;
	EN=1;
	value=P0;
	_nop_();
	_nop_();
	EN=0;	   */

	EN=0;
	P0=0xff;
	RS=1;
	RW=1;
	EN=1;
	delay_nop();

	value=P0;

	EN=0;


	return value;      
}

/********************************
设定汉字显示位置
********************************/
void LCD_pos(uchar x, uchar y)
{
	uchar pos;
	switch(x)
	{
		case 0:x=0x80;break;
		case 1:x=0x90;break;
		case 2:x=0x88;break;
		case 3:x=0x98;break;
	}
	pos=x+y;
	W_com(pos);
}

/********************************
屏幕闪烁函数
********************************/
void LCD_bling()
{
	uchar j;
	for(j=0;j<3;j++)
	{
		W_com(0x08);		//关显示
		delay(900);
		W_com(0x0c);		//开显示
		delay(900);
	}
//	W_com(0x01);		//清屏
	delay(5);
}

/********************************
对GDRAM进行清屏，防止花屏
********************************/
void Clear_GD()
{
	uchar i,j;
	W_com(0x34);		//开启扩展指令集，并关闭绘图显示
	for(i=0;i<32;i++)
	{
		W_com(0x80+i);		//先写入纵坐标
		W_com(0x80);		//再写入横坐标
		for(j=0;j<32;j++)
		{
			W_data(0x00);
		}
	}
	W_com(0x36);		//开启扩展指令集，开启绘图显示
	W_com(0x30);		//开启基本指令集
}

/********************************
向LCD指定坐标写入一个像素点，
x代表横坐标，为0~127，														   
y代表纵坐标，为0~63，
color表示颜色，0代表白(无显示)，1代表黑(有显示)
********************************/
void W_pixel(uchar x, uchar y, uchar color)
{
	uchar x_byte;		//用于确定横坐标在哪个字节
	uchar x_bit;		//用于确定横坐标在一个字节的哪位
	uchar y_bit;		//用于确定纵坐标在哪位
	uchar start_column;		//用于确定像素点是上半屏或下半屏
	uchar temp_H;		//用于暂存从LCD读出来的数据，高八位
	uchar temp_L;		//用于暂存从LCD读出来的数据，低八位

	x_byte=x/16;		//确定为一行上的第几个16位(地址)
	x_bit=15-x%16;		//确定对该字节的第几位进行操作

	y_bit=y%32;

	if(y<32)		//为上页
		start_column=0x80;
	else		//否则为下页
		start_column=0x88;
	
	W_com(0x34);	//开启扩展指令集，并关闭绘图显示
	
	W_com(0x80+y_bit);		//写入纵坐标
	W_com(start_column+x_byte);		//写入横坐标
	temp_H=R_data();		//假读
	temp_H=R_data();		//读高位
	temp_L=R_data();		//读低位

//由于读数据后AC改变，故重新设置地址
	W_com(0x80+y_bit);		//写入纵坐标
	W_com(start_column+x_byte);		//写入横坐标
		 
	if(color==1)
	{
		if(x_bit>7)
		{
			x_bit=x_bit%8;
			W_data(temp_H|(0x01<<x_bit));
			W_data(temp_L);
		}              
		else
		{
			W_data(temp_H);
			W_data(temp_L|(0x01<<x_bit));
		}
	} 
	else
	{
		 if(x_bit>7)
		{
			x_bit=x_bit%8;
			W_data(temp_H^(0x01<<x_bit));
			W_data(temp_L);
		}              
		else
		{
			W_data(temp_H);
			W_data(temp_L^(0x01<<x_bit));
		}
	} 
//	W_com(0x36);		//开启扩展指令集，开启绘图显示
//	W_com(0x30);		//开启基本指令集       
}