#include "main.h"
#include <string.h>
#include "task.h"
#include "Display.h"


SysTemPat sys;

#define MAXERRTIMES 5  //最大允许错误次数

//要写入到STM32 FLASH的字符串数组
const u8 TEXT_Buffer[]={0x17,0x23,0x6f,0x60,0,0};
#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//数组长度	
#define SIZE TEXT_LENTH/4+((TEXT_LENTH%4)?1:0)
#define FLASH_SAVE_ADDR  0X0802C124 	//设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
										//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.
                                                                                                                                                               锳S608参数                     
u8** kbd_tbl;

void Display_Data(void);//显示时间
void ShowErrMessage(u8 ensure);//显示确认码错误信息
int password(void);//密码锁
void SetPassworld(void);//修改密码
void starting(void);//开机界面信息
u8 MFRC522_lock(void);//刷卡解锁
u8 Add_Rfid(void);		//录入
void Set_Time(void); //设置时间
void Massige(void);  //显示文本
void SysPartInit(void );   //系统参数初始化 
//u8 Pwd[7]="      ";  //解锁密码1
//u8 Pwd2[7]="      ";  //解锁密码2
//u8 cardid[6]={0,0,0,0,0,0};  //卡号1
int Error;  //密码验证信息


u8 DisFlag = 1;

//数字的ASCII码
uc8 numberascii[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
//显示缓冲区
u8  dispnumber5buf[6];
u8  dispnumber3buf[4];
u8  dispnumber2buf[3];
//MFRC522数据区
u8  mfrc552pidbuf[18];
u8  card_pydebuf[2];
u8  card_numberbuf[5]; //卡片物理id
u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  card_readbuf[18];
//SM05-S数据区
u8  sm05cmdbuf[15]={14,128,0,22,5,0,0,0,4,1,157,16,0,0,21};
//extern声明变量已在外部的C文件里定义，可以在主文件中使用
extern u8  sm05receivebuf[16];	//在中断C文件里定义
extern u8  sm05_OK;							//在中断C文件里定义


//日期
u8 * week[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};


#if USE_FINGERPRINT //使用指纹
#define MaxParaNum 6 
u8 * setup[7]={"1、录入指纹","2、删除指纹","3、修改密码","4、修改时间","5、录入卡片","6、查看信息"};
#else
#define MaxParaNum 4
u8 * setup[7]={"1、修改密码","2、修改时间","3、录入卡片","4、查看信息","           ","           "};
#endif



int main(void)
{			
	u16 set=0;
	 u8 err=0;
	int key_num;
	int time1;
	int time2;		//锁屏时间
	char arrow=0;  //箭头位子

    

/*********************
***  各模块初始化  ***
*********************/
	delay_init();	    	    //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	        //串口初始化为9600
	printf("串口功能正常\r\n");
	Button4_4_Init();           //按键初始化
	OLED_Init();                //OLED初始化
	Walkmotor_Init();           //步进电机初始化
	BEEP_Init();			    //蜂鸣器初始化
	usart2_init(usart2_baund);  //串口2初始化
	PS_StaGPIO_Init();          //指纹初始化
	OLED_Clear();  //OLED清屏
	 
	starting();//开机信息  logo
	err = RTC_Init();   //RTC时钟初始化
    if(err) { 	//初始化时钟失败,晶振有问题
        OLED_Clear(); 
        Show_Str(12,13,128,20,"RTC CRY ERR!",12,0); //初始化失败 
        OLED_Refresh_Gram();  delay_ms(3000);  }
	SysPartInit();   //系统参数初始化 
 	
    
    while(1)
	{
//锁屏界面
MAIN:
     OLED_Clear(); 
     OLED_Show_Font(56,48,0);//显示锁图标
     while(1)
    {
        time1++;Display_Data();//时间显示：每1000ms更新一次显示数据
				
        if(DisFlag == 1)
        {
            DisFlag = 0;
            OLED_Fill(0,24,16,63,0); //清空屏幕
            OLED_Refresh_Gram();//更新显示
        }
				
        if((time1%100)==1) //每隔100ms更新解锁
        {
            /***********
            * 刷卡解锁 *
            ***********/
            time1=0;
            MFRC522_Initializtion();  //IC卡初始化	
            Error = MFRC522_lock();  //读IC卡 解锁
            if(Error == 0)//解锁成功
            {    goto MENU; } // 前往主页面
            else 
            {    OLED_Show_Font(56,48,0);   } //锁
                
            /****************
            * 蓝牙解锁密码1 *
            ****************/
            Error = usart1_cherk((char*)sys.passwd1);   //检测蓝牙输入密码与设置的密码是否匹配   
            if(Error == 0) {   //解锁成功
                OLED_Clear_NOupdate(); //清屏
                Show_Str(12,13,128,20,"蓝牙密码1：正确",12,0); 
                OLED_Refresh_Gram();//更新显示
                delay_ms(800);
                DisUnLock(); //解锁
                goto MENU;	}
            else  {
//              OLED_Clear_NOupdate();
//              Show_Str(12,13,128,12,"蓝牙密码：错误！",12,0); 
//              OLED_Refresh_Gram();//更新显示
//              delay_ms(800);
//              OLED_Show_Font(56,48,0);  //锁
            }
            /****************
            * 蓝牙解锁密码2 *
            ****************/
            Error=usart1_cherk((char*)sys.passwd2);         
            if(Error==0){
                sys.errCnt = 0;
                OLED_Clear_NOupdate();
                Show_Str(12,13,128,12,"蓝牙密码2：正确",12,0); 
                OLED_Refresh_Gram();//更新显示
                delay_ms(800);
                DisUnLock();
                goto MENU;	
               }
            else 
                {
                //OLED_Show_Font(56,48,0);//锁
                }
						
			}
                          
            /***********
            * 指纹解锁 *
            ***********/
            if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
            {
                while(PS_Sta){
                  Error=press_FR();//刷指纹
                  if(Error==0) {
                       DisUnLock();
                     goto MENU; }  //跳到解锁界面
                  else  {
                     OLED_Show_Font(56,48,0); }  //锁
				}
			}
                
            /***********
            * 密码解锁 *
            ***********/
            key_num=Button4_4_Scan();	//按键扫描
            if(key_num!=-1) //如果输入在1-9之内有效输入
				{
					Error=password();//密码解锁函数
					if(Error==0)
					{
						goto MENU;	//跳到解锁界面
					}
					else 
					{
						OLED_Show_Font(56,48,0);//锁
					}
				}
				delay_ms(1);									
			}
    
            
			/**************************
                     主界面
            **************************/

MENU:
			OLED_Clear();
MENUNOCLR:   //主页功能选择
			OLED_Fill(0,0,20,48,0); 
			//主页菜单显示  
			if(arrow<3){   //用法：用于分页显示菜单项，当选项超过一定数量时，通过调整显示位置来适应屏幕空间。
				Show_Str(5,arrow*16,128,16,"->",16,0);//显示箭头
				set=0;} 
			else {
				Show_Str(5,(arrow-3)*16,128,16,"->",16,0);
				set=3;}
            
			Show_Str(25,0,128,16,setup[set],16,0);      //功能1 -->  录入指纹
			Show_Str(25,16,128,16,setup[set+1],16,0);   //功能2 -->  删除指纹
			Show_Str(25,32,128,16,setup[set+2],16,0);   //功能3 -->  修改密码
			Show_Str(0,52,128,12,"上    下     确定",12,0); //按键提示
			OLED_Refresh_Gram();//更新显示
			time2=0;
			while(1)
			{
                        /***********
                        * 超时锁屏 *
                        ***********/			
                        time2++;
						if(time2>10000 | key_num==4){  
							OLED_Clear();
								DisLock(); //解锁
								if(time2>10000) beep_on_mode2(); //蜂鸣器鸣叫
								time2 = 0; //解锁后清除超时检测
								//delay_ms(1000);
								OLED_Clear();
								goto MAIN;
						}
                        /***********
                        * 蓝牙锁屏 *
                        ***********/
						if(memcmp(USART_RX_BUF,"LOCK",4)==0)	{
//							USART_RX_STA=0;
//							memset(USART_RX_BUF,0,USART_REC_LEN);
							DisLock();
							goto MAIN;
						}
                                    
                        /***********
                        * 功能选择 *
                        ***********/
						key_num=Button4_4_Scan();	 //扫描按键
						if(key_num)
						{
							if(key_num==13){
								if(arrow>0)arrow--;  //向上索引，箭头返回上一个
								goto MENUNOCLR;  //
							}
							if(key_num==15){
								if(arrow<MaxParaNum-1)arrow++; //向下索引，箭头前进下一个
								goto MENUNOCLR;  
							}
							if(key_num==16){
								switch(arrow)
								{
#if USE_FINGERPRINT
									case 0:Add_FR();	break;   //录入指纹
									case 1:Del_FR();	break;   //删除指纹
									case 2:SetPassworld();break; //修改密码
									case 3:Set_Time(); break;    //设置时间
									case 4:Add_Rfid(); break;    //录入卡片
									case 5:Massige(); break;     //显示信息
#else
									case 0:SetPassworld();break; //修改密码
									case 1:Set_Time(); break;    //设置时间
									case 2:Add_Rfid(); break;    //录入卡片
									case 3:Massige(); break;     //显示信息
#endif									
								}
								goto MENU;
							}		
						}delay_ms(1);
			}	
	}//while
			
			
			
			
		
		 
 }
 
//错误显示
 u8 DisErrCnt(void)
 {
	 int time=0;
	 u8 buf[64];
	 if(sys.errTime>0)//错误次数计数
	{
		OLED_Clear();
		while(1)
		{
			if(time++ == 1000) //每1000ms刷新
			{
				time = 0;
				if(sys.errTime==0) 
				{
					OLED_Clear();
					break;
				}
				Show_Str(0,16,128,16,"密码错误次数过多",16,0);
				sprintf(buf,"请%02d秒后重试", sys.errTime);
				Show_Str(20,32,128,16,buf,16,0);
				OLED_Refresh_Gram();//更新显示
			}
			delay_ms(1);
			if(4 == Button4_4_Scan())//返回
			{
				OLED_Clear();
				return 1;
			}
		}
	}
 }
 

 

//密码锁
int password(void)
{
	int  key_num=0,i=0,satus=0;
	u16 num=0,num2=0,time3=0,time;
	u8 pwd[11]="          ";
	u8 hidepwd[11]="          ";
	u8 buf[64];
	OLED_Clear();//清屏

	if(DisErrCnt())return -1;//错误次数超限
	
	OLED_Clear();//清屏
	Show_Str(5,0,128,16,"密码：",16,0);
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Clr  OK",12,0);
	OLED_Refresh_Gram();//更新显示
//	Show_Str(102,36,128,12,"显示",12,0);
//	Show_Str(0,52,128,12,"删除 清空   返回 确认",12,0);
	while(1)
	{
		key_num=Button4_4_Scan();  //获取按键健码
		if(key_num != -1)
		{	
            printf("key=[%d]\r\n",key_num); //显示按键码
			DisFlag = 1;
			time3=0;
			if(key_num != -1)
			{	
				DisFlag = 1;
				time3=0;
				switch(key_num)
				{
					case 1:
					case 2:
					case 3:
                            pwd[i]=key_num+0x30; //1-3
                            hidepwd[i]='*';  //隐藏密码
                            i++;
                            break;
					case 4://返回
                            OLED_Clear();
                            delay_ms(500);
                            return -1;  break;
					case 5:
					case 6:
					case 7:
						pwd[i]=key_num+0x30-1; //4-6 
						hidepwd[i]='*';
						i++;
					break;
					case 8:
						if( i > 0){
							pwd[--i]=' ';  //‘del’键
							hidepwd[i]=' '; 
						}
					break;
					case 9:
					case 10:
					case 11:
						pwd[i]=key_num+0x30-2; //4-6
						hidepwd[i]='*';
						i++;
					break;
					case 12:satus=!satus; break;//DIS
					case 13:
					case 15:
						while(i--){
						pwd[i]=' ';  //‘清空’键
						hidepwd[i]=' '; 
						}
						i=0;
					break;
					case 14:
						pwd[i]=0x30; //4-6
						hidepwd[i]='*';
						i++;
					break;
					case 16:
						goto UNLOCK; //前往解锁验证
					break;
				}
		}
		if(DisFlag == 1)
		{
            if(satus==0) OLED_ShowString(53,0,hidepwd,12);  //隐藏输入的密码
            else         OLED_ShowString(53,0,pwd,12);      //显示输入的密码
            OLED_Refresh_Gram();//更新显示
		}
		
		time3++;
		if(time3%1000==0){
			OLED_Clear();//清屏
			return -1;
		}
	}
}

UNLOCK:	
		for(i=0; i<10; i++){   //验证虚伪密码
			if(pwd[i]==sys.passwd1[num])num++;
				else num=0;
			if(num==6)
				break;
		}
		for(i=0; i<10; i++){   //验证密码
			if(pwd[i]==sys.passwd2[num2])num2++;
				else num2=0;
			if(num2==6)
				break;
		}
		if(num==6 | num2==6){
			DisUnLock(); //解锁
			OLED_Clear();//清屏
			sys.errCnt = 0;
			return 0;
		}
		else {
			sys.errCnt++;//错误次数计数
			if(sys.errCnt>MAXERRTIMES) //如果超过最大错误限制次数
				sys.errTime = 30; //30秒不能再解锁
			OLED_Clear();//清屏
			Show_Str(45,48,128,16,"密码错误！",16,0);
			OLED_Refresh_Gram();//更新显示
			beep_on_mode1();
			delay_ms(1500);
			OLED_Clear();//清屏
			return -1;
		}
	
}


//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	Show_Str(0,48,128,12,(u8*)EnsureMessage(ensure),12,0);	
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
	OLED_ShowString(0,48,"                   ",12);	
	
	OLED_Refresh_Gram();//更新显示
}
/***********
* 指纹录取 *
***********/
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	int key_num;
	u16 ID;
	OLED_Clear();//清屏
	while(1)
	{
		key_num = Button4_4_Scan();	
		if(key_num==16){ //返回
			OLED_Clear();//清屏
			return ;
		}
		switch (processnum)
		{
			case 0: //录入指纹
				//OLED_Clear();//清屏
				i++;
				Show_Str(0,0,128,16,"=== 录入指纹 ===",16,0);
				Show_Str(0,24,128,12,"请按指纹！  ",12,0);	
				Show_Str(104,52,128,12,"返回",12,0);		
				OLED_Refresh_Gram();//更新显示	
				ensure=PS_GetImage(); //指纹识别判断
				if(ensure==0x00)  //录入过程
				{
					BEEP=0; //蜂鸣器提示
					ensure=PS_GenChar(CharBuffer1);//生成特征
					BEEP=1;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"指纹正常！    ",12,0);
						OLED_Refresh_Gram();  //更新显示	
						i=0;
						processnum=1;  //跳到第二步						
					}else ShowErrMessage(ensure);	 //显示指纹录取错误			
				}else ShowErrMessage(ensure); //显示指纹录取错误
				//OLED_Clear();//清屏
				break;
			
			case 1: //再次录入此指纹
				i++;
				Show_Str(0,24,128,12,"请再按一次指纹",12,0);
				OLED_Refresh_Gram();//更新显示		
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=0;
					ensure=PS_GenChar(CharBuffer2);//生成特征
					BEEP=1;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"指纹正常！",12,0);	
						OLED_Refresh_Gram();//更新显示
						i=0;
						processnum=2;//跳到第三步
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				//OLED_Clear();//清屏
				break;

			case 2:	 //对比指纹
				Show_Str(0,24,128,12,"对比两次指纹        ",12,0);
				OLED_Refresh_Gram();//更新显示
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					Show_Str(0,24,128,12,"两次指纹一样       ",12,0);
					OLED_Refresh_Gram();//更新显示
					processnum=3;//跳到第四步
				}
				else  //指纹不同
				{
					Show_Str(0,24,128,12,"对比失败 请重录    ",12,0);	
					OLED_Refresh_Gram();//更新显示
					ShowErrMessage(ensure);
					i=0;
					OLED_Clear();//清屏
					processnum=0;//跳回第一步		
				}
				delay_ms(1200);
				//OLED_Clear();//清屏
				break;

			case 3: //生成指纹模版
			Show_Str(0,24,128,12,"生成指纹模板...    ",12,0);
			OLED_Refresh_Gram();//更新显示	
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					Show_Str(0,24,128,12,"生成指纹模板成功!",12,0);
					OLED_Refresh_Gram();//更新显示
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	 //录入ID
				//OLED_Clear();//清屏
			Show_Str(0,24,128,12,"请输入储存ID:        ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"删除 清空      确认",12,0);
			OLED_Refresh_Gram();//更新显示
				do
					ID=GET_NUM(); //获取用户输入的ID
				while(!(ID<AS608Para.PS_max));//输入ID必须小于模块容量最大的数值
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
			
                if(ensure==0x00)  //存储成功
				{			
                    OLED_Clear_NOupdate();//清屏
					Show_Str(0,30,128,16,"录指纹成功!",16,0);	
					PS_ValidTempleteNum(&ValidN);//读库指纹个数
					Show_Str(66,52,128,12,"剩余",12,0);
					OLED_ShowNum(90,52,AS608Para.PS_max-ValidN,3,12);
					OLED_Refresh_Gram();//更新显示
					delay_ms(1500);
					OLED_Clear();	
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}
				OLED_Clear();//清屏					
				break;				
		}
		delay_ms(400);
		if(i==10)//超过5次没有按手指则退出
		{
			OLED_Clear();
			break;
		}				
	}
}

//刷指纹
int press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char str[256];
	
	
	if(DisErrCnt())return -1;//错误次数超限
	ensure=PS_GetImage();
	
	OLED_Clear_NOupdate();
	Show_Str(0,0,128,16,"正在检测指纹",16,0);
	OLED_Refresh_Gram();//更新显示
	if(ensure==0x00)//获取图像成功 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
			{				
				OLED_Clear_NOupdate();
				Show_Str(20,10,128,24,"解锁中...",24,0);	
				OLED_Refresh_Gram();//更新显示
				Walkmotor_ON();
				Show_Str(20,10,128,24,"已解锁！",24,0);
				OLED_Refresh_Gram();//更新显示
				OLED_Show_Font(112,18,1);//开锁				
				//str=mymalloc(SRAMIN,2000);
				sprintf(str,"ID:%d     匹配分",seach.pageID);
				Show_Str(0,52,128,12,(u8*)str,12,0);	
				sprintf(str,":%d",seach.mathscore);
				Show_Str(96,52,128,12,(u8*)str,12,0);	
				//myfree(SRAMIN,str);
				OLED_Refresh_Gram();//更新显示
				delay_ms(1800);
				OLED_Clear();
				return 0;
			}
			else {
				sys.errCnt++;//错误次数计数
				if(sys.errCnt>MAXERRTIMES)
					sys.errTime = 30; //30秒不能再解锁
				ShowErrMessage(ensure);	
				OLED_Refresh_Gram();//更新显示
				beep_on_mode1();
				OLED_Clear();
				return -1;
			}				
	  }
		else
			ShowErrMessage(ensure);
		
	OLED_Refresh_Gram();//更新显示
	 delay_ms(2000);
		OLED_Clear();
		
	}
	return -1;	
}
/***********
* 删除指纹 *
***********/
void Del_FR(void)
{
	u8  ensure;
	u16 num;
	OLED_Clear();
	Show_Str(0,0,128,16,"=== 删除指纹 ===",16,0);	
	Show_Str(0,16,128,12,"输入指纹ID：",12,0);
	Show_Str(0,52,128,12,"返回 清空    确认删除",12,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	num=GET_NUM();  //用户输入 
	if(num==0xFFFF)      //按下s16返回按键
		goto MENU ; //返回主页面
	else if(num==0xFF00)   //按下s8清除按键
		ensure=PS_Empty(); //清空指纹库
	else  
		ensure=PS_DeletChar(num,1);//删除单个指纹
  if(ensure==0)
	{
		OLED_Clear();
		Show_Str(0,20,128,12,"删除指纹成功！",12,0);		
		Show_Str(80,48,128,12,"剩余",12,0);		
        OLED_Refresh_Gram();//更新显示
	}
  else //删除失败
    ShowErrMessage(ensure);	
	OLED_Refresh_Gram();//更新显示
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
	OLED_ShowNum(110,48,AS608Para.PS_max-ValidN,3,12); //显示剩余可录取个数
	delay_ms(1200);
	
MENU:	
	OLED_Clear();
}

/***********
* 修改密码 *
***********/
void SetPassworld(void)
{
	int pwd_ch=0;
	int  key_num=0,i=0,satus=0;
	u16 time4=0;
	u8 pwd[6]="      ";
	u8 hidepwd[6]="      "; //隐藏密码
	u8 buf[10];
	OLED_Clear();//清屏
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Chg  OK",12,0);
	
	
	Show_Str(5,0,128,16,"新密码",16,0);
	sprintf((char*)buf,"%d:",pwd_ch+1);
	Show_Str(5,48,128,16,buf,16,0);
	OLED_Refresh_Gram();//更新显示
	while(1)
	{
		key_num=Button4_4_Scan();	
		if(key_num != -1)
		{	
			DisFlag = 1;
			time4=0;
			switch(key_num)
			{
				case 1:
				case 2:
				case 3:
					pwd[i]=key_num+0x30; //1-3 转换为ASCII码值
					hidepwd[i]='*';
					i++;
                    break;
				case 4://返回
					OLED_Clear();
					delay_ms(500);
                    return ;
					
                    break;
				case 5:
				case 6:
				case 7:
					pwd[i]=key_num+0x30-1; //4-6 对应按键键码-1 = 显示的输入(s5-->4)
					hidepwd[i]='*';
					i++;
                    break;
				case 8:
					if( i > 0){
						pwd[--i]=' ';  //‘del’键  直接给此索引下清除
						hidepwd[i]=' '; 
					}
                    break;
				case 9:
				case 10:
				case 11:
					pwd[i]=key_num+0x30-2; //7-9  输入与键码的转换
					hidepwd[i]='*';
					i++;
                    break;
				case 12:satus=!satus; break;//DIS  在密码和隐藏密码中切换显示
				case 13:
					sprintf((char*)buf,"%d:",pwd_ch+1);
					Show_Str(5,48,128,16,buf,16,0);
					pwd_ch = !pwd_ch;
				case 15:
					while(i--){
					pwd[i]=' ';  //‘清空’键
					hidepwd[i]=' '; }
					i=0;
                    break;
				case 14:
					pwd[i]=0x30; //4-6
					hidepwd[i]='*';
					i++;
				break;
				case 16:
					goto MODIF;
				break;
			}
		}
		if(DisFlag == 1)
		if(satus==0)
		{
			OLED_ShowString(70,0,hidepwd,12);
			OLED_Refresh_Gram();//更新显示
		}
		else 
		{
			OLED_ShowString(70,0,pwd,12);
			OLED_Refresh_Gram();//更新显示
		}
		
		time4++;
		if(time4%1000==0){
			OLED_Clear();//清屏
			DisFlag = 1;
			return ;
		}
	}	
	
	
MODIF:
	if(pwd_ch==0)
	{
		memcpy(sys.passwd1,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存到内部FLASH
		
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取
		//printf("pwd=%s",sys.passwd1);
	}
	else
	{		
		memcpy(sys.passwd2,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存密码到内部FLASH
//		STMFLASH_Write(0X08090004,(u32*)pwd,2);//保存密码到内部eeprom
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取密码2
		//printf("pwd2=%s",sys.passwd1);
	}
	OLED_Clear();//清屏
	Show_Str(0,48,128,16,"密码修改成功 ！",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
}

/***********
* 设置时间 *
***********/
void Set_Time(void)
{
	u16 year;
	u8 mon,dat,wek,hour,min,sec;
	u16 time5=0;
	u8 tbuf[40];
	int key_num;
	int st=0;
	
//年月日等信息   
	year=calendar.w_year; 
	mon=calendar.w_month;
	dat=calendar.w_date;
	wek=calendar.week;
	hour=calendar.hour;
	min=calendar.min;
	sec=calendar.sec;
	OLED_Clear();
	Show_Str(98,38,128,12,"<--",12,0);
	Show_Str(0,52,128,12,"减  加   切换  确定",12,0);
	
	OLED_Refresh_Gram();//更新显示
	while(1)
	{
		time5++;
		key_num = Button4_4_Scan();	
			if(key_num==12 | time5==3000){ //返回页面
				OLED_Clear();//清屏
				return ;
			}
			if(key_num==13){ //减一
				switch(st)
				{
					case 0:if(hour>0)hour--;break;
					case 1:if(min>0)min--;break;
					case 2:if(sec>0)sec--;break;
					case 3:if(wek>0)wek--;break;
					case 4:if(year>0)year--;break;
					case 5:if(mon>0)mon--;break;
					case 6:if(dat>0)dat--;break;
				}
			}
			if(key_num==14){ //加一
				switch(st)
				{
					case 0:if(hour<23)hour++;break;
					case 1:if(min<59)min++;break;
					case 2:if(sec<59)sec++;break;
					case 3:if(wek<7)wek++;break;
					case 4:if(year<2099)year++;break;
					case 5:if(mon<12)mon++;break;
					case 6:if(dat<31)dat++;break;
				}
			}
			if(key_num==15){  //切换修改
				if(st<7)st++;
				if(st==7)st=0;
			}
			if(key_num==16){
				break;
			}
		if(time5%250==0) // 0.25ms关闭
		{
			switch(st)			//闪烁 
				{
					case 0:OLED_ShowString(0,0,"  ",24);break;
					case 1:OLED_ShowString(36,0,"  ",24);break;
					case 2:OLED_ShowString(72,0,"  ",24);break;
					case 3:OLED_ShowString(110,12,"   ",12);break;
					case 4:OLED_ShowString(68,26,"    ",12);break;
					case 5:OLED_ShowString(98,26,"  ",12);break;
					case 6:OLED_ShowString(116,26,"  ",12);break;
				}
			OLED_Refresh_Gram();//更新显示
		}
		if(time5%500==0) //0.5ms显示
		{
			time5=0;
			sprintf((char*)tbuf,"%02d:%02d:%02d",hour,min,sec); //格式化为字符串
			OLED_ShowString(0,0,tbuf,24);	
			sprintf((char*)tbuf,"%04d-%02d-%02d",year,mon,dat); 
			OLED_ShowString(68,26,tbuf,12);		
			sprintf((char*)tbuf,"%s",week[wek]); 
			OLED_ShowString(110,12,tbuf,12);	
			OLED_Refresh_Gram();//更新显示
		}delay_ms(1);
	}
	RTC_Set(year,mon,dat,hour,min,sec); //把修改的数值传进去
	OLED_Clear();
	Show_Str(20,48,128,16,"设置成功！",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(1000);
}

/***********
* 录入新卡 *
***********/
u8 Add_Rfid(void)
{
	u8 ID;
	u16 time6=0;
	u8 i,key_num,status=1,card_size;
	OLED_Clear();
	Show_Str(0,0,128,16,"^_^ 录入卡片 ^_^",16,0);	
	Show_Str(0,20,128,12,"请放入新卡片：",12,0);	
	Show_Str(0,52,128,12,"返回",12,0);
    OLED_Refresh_Gram();        //更新显示
	MFRC522_Initializtion();    //初始化MFRC522
	while(1)
	{
		AntennaOn(); //开启天线
		status = MFRC522_Request(0x52, card_pydebuf);			//寻卡 
		if(status == 0)		//寻卡成功
		{
			printf("rc522 ok\r\n");
			Show_Str(0,38,128,12,"读卡成功！",12,0);
			OLED_Refresh_Gram();//更新显示
			status=MFRC522_Anticoll(card_numberbuf);    //防撞处理	 因为可能有很多卡片		
			card_size=MFRC522_SelectTag(card_numberbuf);	//选卡
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
			status=MFRC522_Write(4, card_writebuf);     //写卡（写卡要小心，特别是各区的块3）
			status=MFRC522_Read(4, card_readbuf);       //读卡
			//printf("卡的类型：%#x %#x",card_pydebuf[0],card_pydebuf[1]);
			//卡序列号显，最后一字节为卡的校验码
			printf("卡的序列号：");
			for(i=0;i<5;i++)
			{
				printf("%#x ",card_numberbuf[i]);  //卡片物理id
			}
			printf("\r\n");
			//卡容量显示，单位为Kbits
			//printf("卡的容量：%dKbits\n",card_size);
			AntennaOff();  //关闭天线
			
			OLED_Clear_NOupdate();
			Show_Str(0,12,128,12,"请输入储存ID(0-9):  ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"删除 清空      确认",12,0);
			OLED_Refresh_Gram();//更新显示
				do
					ID=GET_NUM(); //读取用户输入
				while(!(ID<10));  //输入ID必须小于最大容量
			printf("正在录入卡片：%d\r\n",ID);
			OLED_Clear_NOupdate(); //清屏
			Show_Str(0,38,128,12,"正在录入.",12,0);
			OLED_Refresh_Gram();//更新显示
			for(i=0;i<5;i++)
			{
				sys.cardid[ID][i] = card_numberbuf[i]; //将物理id存入flash
			}
				
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//保存到内部FLASH
			for(i=0;i<10;i++)
			printf("cardid={%X,%X,%X,%X}\r\n",sys.cardid[i][0],sys.cardid[i][1],sys.cardid[i][2],sys.cardid[i][3]);
			Show_Str(0,38,128,12,"录入成功！",12,0);
			OLED_Refresh_Gram();//更新显示
			delay_ms(1000);
			OLED_Clear();
			return 0;
		}
		key_num = Button4_4_Scan();	
		time6++;
		if(time6%5000==0 | key_num==13) //如果超时或者点击返回
		{
			OLED_Clear(); //清屏
			return 1;
		}
	}
}

/***********
* rfid卡锁 *
***********/
u8 MFRC522_lock(void)  //读卡
{
	u8 i,j,status=1,card_size;
	u8 count;
	u8 prtfbuf[64];
	
	AntennaOn(); //打开天线
    status=MFRC522_Request(0x52, card_pydebuf);			//寻卡
	if(status==0)		//如果读到卡
	{
		if(DisErrCnt())return -1;//错误次数超限
		printf("rc522 ok\r\n");
		status = MFRC522_Anticoll(card_numberbuf);			//防冲突检测			读取卡序列
		card_size = MFRC522_SelectTag(card_numberbuf);	//选卡 card_numberbuf传出来是需要的物理卡号
		status = MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//验卡
		status = MFRC522_Write(4, card_writebuf);				//写卡（写卡要小心，特别是各区的块3）
		status = MFRC522_Read(4, card_readbuf);					//读卡
		//MFRC522_Halt();															//使卡进入休眠状态
		//卡类型显示
		
		//printf("卡的类型：%#x %#x",card_pydebuf[0],card_pydebuf[1]);
		
		//卡序列号显，最后一字节为卡的校验码
		count=0;
		
		for(j=0;j<10;j++){
		printf("\r\n卡%d 的序列号：",j);
			for(i=0;i<5;i++)
			{
				printf("%x=%x    ",card_numberbuf[i],sys.cardid[j][i]);
				if(card_numberbuf[i]==sys.cardid[j][i])count++;  //读取和flash存储卡id相同
			}
		printf("\r\n");
			if(count >= 4) //IC卡匹配成功
			{
				sys.errCnt = 0; //错误次数清除
				OLED_Clear_NOupdate();  
				sprintf(prtfbuf,"RFID:%d匹配成功",j);
				Show_Str(12,13,128,20,prtfbuf,12,0);  //IC卡匹配成功显示
				OLED_Refresh_Gram();//更新显示
				delay_ms(500);
				DisUnLock();
				return 0;
			}else count=0;
		}
		{
			sys.errCnt++;//错误次数计数
			
			if(sys.errCnt>MAXERRTIMES) //错误次数过多
				sys.errTime = 30; //30秒不能再解锁
			OLED_Clear(); 
			Show_Str(12,13,128,20,"卡片错误",12,0); 
			OLED_Refresh_Gram();//更新显示
			beep_on_mode1();
			OLED_Clear(); 
			OLED_Show_Font(56,48,0);//锁
			DisFlag = 1;
		}
		
		printf("\n");
	}	
	
	AntennaOff();
	return 1;
}

//显示信息
void Massige(void)
{
	OLED_Clear();
	Show_Str(0,0,128,12,"门锁智能系统",12,0); 
	Show_Str(0,24,128,12,"追光的人,终会光芒万丈",12,0); 
	Show_Str(0,48,128,12,"生命不止  奋斗不息",12,0); 
    
	OLED_Refresh_Gram();//更新显示
	delay_ms(3000);
}

 
 //显示时间
void Display_Data(void)
{
	static u8 t=1;	
	u8 tbuf[40]; //显示时间缓冲区
	if(t!=calendar.sec) //如果不等于rtc时钟
	{
		t=calendar.sec; //更新秒
		sprintf((char*)tbuf,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);  //时分秒  格式化为字符串 
		OLED_ShowString(0,0,tbuf,24);	
		sprintf((char*)tbuf,"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);  //年月日
		OLED_ShowString(68,26,tbuf,12);		
		sprintf((char*)tbuf,"%s",week[calendar.week]); 
		OLED_ShowString(110,12,tbuf,12);
		DisFlag = 1;//更新显示
	}
}
 

 //开机信息
void starting(void)
{
	u8 cnt = 0;
	u8 ensure;
	char str[64]; 			  
	Show_Str(16,12,128,16,"智能门锁系统",16,0);
//	Show_Str(16,12,128,16,"愚昧门锁系统",16,0);
	OLED_Refresh_Gram();//更新显示
	delay_ms(2000); //显示延时2s
    
/*********************************开机信息提示***********************************/
#if USE_FINGERPRINT
	OLED_Clear();
	Show_Str(0,0,128,12,"fingerprint system!",12,0); 
	Show_Str(0,12,128,12,"connect to as608",12,0);
	OLED_Refresh_Gram();//更新显示
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		cnt++;if(cnt>10)break;
		delay_ms(400);
		Show_Str(0,24,128,12,"connect failed! ",12,0);
	OLED_Refresh_Gram();//更新显示
		delay_ms(800);
		Show_Str(0,24,128,12,"connect to as608  ",12,0);	
		printf("connect to as608..\r\n");
	OLED_Refresh_Gram();//更新显示
	}
	if(cnt>10)Show_Str(0,24,128,12,"connect failed!",12,0);	
	
	OLED_Refresh_Gram();//更新显示
	sprintf(str,"baud:%d  addr:%x",usart2_baund,AS608Addr);
	Show_Str(0,36,128,12,(u8*)str,12,0);
	OLED_Refresh_Gram();//更新显示
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		printf("ERR:010\r\n");
		//ShowErrMessage(ensure);//显示确认码错误信息	
	ensure=PS_ReadSysPara(&AS608Para);  //读参数 
//	if(ensure==0x00)
//	{
		sprintf(str,"capacity:%d  Lv: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		Show_Str(0,48,128,12,(u8*)str,12,0);
		OLED_Refresh_Gram();//更新显示
//	}
//	else
//		ShowErrMessage(ensure);	//显示确认码错误信息	
	
//	delay_ms(1000);
//
#endif
	OLED_Clear();
}

 


void SysPartInit(void )   //系统参数初始化 
{
		STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //读取Flash
		if(sys.HZCFlag != 980706)
		{
			memset(&sys,0,sizeof(sys));
			sys.HZCFlag = 980706;
			strcpy((char *)sys.passwd1, "123456");//按键解锁密码  将”密码“复制到password1内存空间中
			strcpy((char *)sys.passwd2, "980706");//蓝牙解锁密码
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));  //写入FLASH
			printf("初始化配置成功\r\n");}
        else {
        printf("欢迎使用智能门锁\r\n");  }
}
 


 
