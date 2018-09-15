#include<reg52.h>
#include<intrins.h>
#include<stdlib.h>

#define uchar unsigned char
#define uint unsigned int

sbit RS=P2^5;		//H��ʾ���ݣ�L��ʾָ��
sbit RW=P2^6;		//H��ʾ����L��ʾд
sbit EN=P2^7;		//ʹ�ܶ�
sbit PSB=P2^4;		//H���ڷ�ʽ��L���ڷ�ʽ�����ò���

sbit key_up=P2^3;
sbit key_left=P2^2;
sbit key_down=P2^1;
sbit key_right=P2^0;
sbit key_sp=P3^2;		//��ʼ����ͣ
sbit key_speed=P1^1;		//���ټ�

uchar win_score=0;		//��ӡ�������±���
uchar num[]={48,49,50,51,52,53,54,55,56,57,48};		//0~9

/******************************************
LCD12864����
******************************************/
/********************************
��ʱ��������ʱx (ms)
********************************/
void delay(uint x)
{
	uint i,j;
	for(i=x;i>0;i--)		//i=x����ʱx����
		for(j=110;j>0;j--);
}

void delay_nop()
{
	_nop_();
	_nop_();
}

/********************************
��æ����
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
дָ��
********************************/
void W_com(uchar com)
{
	while(R_busy());		//ÿ�β���ǰ��Ӧ���ȶ�æ
	RS=0;
	RW=0;
	EN=0;

	P0=com;
	delay_nop();

	EN=1;
	delay_nop();		//�ӳ�һ�¸���������
	EN=0;

}

/********************************
д����
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
������	   
********************************/
uchar R_data()
{
	uchar value;
//	delay20us();
	while(R_busy());
/*	P0=0xFF;		//�Ƚ���һ�οն�����
	RS=1;
	RW=1;
	EN=1;
	_nop_();
	_nop_();
	EN=0;			*/

/*	P0=0xFF;		//��ȡ����������
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
�趨������ʾλ��
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
��Ļ��˸����
********************************/
void LCD_bling()
{
	uchar j;
	for(j=0;j<3;j++)
	{
		W_com(0x08);		//����ʾ
		delay(900);
		W_com(0x0c);		//����ʾ
		delay(900);
	}
//	W_com(0x01);		//����
	delay(5);
}

/********************************
��GDRAM������������ֹ����
********************************/
void Clear_GD()
{
	uchar i,j;
	W_com(0x34);		//������չָ������رջ�ͼ��ʾ
	for(i=0;i<32;i++)
	{
		W_com(0x80+i);		//��д��������
		W_com(0x80);		//��д�������
		for(j=0;j<32;j++)
		{
			W_data(0x00);
		}
	}
	W_com(0x36);		//������չָ���������ͼ��ʾ
	W_com(0x30);		//��������ָ�
}

/********************************
��LCDָ������д��һ�����ص㣬
x��������꣬Ϊ0~127��														   
y���������꣬Ϊ0~63��
color��ʾ��ɫ��0�����(����ʾ)��1�����(����ʾ)
********************************/
void W_pixel(uchar x, uchar y, uchar color)
{
	uchar x_byte;		//����ȷ�����������ĸ��ֽ�
	uchar x_bit;		//����ȷ����������һ���ֽڵ���λ
	uchar y_bit;		//����ȷ������������λ
	uchar start_column;		//����ȷ�����ص����ϰ������°���
	uchar temp_H;		//�����ݴ��LCD�����������ݣ��߰�λ
	uchar temp_L;		//�����ݴ��LCD�����������ݣ��Ͱ�λ

	x_byte=x/16;		//ȷ��Ϊһ���ϵĵڼ���16λ(��ַ)
	x_bit=15-x%16;		//ȷ���Ը��ֽڵĵڼ�λ���в���

	y_bit=y%32;

	if(y<32)		//Ϊ��ҳ
		start_column=0x80;
	else		//����Ϊ��ҳ
		start_column=0x88;
	
	W_com(0x34);	//������չָ������رջ�ͼ��ʾ
	
	W_com(0x80+y_bit);		//д��������
	W_com(start_column+x_byte);		//д�������
	temp_H=R_data();		//�ٶ�
	temp_H=R_data();		//����λ
	temp_L=R_data();		//����λ

//���ڶ����ݺ�AC�ı䣬���������õ�ַ
	W_com(0x80+y_bit);		//д��������
	W_com(start_column+x_byte);		//д�������
		 
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
//	W_com(0x36);		//������չָ���������ͼ��ʾ
//	W_com(0x30);		//��������ָ�       
}

/********************************
�ж�һ�����ص��Ƿ������������1��0
********************************/
uchar on_or_off(uchar x, uchar y)
{
	uchar x_byte;		//����ȷ�����������ĸ��ֽ�
	uchar x_bit;		//����ȷ����������һ���ֽڵ���λ
	uchar y_bit;		//����ȷ������������λ
	uchar start_column;		//����ȷ�����ص����ϰ������°���
	uchar temp_H;		//�����ݴ��LCD�����������ݣ��߰�λ
	uchar temp_L;		//�����ݴ��LCD�����������ݣ��Ͱ�λ

	x_byte=x/16;		//ȷ��Ϊһ���ϵĵڼ�����ַ
	x_bit=15-x%16;		//ȷ���Ը��ֽڵĵڼ�λ���в���

	y_bit=y%32;

	if(y<32)		//Ϊ��ҳ
		start_column=0x80;
	else		//����Ϊ��ҳ
		start_column=0x88;
	
	W_com(0x34);	//������չָ������رջ�ͼ��ʾ
	
	W_com(0x80+y_bit);		//д��������
	W_com(start_column+x_byte);		//д�������
	temp_H=R_data();		//�ٶ�
	temp_H=R_data();		//����λ
	temp_L=R_data();		//����λ
		 
	if(x_bit>7)
	{
		x_bit=x_bit%8;
		if((temp_H|(0x01<<x_bit))==temp_H)
			return 1;
		else
			return 0;
	}              
	else
	{
		if((temp_L|(0x01<<x_bit))==temp_L)
			return 1;
		else
			return 0;
	}
 
	W_com(0x36);		//������չָ���������ͼ��ʾ
}

/********************************
������λ�û���ˮƽ��
(x,y)������ʼ�㣬length������
********************************/
void W_x_line(uchar x, uchar y, uchar length)
{
	uchar i;
	for(i=0;i<length;i++)
	{
		W_pixel(x+i,y,1);
	}
}

/********************************
������λ�û��ƴ�ֱ��
(x,y)������ʼ�㣬length������
********************************/
void W_y_line(uchar x, uchar y, uchar length)
{
	uchar i;
	for(i=0;i<length;i++)
	{
		W_pixel(x,y+i,1);
	}
}

/********************************
���Ƶ�ͼ
********************************/
void W_map1()		//��һ����ͼ
{
//	Clear_GD();		//����
	
	W_com(0x34);		//������չָ������رջ�ͼ��ʾ

//��ߵľ��ο�
	W_x_line(0,0,70);		//��
	W_x_line(0,1,70);
	W_y_line(0,2,60);		//��
	W_y_line(1,2,60);
	W_x_line(0,62,70);		//��
	W_x_line(0,63,70);
	W_y_line(68,2,60);		//��
	W_y_line(69,2,60);

//	W_com(0x36);		//������չָ���������ͼ��ʾ
}

void W_map2()		//�ڶ�����ͼ���Ӹ�Ц��
{
	W_com(0x34);		//����չָ����رջ�ͼ��ʾ

	W_x_line(14,20,8);
	W_x_line(14,21,8);
	W_x_line(14,22,8);
	W_x_line(14,23,8);
	W_x_line(14,24,8);
	W_x_line(14,25,8);
	W_x_line(14,26,8);
	W_x_line(14,27,8);

	W_x_line(48,20,8);
	W_x_line(48,21,8);
	W_x_line(48,22,8);
	W_x_line(48,23,8);
	W_x_line(48,24,8);
	W_x_line(48,25,8);
	W_x_line(48,26,8);
	W_x_line(48,27,8);

	W_x_line(22,44,2);
	W_x_line(22,45,2);
	W_x_line(46,44,2);
	W_x_line(46,45,2);
	W_x_line(24,46,2);
	W_x_line(24,47,2);
	W_x_line(44,46,2);
	W_x_line(44,47,2);
	W_x_line(26,48,18);
	W_x_line(26,49,18);

	W_x_line(22,42,2);
	W_x_line(22,43,2);
	W_x_line(46,42,2);
	W_x_line(46,43,2);
}

/********************************
��Ϸ������ʾ����
********************************/
void W_word()
{
	uchar open_word[]={"����̰����С��Ϸ"};
	uchar unline[]={"By:K.B"};
	uchar i;
	i=0;
	delay(1);
	W_com(0x90);
	while(open_word[i]!='\0')
	{
		W_data(open_word[i]);
		i++;
	}

	i=0;
	LCD_pos(3,4);
	while(unline[i]!='\0')
	{
		W_data(unline[i]);
		i++;
	}
}

void W_map_word()		
{
	uchar open_map[]={"��ϲ��һ��"};
	uchar i;
	i=0;
	LCD_pos(1,1);
	while(open_map[i]!='\0')
	{
		W_data(open_map[i]);
		i++;
	}
}

void W_last_part()
{
	uchar last_word1[]={"����������"};
	uchar last_word2[]={"�Լ����ϰ����"};
	uchar i;
	i=0;
	LCD_pos(1,1);
	while(last_word1[i]!='\0')
	{
		W_data(last_word1[i]);
		i++;
	}
	i=0;
	LCD_pos(2,0);
	while(last_word2[i]!='\0')
	{
		W_data(last_word2[i]);
		i++;
	}
}

void W_over_word()
{
	uchar over_word1[]={"���ޣ����ޣ�"};
	uchar over_word2[]={"����һ�ξͺ��ˣ�"};
	uchar i;
	i=0;
	LCD_pos(1,1);
	while(over_word1[i]!='\0')
	{
		W_data(over_word1[i]);
		i++;
	}
	i=0;
	LCD_pos(2,0);
	while(over_word2[i]!='\0')
	{
		W_data(over_word2[i]);
		i++;
	}
}

void W_bye_word()
{
	uchar bye1[]={"���ǿ��"};
	uchar bye2[]={"�����㲻������"};
	uchar i;
	i=0;
	LCD_pos(1,2);
	while(bye1[i]!='\0')
	{
		W_data(bye1[i]);
		i++;
	}
	i=0;
	LCD_pos(2,0);
	while(bye2[i]='\0')
	{
		W_data(bye2[i]);
		i++;
	}
}

/********************************
LCD��ʼ��
********************************/
void LCD_init()
{
	delay(5);
	PSB=1;
	W_com(0x30);		//�򿪻���ָ�
	delay(5);
	W_com(0x30);
	delay(5);
	W_com(0x08);		//������ʾ�أ��α�أ��α�λ�ù�
	delay(5);
	W_com(0x0C);		//�رչ��
	delay(5);
	W_com(0x01);		//����
	delay(5);
	W_com(0x06);		//�������������ʾ���ƶ�
	delay(5);
}

void LCD_init_score()		//רΪ��ӡ�����ĳ�ʼ������
{
	delay(5);
	PSB=1;
	W_com(0x30);
	delay(5);
	W_com(0x30);
	delay(5);
	W_com(0x08);
	delay(5);
	W_com(0x14);
	delay(5);
	W_com(0x0C);
	delay(5);
	W_com(0x06);
	delay(5);	
	W_com(0x02);		//��ַ��λ�����ص�ԭ��
	delay(5);
} 

/******************************************
�߲���
******************************************/
/********************************
��ṹ�壬�ĸ����ص�Ϊһ��
********************************/
/*typedef struct Point
{
	uchar x_left_up;		//���ϵ�x����,ֻ��Ϊż��
	uchar y_left_up;		//���ϵ�y����,ֻ��Ϊż��
}Point;		*/

/********************************
������Ϩ��һ����ṹ�壬���ĸ����ص�
********************************/
void W_point(uchar x,uchar y,uchar color)
{
	if(color==1)
	{
		W_pixel(x,y,1);
		W_pixel(x+1,y,1);
		W_pixel(x,y+1,1);
		W_pixel(x+1,y+1,1);
	}
	else
	{
		W_pixel(x,y,0);
		W_pixel(x+1,y,0);
		W_pixel(x,y+1,0);
		W_pixel(x+1,y+1,0);
	}
}

/********************************
�����߽ṹ��
********************************/
typedef struct _Snake
{
	uchar x;		//��ͷ���Ϻ�����
	uchar y;		//��ͷ����������
	uchar direction;		//���ƶ�����,2�ϣ�4��6�ң�8��
	uchar score;		//����	
	uchar life;		//���Ƿ��1�����0��������	
	uchar flag;		//1��ʼ��0��ͣ
	uchar sub;		//��ͷ�������±�
	uchar speed;		//�ٶȣ���ֵԽ���ٶ�Խ��
}Snake;

uchar loc[13][2];		//����ߵ�����,�ﵽ10��������

/********************************
������ʼ����
********************************/
Snake snake;		//��

void init_snake()
{
	uchar i;
	delay(10);
	W_point(2,30,1);
	delay(10);
	W_point(4,30,1);
	delay(10);
	W_point(6,30,1);

//�ߵĳ�ʼ����	
	snake.x=6;
	snake.y=30;
	snake.life=1;
	snake.score=0;
	snake.direction=6;
	snake.flag=1;	
	snake.sub=2;
	snake.speed=100;

//��������Ϊ0��Ϊ��һ����׼��
	for(i=0;i<13;i++)
	{
		loc[i][0]=0;
		loc[i][1]=0;
	}
	
//����ʼ����Ž�����
	loc[0][0]=2;	
	loc[0][1]=30;
	loc[1][0]=4;
	loc[1][1]=30;	
	loc[2][0]=6;
	loc[2][1]=30;	  
}

/********************************
��ӡ����
********************************/
void W_score_word()
{
	uchar score_word[]={"�÷�"};
	uchar i;
	i=0;

	LCD_init_score();
	LCD_pos(0,6);
	while(score_word[i]!='\0')
	{
		W_data(score_word[i]);
		i++;
	}
	LCD_pos(1,7);
}

void W_score()
{
	LCD_init_score();
	LCD_pos(1,7);
	
	if(win_score==10)
	{
		LCD_pos(1,6);
		delay_nop();
		W_data(num[1]);
	}
	LCD_pos(1,7);
	W_data(num[win_score]);
}

/********************************
��ʱ����
********************************/
uchar min,sec,sec_ten,sec_ge;
void W_time()
{
	LCD_init_score();
	LCD_pos(3,7);
	delay(5);
	if(snake.flag==1)
	{
		delay_nop();
		sec_ten=sec/10;		//���ʮλ
		sec_ge=sec%10;		//��ĸ�λ				   
		
		if(sec==0)		//��Ҫ��ʱ����д�֣������ظ�дһ��������
		{
			LCD_pos(3,6);
			W_data(num[min]);
			W_data(':');
		}
		LCD_pos(3,7);
		W_data(num[sec_ten]);
		W_data(num[sec_ge]);

		if(sec==59)
		{
			min++;
			sec=0;
		}
		else
		{
			sec++;
		}
	}

}

/********************************
��������������ʳ������
********************************/
typedef struct Food		//ʳ��
{						
	uchar x;			
	uchar y;
}Food;

Food food;

void W_food()
{
	uchar x;
	uchar y;

	uchar temp_x;
	uchar temp_y;

	temp_x=loc[snake.sub-1][0];		//�����Ե�ʱ���ּ��������Ȼ��������
	temp_y=loc[snake.sub-1][1];		//���Խ����ѭ�������⡣
									//�гɹ���
	do				   //���Ե�ʱ���������һ�ػ�
	{				   //����ʳ�ﻭ��������������ѭ��
		srand(temp_x);		//���ߵĵڶ���������Ϊ���ӣ�
		x=2*(rand()%33+1);
		srand(temp_y);		//����ʳ����ֵ�λ�ò����ظ�
		y=2*(rand()%30+1);	
		temp_x++;		//��ֹ������ѭ�������ӵ���
		temp_y++;
	}
	while((on_or_off(x,y))==1);
	
	W_point(x,y,1);	

//��ʳ�����������
	food.x=x;
	food.y=y;
	
	W_com(0x36);		//������չָ���������ͼ��ʾ
}

/********************************
�������ж�
********************************/
void R_key()
{
	if(key_up==0)
	{
		delay(10);		//����
		if(key_up==0)
		{
			if(snake.direction!=8)
			{
				snake.direction=2;
			}
		}
	}
	if(key_left==0)
	{
		delay(10);
		if(key_left==0)
		{
			if(snake.direction!=6)
			{
				snake.direction=4;
			}
		}
	}
	if(key_down==0)
	{
		delay(10);
		if(key_down==0)
		{
			if(snake.direction!=2)
			{
				snake.direction=8;
			}
		}
	}
	if(key_right==0)
	{
		delay(10);
		if(key_right==0)
		{
			if(snake.direction!=4)
			{
				snake.direction=6;
			}
		}
	}
}

void R_key_sp()
{
	if(key_sp==0)
	{
		delay(10);
		if(key_sp==0)
		{
			snake.flag=~snake.flag;
		}
	}
}

/********************************
��ʼ��Ϸ�ж�
********************************/
void start_game()
{
	while(snake.flag==0)		//�Ȱ�������
	{
		R_key_sp();
	}
}

/********************************
ǰ��һ��
********************************/
void move_on()
{
	uint i;
	i=0;
	while(i<7000)		//���㹻��ʱ���жϷ�����Ƿ��£�ʱ��Ҳ����̫��
	{
		R_key();
		i++;
	}

	switch(snake.direction)		//�Է�������ж�
	{
		case 2:
		{
			if((on_or_off(snake.x,snake.y-2))==0)		//ǰ��Ϊ�գ�ֱ��ǰ��
			{
				snake.y=snake.y-2;		//����ͷ��ǰһ������Ϊ��ͷ
				delay(5);
				W_point(loc[0][0],loc[0][1],0);		//����βϨ��
				delay_nop();		//������ʱ����ֹ��ӡ����
				W_point(snake.x,snake.y,1);		//������ͷ
				W_com(0x36);
	
				for(i=0;i<snake.sub;i++)		//����������ƶ�
				{
					loc[i][0]=loc[i+1][0];
					loc[i][1]=loc[i+1][1];
				}						
				loc[snake.sub][0]=snake.x;
				loc[snake.sub][1]=snake.y;
			}
			else		//ǰ����Ϊ�գ��������������ʳ���ײǽ
			{
				if((snake.x==food.x)&&(snake.y-2==food.y))		//ʳ������
				{
					snake.score++;		//������1
					snake.sub++;		//��ͷ�����±��1

					//��ӡ�µķ���
					win_score=snake.score;
					W_score();
	
					snake.y=snake.y-2;		//������ͷ

					loc[snake.sub][0]=snake.x;
					loc[snake.sub][1]=snake.y;

					W_food();		//��ӡ�µ�ʳ�����
				}
				else		//ײǽ�����
				{
					snake.life=0;		//��ֱ�ӹ���
				}
			}
			break;
		}
		case 4:
		{
			if((on_or_off(snake.x-2,snake.y))==0)
			{
				snake.x=snake.x-2;
				delay(5);
				W_point(loc[0][0],loc[0][1],0);
				delay_nop();
				W_point(snake.x,snake.y,1);
				W_com(0x36);
	
				for(i=0;i<snake.sub;i++)
				{
					loc[i][0]=loc[i+1][0];
					loc[i][1]=loc[i+1][1];
				}						
				loc[snake.sub][0]=snake.x;
				loc[snake.sub][1]=snake.y;
			}
			else
			{
				if((snake.x-2==food.x)&&(snake.y==food.y))
				{
					snake.score++;
					snake.sub++;

					win_score=snake.score;
					W_score();
	
					snake.x=snake.x-2;

					loc[snake.sub][0]=snake.x;
					loc[snake.sub][1]=snake.y;
	
					W_food();
				}
				else
				{
					snake.life=0;
				}
			}
			break;
		}
		case 6:
		{
			if((on_or_off(snake.x+2,snake.y))==0)
			{
				snake.x=snake.x+2;
				delay(5);
				W_point(loc[0][0],loc[0][1],0);
				delay_nop();
				W_point(snake.x,snake.y,1); 
				W_com(0x36);
	
				for(i=0;i<snake.sub;i++)
				{
					loc[i][0]=loc[i+1][0];
					loc[i][1]=loc[i+1][1];
				}						
				loc[snake.sub][0]=snake.x;
				loc[snake.sub][1]=snake.y;
			}
			else
			{
				if((snake.x+2==food.x)&&(snake.y==food.y))
				{
					snake.score++;
					snake.sub++;

					win_score=snake.score;
					W_score();
	
					snake.x=snake.x+2;

					loc[snake.sub][0]=snake.x;
					loc[snake.sub][1]=snake.y;
	
					W_food();
				}
				else
				{
					snake.life=0;
				}
			}
			break;
		}
		case 8:
		{
			if((on_or_off(snake.x,snake.y+2))==0)
			{
				snake.y=snake.y+2;
				delay(5);
				W_point(loc[0][0],loc[0][1],0);
				delay_nop();
				W_point(snake.x,snake.y,1);
				W_com(0x36);
	
				for(i=0;i<snake.sub;i++)
				{
					loc[i][0]=loc[i+1][0];
					loc[i][1]=loc[i+1][1];
				}						
				loc[snake.sub][0]=snake.x;
				loc[snake.sub][1]=snake.y;
			}
			else
			{
				if((snake.x==food.x)&&(snake.y+2==food.y))
				{
					snake.score++;
					snake.sub++;

					win_score=snake.score;
					W_score();
	
					snake.y=snake.y+2;

					loc[snake.sub][0]=snake.x;
					loc[snake.sub][1]=snake.y;

					W_food();
				}
				else
				{
					snake.life=0;
				}
			}
			break;
		}
	}
}