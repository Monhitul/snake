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

/********************************
判断一个像素点是否点亮，并返回1或0
********************************/
uchar on_or_off(uchar x, uchar y)
{
	uchar x_byte;		//用于确定横坐标在哪个字节
	uchar x_bit;		//用于确定横坐标在一个字节的哪位
	uchar y_bit;		//用于确定纵坐标在哪位
	uchar start_column;		//用于确定像素点是上半屏或下半屏
	uchar temp_H;		//用于暂存从LCD读出来的数据，高八位
	uchar temp_L;		//用于暂存从LCD读出来的数据，低八位

	x_byte=x/16;		//确定为一行上的第几个地址
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
 
	W_com(0x36);		//开启扩展指令集，开启绘图显示
}

/********************************
在任意位置绘制水平线
(x,y)代表起始点，length代表长度
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
在任意位置绘制垂直线
(x,y)代表起始点，length代表长度
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
绘制地图
********************************/
void W_map1()		//第一个地图
{
//	Clear_GD();		//清屏
	
	W_com(0x34);		//开启扩展指令集，并关闭绘图显示

//左边的矩形框
	W_x_line(0,0,70);		//上
	W_x_line(0,1,70);
	W_y_line(0,2,60);		//左
	W_y_line(1,2,60);
	W_x_line(0,62,70);		//下
	W_x_line(0,63,70);
	W_y_line(68,2,60);		//右
	W_y_line(69,2,60);

//	W_com(0x36);		//开启扩展指令集，开启绘图显示
}

void W_map2()		//第二个地图，加个笑脸
{
	W_com(0x34);		//打开扩展指令集，关闭绘图显示

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
游戏文字显示界面
********************************/
void W_word()
{
	uchar open_word[]={"智障贪吃蛇小游戏"};
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
	uchar open_map[]={"恭喜下一关"};
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
	uchar last_word1[]={"不够你玩了"};
	uchar last_word2[]={"自己画障碍物吧"};
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
	uchar over_word1[]={"不哭！不哭！"};
	uchar over_word2[]={"再玩一次就好了！"};
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
	uchar bye1[]={"你很强！"};
	uchar bye2[]={"相信你不是智障"};
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
LCD初始化
********************************/
void LCD_init()
{
	delay(5);
	PSB=1;
	W_com(0x30);		//打开基本指令集
	delay(5);
	W_com(0x30);
	delay(5);
	W_com(0x08);		//整体显示关，游标关，游标位置关
	delay(5);
	W_com(0x0C);		//关闭光标
	delay(5);
	W_com(0x01);		//清屏
	delay(5);
	W_com(0x06);		//光标右移整体显示不移动
	delay(5);
}

void LCD_init_score()		//专为打印分数的初始化函数
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
	W_com(0x02);		//地址归位，光标回到原点
	delay(5);
} 

/******************************************
蛇部分
******************************************/
/********************************
点结构体，四个像素点为一点
********************************/
/*typedef struct Point
{
	uchar x_left_up;		//左上点x坐标,只能为偶数
	uchar y_left_up;		//左上点y坐标,只能为偶数
}Point;		*/

/********************************
点亮或熄灭一个点结构体，即四个像素点
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
建立蛇结构体
********************************/
typedef struct _Snake
{
	uchar x;		//蛇头左上横坐标
	uchar y;		//蛇头左上纵坐标
	uchar direction;		//蛇移动方向,2上，4左，6右，8下
	uchar score;		//分数	
	uchar life;		//蛇是否存活，1代表存活，0代表死亡	
	uchar flag;		//1开始，0暂停
	uchar sub;		//蛇头的数组下标
	uchar speed;		//速度，数值越大速度越慢
}Snake;

uchar loc[13][2];		//存放蛇的坐标,达到10个即过关

/********************************
建立初始蛇身
********************************/
Snake snake;		//蛇

void init_snake()
{
	uchar i;
	delay(10);
	W_point(2,30,1);
	delay(10);
	W_point(4,30,1);
	delay(10);
	W_point(6,30,1);

//蛇的初始数据	
	snake.x=6;
	snake.y=30;
	snake.life=1;
	snake.score=0;
	snake.direction=6;
	snake.flag=1;	
	snake.sub=2;
	snake.speed=100;

//对数组清为0，为下一关做准备
	for(i=0;i<13;i++)
	{
		loc[i][0]=0;
		loc[i][1]=0;
	}
	
//将初始蛇身放进数组
	loc[0][0]=2;	
	loc[0][1]=30;
	loc[1][0]=4;
	loc[1][1]=30;	
	loc[2][0]=6;
	loc[2][1]=30;	  
}

/********************************
打印分数
********************************/
void W_score_word()
{
	uchar score_word[]={"得分"};
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
计时函数
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
		sec_ten=sec/10;		//秒的十位
		sec_ge=sec%10;		//秒的个位				   
		
		if(sec==0)		//需要的时候再写分，避免重复写一样的数据
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
产生随机数来获得食物坐标
********************************/
typedef struct Food		//食物
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

	temp_x=loc[snake.sub-1][0];		//最后调试的时候发现加上这个，然后自增，
	temp_y=loc[snake.sub-1][1];		//可以解决死循环的问题。
									//有成功过
	do				   //调试的时候发现在最后一关会
	{				   //由于食物画不出来而陷入死循环
		srand(temp_x);		//将蛇的第二个坐标作为种子，
		x=2*(rand()%33+1);
		srand(temp_y);		//这样食物出现的位置不会重复
		y=2*(rand()%30+1);	
		temp_x++;		//防止陷入死循环，种子递增
		temp_y++;
	}
	while((on_or_off(x,y))==1);
	
	W_point(x,y,1);	

//将食物坐标存下来
	food.x=x;
	food.y=y;
	
	W_com(0x36);		//开启扩展指令集，开启绘图显示
}

/********************************
按键的判断
********************************/
void R_key()
{
	if(key_up==0)
	{
		delay(10);		//消抖
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
开始游戏判断
********************************/
void start_game()
{
	while(snake.flag==0)		//等按键按下
	{
		R_key_sp();
	}
}

/********************************
前进一步
********************************/
void move_on()
{
	uint i;
	i=0;
	while(i<7000)		//给足够的时间判断方向键是否按下，时间也不能太长
	{
		R_key();
		i++;
	}

	switch(snake.direction)		//对方向进行判断
	{
		case 2:
		{
			if((on_or_off(snake.x,snake.y-2))==0)		//前方为空，直接前进
			{
				snake.y=snake.y-2;		//将蛇头的前一个点设为蛇头
				delay(5);
				W_point(loc[0][0],loc[0][1],0);		//将蛇尾熄灭
				delay_nop();		//给个延时，防止打印出错
				W_point(snake.x,snake.y,1);		//点亮蛇头
				W_com(0x36);
	
				for(i=0;i<snake.sub;i++)		//对数组进行移动
				{
					loc[i][0]=loc[i+1][0];
					loc[i][1]=loc[i+1][1];
				}						
				loc[snake.sub][0]=snake.x;
				loc[snake.sub][1]=snake.y;
			}
			else		//前方不为空，则有两种情况，食物跟撞墙
			{
				if((snake.x==food.x)&&(snake.y-2==food.y))		//食物的情况
				{
					snake.score++;		//分数加1
					snake.sub++;		//蛇头数组下标加1

					//打印新的分数
					win_score=snake.score;
					W_score();
	
					snake.y=snake.y-2;		//调整蛇头

					loc[snake.sub][0]=snake.x;
					loc[snake.sub][1]=snake.y;

					W_food();		//打印新的食物出来
				}
				else		//撞墙的情况
				{
					snake.life=0;		//蛇直接挂了
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