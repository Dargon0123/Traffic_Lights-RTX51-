/******************************************************************************
@ subject  :Traffic cross instruction
@ version  :V1.0
@ designer :Dargon
@ date     :2021/04/18
@ contact  :dargon@163.com
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                     RTX_TINY:  Traffic Cross Instruction                   */
/*                                                                            */
/******************************************************************************/
#include <reg52.h>
#include <rtx51tny.h>                 /* RTX-51 tiny functions & defines      */
sbit led0 =P0^0;
sbit led1 =P0^1;
sbit led2 =P0^2;

long counter0;                        /* counter for task 0                   */
long counter1;                        /* counter for task 1                   */
long counter2;                        /* counter for task 2                   */
long counter3;                        /* counter for task 2                   */

#define uint unsigned int
#define uchar unsigned char
/*--------------------------------------
@ 函数测试：函数名声明区域
@ 函数说明：
--------------------------------------*/
void delay_ms(unsigned int n);
void delay(int z);
void display(int EW, int SN, int mode);
void matrix_key();
void init_timer();
void display_led();
void traffic_control();
void parameter_init();

/*--------------------------------------
@ 函数测试：单片机接口定义区域
@ 函数说明：
--------------------------------------*/
sbit SN_LED_RED =P0^0;
sbit SN_LED_YEL =P0^1;
sbit SN_LED_GRE =P0^2;

sbit EW_LED_RED =P0^3;
sbit EW_LED_YEL =P0^4;
sbit EW_LED_GRE =P0^5;

sbit EW_WALK_LED_RED =P0^7;
sbit EW_WALK_LED_GRE =P0^6;
sbit SN_WALK_LED_RED =P3^4;
sbit SN_WALK_LED_GRE =P3^3;

sbit wei0 =P3^0;
sbit wei1 =P3^1;
sbit wei2 =P3^2;


/*--------------------------------------
@ 函数测试：全局变量声明区域
@ 函数说明：
--------------------------------------*/
char temp[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f,0xff};
/* 数码管共阴极 */
char tableCathnode[] ={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c,0x39,0x5e,0x79,0x71}; 
/* 数码管共阳极 */
//char tableAnode[] ={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e };
/* 数码管 38译码器 位选数组 */
char wei[] ={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};

unsigned char nums =0, count =0;

int EW_LED_RealTime =0; /* 东西向数码管显示时间 15（通行）+ 3（黄灯） */
int SN_LED_RealTime =0; /* 南北向数码管显示时间 15（通行）+ 3（黄灯） */

int EW_base_time =15; /* 基准时间 */
int SN_base_time =15;
int EW_set_pass_time =15; /* 默认通行时间 */
int SN_set_pass_time =15;
int key1_count =0; /* 记录key1 按键 按下次数 */

int system_step_index =0; /* 系统基准步态 */
int Traffic_running_flag =1; /* 系统正常运行标志位 */
int Forbid_pass_flag =0; /* 禁止通行模式标志位 */

int EW_LED_YEL_flag =0; /* 东西 黄灯显示 标志位 */
int SN_LED_YEL_flag =0; /* 南北 黄灯显示 标志位 */
//int EW_LED_GRE_flag =0; /* 东西 绿灯显示 标志位 */
//int SN_LED_GRE_flag =0; /* 南北 绿灯显示 标志位 */
//int EW_LED_RED_flag =0; /* 东西 红灯显示 标志位 */
//int SN_LED_RED_flag =0; /* 南北 红灯显示 标志位 */



/******************************************************************************/
/*       Task 0 'job0':  RTX-51 tiny starts execution with task 0             */
/******************************************************************************/
void job0 () _task_ 0  {                  
  os_create_task (1);                 /* start task 1                         */
  os_create_task (2);                 /* start task 2                         */
  os_create_task (3);                 /* start task 3                         */
	
	parameter_init(); /* system init */
	init_timer();	
	
	os_delete_task (0);
}

/******************************************************************************/
/*   Task 1 'job1':  RTX-51 tiny starts this task with os_create_task (1)     */
/******************************************************************************/
job1 () _task_ 1  {
  while (1)  {                        /* endless loop                         */
    counter1++;                       /* increment counter 1                  */
		matrix_key();
    os_wait (K_TMO, 10, 0);           /* wait for timeout: 5 ticks 50ms          */
  }
}

/******************************************************************************/
/*    Task 2 'job2':  RTX-51 tiny starts this task with os_create_task (2)    */
/******************************************************************************/
job2 () _task_ 2  {
  while (1)  {                        /* endless loop                         */
    counter2++;                       /* increment counter 2                  */
		if(Traffic_running_flag ==1) /* 50ms */
			traffic_control(); /* 基本显示 */
		else		
			display(EW_set_pass_time, SN_set_pass_time, 0);
  }
}

/******************************************************************************/
/*    Task 3 'job3':  RTX-51 tiny starts this task with os_create_task (3)    */
/******************************************************************************/
job3 () _task_ 3  {
  while (1)  { 
		/* interrept service */
		count ++; 
		if(count ==20) 
		{
			display_led();
			count =0;
			EW_base_time --;
			SN_base_time --;
			if(EW_base_time <0)
				EW_base_time =EW_set_pass_time;
			if(SN_base_time <0)
				SN_base_time =SN_set_pass_time;
		}		
	/* endless loop                         */
    os_wait (K_SIG, 0, 0);            /* wait for signal                      */
    counter3++;                       /* process overflow from counter 2      */
  }
}


/******************************************************************************/
/*    下面是由 状态机 循环的思路实现 交通灯内容 进行移植   */
/******************************************************************************/
/******************************************************************************/
/*    下面是由 状态机 循环的思路实现 交通灯内容 进行移植   */
/******************************************************************************/
/*--------------------------------------
@ 函数测试：简单 1ms 时延
@ 函数说明：
--------------------------------------*/

/*--------------------------------------
@ 函数测试：普通时延 1ms
@ 函数说明：
--------------------------------------*/
void delay_ms(unsigned int n) 
{
	unsigned int i =0, j =0;
	for(i =0; i <n; i++)
	{
		for(j =0; j <120; j++);
	}
}
/*--------------------------------------
@ 函数测试：数码管显示
@ 函数说明：mode 0: 设置模式 1：运行模式 2：夜间模式
--------------------------------------*/
void display(int EW, int SN, int mode) 
{
	//P3 =wei[2]; /* display ge  */
	wei0 =0;
	wei1 =1;
	wei2 =0;
	P2 =tableCathnode[EW/10];
	delay_ms(5);
	//P2 =0x00;
	
	//P3 =wei[3]; /* display shi  */
	wei0 =1;
	wei1 =1;
	wei2 =0;
	P2 =tableCathnode[EW%10];
	delay_ms(5);
	//P2 =0x00;
	
	//P3 =wei[0]; /* display ge  */
	wei0 =0;
	wei1 =0;
	wei2 =0;
	P2 =tableCathnode[SN/10];
	delay_ms(5);
	//P2 =0x00;
	
	//P3 =wei[1]; /* display shi  */
	wei0 =1;
	wei1 =0;
	wei2 =0;
	P2 =tableCathnode[SN%10];
	delay_ms(5);
	//P2 =0x00;
	
	//P3 =wei[4]; /* SET_H  */
	wei0 =0;
	wei1 =0;
	wei2 =1;
	P2 =tableCathnode[mode/10];
	delay_ms(5);
	//os_wait (K_TMO, 1, 0);
	//P2 =0x00;
	
	//P3 =wei[5]; /* SET_L  */
	wei0 =1;
	wei1 =0;
	wei2 =1;
	P2 =tableCathnode[mode%10];
	delay_ms(5);
	//P2 =0x00;
}
/*--------------------------------------
@ 函数测试：普通按键
@ 函数说明：
--------------------------------------*/

/*--------------------------------------
@ 函数测试：矩阵按键
@ 函数说明：
(0 0): 1下 进入设置模式 东西调整 2下 南北调整 3下 正常运行模式
(0 1): 加操作
(0 2): 减操作
--------------------------------------*/
void matrix_key()
{
	unsigned char rowVal =0, colVal =0, keyVal =0;
	P1 =0xf0;
	if(P1 !=0xf0)
	{
		//delay_ms(10);
		rowVal =P1;
		P1 =0x0f;
		if(P1 !=0x0f)
		{
			colVal =P1;
		}
		keyVal =rowVal +colVal;		
	}
	
	/* 处理按键结果 */
	/* 第一行 0xe~ */
	if(keyVal ==0xee && key1_count ==0) /* 设置模式 设置东西道路通行时间 */
	{
		nums =0;
		key1_count ++;
		EW_set_pass_time ++;
		Traffic_running_flag =0; /* 进入设置模式 */
		
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xee && key1_count ==1) /* 设置模式 设置南北道路通行时间 */
	{
		nums =0;
		key1_count ++;
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xee && key1_count ==2) /* 设置模式 恢复正常 */
	{
		nums =0;
		key1_count =0;
		Traffic_running_flag =1; /* 系统复位 */
		parameter_init(); /* restart traffic system */
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xed) /* 加键 */
	{
		/* to do sth */
		nums--;
		if(key1_count ==1) /* 东西 */
			EW_set_pass_time ++;
		else if(key1_count ==2) /* 南北 */
			SN_set_pass_time ++;
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xeb) /* 减键 */
	{
		/* to do sth */
		nums +=2;
		if(key1_count ==1) /* 东西 */
			EW_set_pass_time --;
		else if(key1_count ==2) /* 南北 */
			SN_set_pass_time --;
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xe7) /* 夜间模式 */
	{
		/* to do sth */
		nums -=2;
		while(P1 !=0x0f);
	}
	/* 第二行0xd~	*/
	else if(keyVal ==0xde && Forbid_pass_flag ==0) /* 禁止通行模式 */
	{
		/* to do sth */
		Forbid_pass_flag =1;
		while(P1 !=0x0f);
	}
	else if(keyVal ==0xde && Forbid_pass_flag ==1) /* 禁止通行模式 */
	{
		/* to do sth */
		Forbid_pass_flag =0;
		while(P1 !=0x0f);
	}
	/* 第三行0xb~ */
	else if(keyVal ==0xbe)
	{
		/* to do sth */
		nums =14;
		while(P1 !=0x0f);
	}
	/* 第四行0x7~ */
	else if(keyVal ==0x7e)
	{
		/* to do sth */
		nums =13;
		while(P1 !=0x0f);
	}
	
//	if(Traffic_running_flag ==0)
//	{
//		display(EW_set_pass_time, SN_set_pass_time, 0);
//	}
//	if(nums >15)
//		nums =0;
	
	
		
}

/*--------------------------------------
@ 函数测试：定时器初始化 T0 and T1
@ 函数说明：
--------------------------------------*/
void init_timer() 
{
	TMOD =0x11; /* hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh !!!!!!!!!!!!!!!!!!!! */
	//TH0 =(65536-50000)/256; /* 50ms 初值配置 */
	//TL0 =(65536-50000)%256;
	TH1 =(65536-50000)/256; /* 50ms 初值配置 */
	TH1 =(65536-50000)%256;
	EA =1; 
	ET1 =1; 
	TR1 =1; 
}

/*--------------------------------------
@ 函数测试：定时器中断处理内容
@ 函数说明：
--------------------------------------*/

void T1_time() interrupt 3
{
	
	TH1 =(65536-50000)/256; /* 50ms 初值配置 */
	TH1 =(65536-50000)%256;
	isr_send_signal(3); /* 中断服务函数 创建 task3 处理 */
//	count ++;
//	if(count ==20) 
//	{
//		display_led();
//		count =0;
//		EW_base_time --;
//		SN_base_time --;
//		if(EW_base_time <0)
//			EW_base_time =EW_set_pass_time;
//		if(SN_base_time <0)
//			SN_base_time =SN_set_pass_time;
//	}
}

/*--------------------------------------
@ 函数测试：LED显示控制
@ 函数说明：
--------------------------------------*/
void display_led()
{
	if(Forbid_pass_flag ==0)
	{
		switch(system_step_index) 
		{
			case 0: /* 状态0 东西向绿灯 南北向红灯 */
			{
				/* drive */
				//SN_LED_RED =1;
				SN_LED_YEL =1;
				SN_LED_GRE =1;
				
				EW_LED_RED =1;
				EW_LED_YEL =1;
				//EW_LED_GRE =1;
				
				EW_LED_GRE =~EW_LED_GRE; /* green */
				SN_LED_RED =~SN_LED_RED;
				
				/* walk */
				EW_WALK_LED_RED =1;
				SN_WALK_LED_GRE =1;
				
				EW_WALK_LED_GRE =~EW_WALK_LED_GRE;
				SN_WALK_LED_RED =~SN_WALK_LED_RED;
				break;
			}
			case 1: /* 状态1 东西向黄灯 南北向红灯 */
			{
				//SN_LED_RED =1;
				SN_LED_YEL =1;
				SN_LED_GRE =1;
				
				EW_LED_RED =1;
				//EW_LED_YEL =1;
				EW_LED_GRE =1;
				
				EW_LED_YEL =~EW_LED_YEL; /* yellow */
				SN_LED_RED =~SN_LED_RED;
				
				/* walk */
				EW_WALK_LED_RED =1;
				SN_WALK_LED_GRE =1;
				
				EW_WALK_LED_GRE =~EW_WALK_LED_GRE;
				SN_WALK_LED_RED =~SN_WALK_LED_RED;
				break;
			}
			case 2: /* 状态2 东西向红灯 南北向绿灯 */
			{
				SN_LED_RED =1;
				SN_LED_YEL =1;
				//SN_LED_GRE =1;
				
				//EW_LED_RED =1;
				EW_LED_YEL =1;
				EW_LED_GRE =1;
				
				EW_LED_RED =~EW_LED_RED; 
				SN_LED_GRE =~SN_LED_GRE; /* green */
				
				/* walk */
				SN_WALK_LED_RED =1;
				EW_WALK_LED_GRE =1;
				
				SN_WALK_LED_GRE =~SN_WALK_LED_GRE;
				EW_WALK_LED_RED =~EW_WALK_LED_RED;
				break;
			}
			case 3: /* 状态3 东西向红灯 南北向黄灯 */
			{
				SN_LED_RED =1;
				//SN_LED_YEL =1;
				SN_LED_GRE =1;
				
				//EW_LED_RED =1;
				EW_LED_YEL =1;
				EW_LED_GRE =1;
				
				EW_LED_RED =~EW_LED_RED;
				SN_LED_YEL =~SN_LED_YEL; /* yellow */
				
				/* walk */
				SN_WALK_LED_RED =1;
				EW_WALK_LED_GRE =1;
				
				SN_WALK_LED_GRE =~SN_WALK_LED_GRE;
				EW_WALK_LED_RED =~EW_WALK_LED_RED;
				break;
			}
			default :
				break;
		}
	}
	else  /* 禁止模式 红灯闪烁 绿灯全灭 */
	{		
		EW_LED_GRE =1;
		SN_LED_GRE =1;
		EW_LED_RED =~EW_LED_RED; 
		SN_LED_RED =~SN_LED_RED;			
		/* walk */	
		SN_WALK_LED_GRE =1;
		EW_WALK_LED_GRE =1;
		SN_WALK_LED_RED =~SN_WALK_LED_RED;
		EW_WALK_LED_RED =~EW_WALK_LED_RED;
	}
}
/*--------------------------------------
@ 函数测试：交通控制
@ 函数说明：
--------------------------------------*/
void traffic_control()
{
	EW_LED_RealTime =EW_base_time;
	SN_LED_RealTime =SN_base_time;
	if(Forbid_pass_flag ==0)
	{
		switch(system_step_index)
		{
			case 0: /* 状态0 东西向绿灯 南北向红灯 */
			{
				if(SN_LED_RealTime <=3)
				{
					SN_LED_YEL_flag =1;
					system_step_index =1;
				}
				display(EW_LED_RealTime-3, SN_LED_RealTime, 1);
				break;
			}
			case 1: /* 状态1 东西向黄灯 南北向红灯 */
			{
				if(SN_LED_RealTime >3)
				{
					SN_LED_YEL_flag =0;
					system_step_index =2;
				}
				display(EW_LED_RealTime, SN_LED_RealTime, 1);
				break;
			}
			case 2: /* 状态2 东西向红灯 南北向绿灯 */
			{
				if(EW_LED_RealTime <=3)
				{
					EW_LED_YEL_flag =1;
					system_step_index =3;
				}
				display(EW_LED_RealTime, SN_LED_RealTime-3, 1);
				break;
			}
			case 3: /* 状态1 东西向红灯 南北向黄灯 */
			{
				if(EW_LED_RealTime >3)
				{
					EW_LED_YEL_flag =0;
					system_step_index =0;
				}
				display(EW_LED_RealTime, SN_LED_RealTime, 1);
				break;
			}
			default:
				break;
		}
	}
	else 
	{
		display(0, 0, 1);
	}
}
/*--------------------------------------
@ 函数测试：外部数据接口初始化
@ 函数说明：
--------------------------------------*/
void parameter_init()
{
	EW_base_time =EW_set_pass_time;
	SN_base_time =SN_set_pass_time;
}
/*--------------------------------------
@ 函数测试：主程序函数
@ 函数说明：
--------------------------------------*/
//void main()
//{
//	parameter_init();
//	init_timer();
//	while(1) 
//	{
//		matrix_key();
//		if(Traffic_running_flag ==1)
//			traffic_control();
//	}
//	return;
//	
//}





