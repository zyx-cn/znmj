#include "main.h"
#include <string.h>
#include "task.h"
#include "Display.h"


SysTemPat sys;

#define USE_FINGERPRINT  1 //ʹ��ָ�Ʊ�־λ
#define MAXERRTIMES 5  //�������������
#define usart2_baund  57600   //����2�����ʣ�����ָ��ģ�鲨���ʸ���

//Ҫд�뵽STM32 FLASH���ַ�������
const u8 TEXT_Buffer[]={0x17,0x23,0x6f,0x60,0,0};
#define TEXT_LENTH sizeof(TEXT_Buffer)	 		  	//���鳤��	
#define SIZE TEXT_LENTH/4+((TEXT_LENTH%4)?1:0)
#define FLASH_SAVE_ADDR  0X0802C124 	//����FLASH �����ַ(����Ϊż��������������,Ҫ���ڱ�������ռ�õ�������.
										//����,д������ʱ��,���ܻᵼ�²�����������,�Ӷ����𲿷ֳ���ʧ.��������.

SysPara AS608Para;//ָ��ģ��                                                                                                                                                                �AS608����                     
u16 ValidN;//ģ������Чָ�Ƹ���
u8** kbd_tbl;

void Display_Data(void);//��ʾʱ��
void Add_FR(void);	//¼ָ��
void Del_FR(void);	//ɾ��ָ��
int press_FR(void);//ˢָ��
void ShowErrMessage(u8 ensure);//��ʾȷ���������Ϣ
int password(void);//������
void SetPassworld(void);//�޸�����
void starting(void);//����������Ϣ
u8 MFRC522_lock(void);//ˢ������
u8 Add_Rfid(void);		//¼��
void Set_Time(void); //����ʱ��
void Massige(void);  //��ʾ�ı�
void SysPartInit(void );   //ϵͳ������ʼ�� 
//u8 Pwd[7]="      ";  //��������1
//u8 Pwd2[7]="      ";  //��������2
//u8 cardid[6]={0,0,0,0,0,0};  //����1
int Error;  //������֤��Ϣ


u8 DisFlag = 1;

//���ֵ�ASCII��
uc8 numberascii[]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
//��ʾ������
u8  dispnumber5buf[6];
u8  dispnumber3buf[4];
u8  dispnumber2buf[3];
//MFRC522������
u8  mfrc552pidbuf[18];
u8  card_pydebuf[2];
u8  card_numberbuf[5]; //��Ƭ����id
u8  card_key0Abuf[6]={0xff,0xff,0xff,0xff,0xff,0xff};
u8  card_writebuf[16]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
u8  card_readbuf[18];
//SM05-S������
u8  sm05cmdbuf[15]={14,128,0,22,5,0,0,0,4,1,157,16,0,0,21};
//extern�������������ⲿ��C�ļ��ﶨ�壬���������ļ���ʹ��
extern u8  sm05receivebuf[16];	//���ж�C�ļ��ﶨ��
extern u8  sm05_OK;							//���ж�C�ļ��ﶨ��


//����
u8 * week[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};


#if USE_FINGERPRINT //ʹ��ָ��
#define MaxParaNum 6 
u8 * setup[7]={"1��¼��ָ��","2��ɾ��ָ��","3���޸�����","4���޸�ʱ��","5��¼�뿨Ƭ","6���鿴��Ϣ"};
#else
#define MaxParaNum 4
u8 * setup[7]={"1���޸�����","2���޸�ʱ��","3��¼�뿨Ƭ","4���鿴��Ϣ","           ","           "};
#endif



int main(void)
{			
	u16 set=0;
	 u8 err=0;
	int key_num;
	int time1;
	int time2;		//����ʱ��
	char arrow=0;  //��ͷλ��

    

/*********************
***  ��ģ���ʼ��  ***
*********************/
	delay_init();	    	    //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(9600);	        //���ڳ�ʼ��Ϊ9600
	printf("���ڹ�������\r\n");
	Button4_4_Init();           //������ʼ��
	OLED_Init();                //OLED��ʼ��
	Walkmotor_Init();           //���������ʼ��
	BEEP_Init();			    //��������ʼ��
	usart2_init(usart2_baund);  //����2��ʼ��
	PS_StaGPIO_Init();          //ָ�Ƴ�ʼ��
	OLED_Clear();  //OLED����
	 
	starting();//������Ϣ  logo
	err = RTC_Init();   //RTCʱ�ӳ�ʼ��
    if(err) { 	//��ʼ��ʱ��ʧ��,����������
        OLED_Clear(); 
        Show_Str(12,13,128,20,"RTC CRY ERR!",12,0); //��ʼ��ʧ�� 
        OLED_Refresh_Gram();  delay_ms(3000);  }
	SysPartInit();   //ϵͳ������ʼ�� 
 	
    
    while(1)
	{
//��������
MAIN:
     OLED_Clear(); 
     OLED_Show_Font(56,48,0);//��ʾ��ͼ��
     while(1)
    {
        time1++;Display_Data();//ʱ����ʾ��ÿ1000ms����һ����ʾ����
				
        if(DisFlag == 1)
        {
            DisFlag = 0;
            OLED_Fill(0,24,16,63,0); //�����Ļ
            OLED_Refresh_Gram();//������ʾ
        }
				
        if((time1%100)==1) //ÿ��100ms���½���
        {
            /***********
            * ˢ������ *
            ***********/
            time1=0;
            MFRC522_Initializtion();  //IC����ʼ��	
            Error = MFRC522_lock();  //��IC�� ����
            if(Error == 0)//�����ɹ�
            {    goto MENU; } // ǰ����ҳ��
            else 
            {    OLED_Show_Font(56,48,0);   } //��
                
            /****************
            * ������������1 *
            ****************/
            Error = usart1_cherk((char*)sys.passwd1);   //��������������������õ������Ƿ�ƥ��   
            if(Error == 0) {   //�����ɹ�
                OLED_Clear_NOupdate(); //����
                Show_Str(12,13,128,20,"��������1����ȷ",12,0); 
                OLED_Refresh_Gram();//������ʾ
                delay_ms(800);
                DisUnLock(); //����
                goto MENU;	}
            else  {
//              OLED_Clear_NOupdate();
//              Show_Str(12,13,128,12,"�������룺����",12,0); 
//              OLED_Refresh_Gram();//������ʾ
//              delay_ms(800);
//              OLED_Show_Font(56,48,0);  //��
            }
            /****************
            * ������������2 *
            ****************/
            Error=usart1_cherk((char*)sys.passwd2);         
            if(Error==0){
                sys.errCnt = 0;
                OLED_Clear_NOupdate();
                Show_Str(12,13,128,12,"��������2����ȷ",12,0); 
                OLED_Refresh_Gram();//������ʾ
                delay_ms(800);
                DisUnLock();
                goto MENU;	
               }
            else 
                {
                //OLED_Show_Font(56,48,0);//��
                }
						
			}
                          
            /***********
            * ָ�ƽ��� *
            ***********/
            if(PS_Sta)	 //���PS_Sta״̬���������ָ����
            {
                while(PS_Sta){
                  Error=press_FR();//ˢָ��
                  if(Error==0) {
                       DisUnLock();
                     goto MENU; }  //������������
                  else  {
                     OLED_Show_Font(56,48,0); }  //��
				}
			}
                
            /***********
            * ������� *
            ***********/
            key_num=Button4_4_Scan();	//����ɨ��
            if(key_num!=-1) //���������1-9֮����Ч����
				{
					Error=password();//�����������
					if(Error==0)
					{
						goto MENU;	//������������
					}
					else 
					{
						OLED_Show_Font(56,48,0);//��
					}
				}
				delay_ms(1);									
			}
    
            
			/**************************
                     ������
            **************************/

MENU:
			OLED_Clear();
MENUNOCLR:   //��ҳ����ѡ��
			OLED_Fill(0,0,20,48,0); 
			//��ҳ�˵���ʾ  
			if(arrow<3){   //�÷������ڷ�ҳ��ʾ�˵����ѡ���һ������ʱ��ͨ��������ʾλ������Ӧ��Ļ�ռ䡣
				Show_Str(5,arrow*16,128,16,"->",16,0);//��ʾ��ͷ
				set=0;} 
			else {
				Show_Str(5,(arrow-3)*16,128,16,"->",16,0);
				set=3;}
            
			Show_Str(25,0,128,16,setup[set],16,0);      //����1 -->  ¼��ָ��
			Show_Str(25,16,128,16,setup[set+1],16,0);   //����2 -->  ɾ��ָ��
			Show_Str(25,32,128,16,setup[set+2],16,0);   //����3 -->  �޸�����
			Show_Str(0,52,128,12,"��    ��     ȷ��",12,0); //������ʾ
			OLED_Refresh_Gram();//������ʾ
			time2=0;
			while(1)
			{
                        /***********
                        * ��ʱ���� *
                        ***********/			
                        time2++;
						if(time2>10000 | key_num==4){  
							OLED_Clear();
								DisLock(); //����
								if(time2>10000) beep_on_mode2(); //����������
								time2 = 0; //�����������ʱ���
								//delay_ms(1000);
								OLED_Clear();
								goto MAIN;
						}
                        /***********
                        * �������� *
                        ***********/
						if(memcmp(USART_RX_BUF,"LOCK",4)==0)	{
//							USART_RX_STA=0;
//							memset(USART_RX_BUF,0,USART_REC_LEN);
							DisLock();
							goto MAIN;
						}
                                    
                        /***********
                        * ����ѡ�� *
                        ***********/
						key_num=Button4_4_Scan();	 //ɨ�谴��
						if(key_num)
						{
							if(key_num==13){
								if(arrow>0)arrow--;  //������������ͷ������һ��
								goto MENUNOCLR;  //
							}
							if(key_num==15){
								if(arrow<MaxParaNum-1)arrow++; //������������ͷǰ����һ��
								goto MENUNOCLR;  
							}
							if(key_num==16){
								switch(arrow)
								{
#if USE_FINGERPRINT
									case 0:Add_FR();	break;   //¼��ָ��
									case 1:Del_FR();	break;   //ɾ��ָ��
									case 2:SetPassworld();break; //�޸�����
									case 3:Set_Time(); break;    //����ʱ��
									case 4:Add_Rfid(); break;    //¼�뿨Ƭ
									case 5:Massige(); break;     //��ʾ��Ϣ
#else
									case 0:SetPassworld();break; //�޸�����
									case 1:Set_Time(); break;    //����ʱ��
									case 2:Add_Rfid(); break;    //¼�뿨Ƭ
									case 3:Massige(); break;     //��ʾ��Ϣ
#endif									
								}
								goto MENU;
							}		
						}delay_ms(1);
			}	
	}//while
			
			
			
			
		
		 
 }
 
//������ʾ
 u8 DisErrCnt(void)
 {
	 int time=0;
	 u8 buf[64];
	 if(sys.errTime>0)//�����������
	{
		OLED_Clear();
		while(1)
		{
			if(time++ == 1000) //ÿ1000msˢ��
			{
				time = 0;
				if(sys.errTime==0) 
				{
					OLED_Clear();
					break;
				}
				Show_Str(0,16,128,16,"��������������",16,0);
				sprintf(buf,"��%02d�������", sys.errTime);
				Show_Str(20,32,128,16,buf,16,0);
				OLED_Refresh_Gram();//������ʾ
			}
			delay_ms(1);
			if(4 == Button4_4_Scan())//����
			{
				OLED_Clear();
				return 1;
			}
		}
	}
 }
 

 

//������
int password(void)
{
	int  key_num=0,i=0,satus=0;
	u16 num=0,num2=0,time3=0,time;
	u8 pwd[11]="          ";
	u8 hidepwd[11]="          ";
	u8 buf[64];
	OLED_Clear();//����

	if(DisErrCnt())return -1;//�����������
	
	OLED_Clear();//����
	Show_Str(5,0,128,16,"���룺",16,0);
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Clr  OK",12,0);
	OLED_Refresh_Gram();//������ʾ
//	Show_Str(102,36,128,12,"��ʾ",12,0);
//	Show_Str(0,52,128,12,"ɾ�� ���   ���� ȷ��",12,0);
	while(1)
	{
		key_num=Button4_4_Scan();  //��ȡ��������
		if(key_num != -1)
		{	
            printf("key=[%d]\r\n",key_num); //��ʾ������
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
                            hidepwd[i]='*';  //��������
                            i++;
                            break;
					case 4://����
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
							pwd[--i]=' ';  //��del����
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
						pwd[i]=' ';  //����ա���
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
						goto UNLOCK; //ǰ��������֤
					break;
				}
		}
		if(DisFlag == 1)
		{
            if(satus==0) OLED_ShowString(53,0,hidepwd,12);  //�������������
            else         OLED_ShowString(53,0,pwd,12);      //��ʾ���������
            OLED_Refresh_Gram();//������ʾ
		}
		
		time3++;
		if(time3%1000==0){
			OLED_Clear();//����
			return -1;
		}
	}
}

UNLOCK:	
		for(i=0; i<10; i++){   //��֤��α����
			if(pwd[i]==sys.passwd1[num])num++;
				else num=0;
			if(num==6)
				break;
		}
		for(i=0; i<10; i++){   //��֤����
			if(pwd[i]==sys.passwd2[num2])num2++;
				else num2=0;
			if(num2==6)
				break;
		}
		if(num==6 | num2==6){
			DisUnLock(); //����
			OLED_Clear();//����
			sys.errCnt = 0;
			return 0;
		}
		else {
			sys.errCnt++;//�����������
			if(sys.errCnt>MAXERRTIMES) //����������������ƴ���
				sys.errTime = 30; //30�벻���ٽ���
			OLED_Clear();//����
			Show_Str(45,48,128,16,"�������",16,0);
			OLED_Refresh_Gram();//������ʾ
			beep_on_mode1();
			delay_ms(1500);
			OLED_Clear();//����
			return -1;
		}
	
}


//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
	Show_Str(0,48,128,12,(u8*)EnsureMessage(ensure),12,0);	
	OLED_Refresh_Gram();//������ʾ
	delay_ms(1000);
	OLED_ShowString(0,48,"                   ",12);	
	
	OLED_Refresh_Gram();//������ʾ
}
/***********
* ָ��¼ȡ *
***********/
void Add_FR(void)
{
	u8 i,ensure ,processnum=0;
	int key_num;
	u16 ID;
	OLED_Clear();//����
	while(1)
	{
		key_num = Button4_4_Scan();	
		if(key_num==16){ //����
			OLED_Clear();//����
			return ;
		}
		switch (processnum)
		{
			case 0: //¼��ָ��
				//OLED_Clear();//����
				i++;
				Show_Str(0,0,128,16,"=== ¼��ָ�� ===",16,0);
				Show_Str(0,24,128,12,"�밴ָ�ƣ�  ",12,0);	
				Show_Str(104,52,128,12,"����",12,0);		
				OLED_Refresh_Gram();//������ʾ	
				ensure=PS_GetImage(); //ָ��ʶ���ж�
				if(ensure==0x00)  //¼�����
				{
					BEEP=0; //��������ʾ
					ensure=PS_GenChar(CharBuffer1);//��������
					BEEP=1;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"ָ��������    ",12,0);
						OLED_Refresh_Gram();  //������ʾ	
						i=0;
						processnum=1;  //�����ڶ���						
					}else ShowErrMessage(ensure);	 //��ʾָ��¼ȡ����			
				}else ShowErrMessage(ensure); //��ʾָ��¼ȡ����
				//OLED_Clear();//����
				break;
			
			case 1: //�ٴ�¼���ָ��
				i++;
				Show_Str(0,24,128,12,"���ٰ�һ��ָ��",12,0);
				OLED_Refresh_Gram();//������ʾ		
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=0;
					ensure=PS_GenChar(CharBuffer2);//��������
					BEEP=1;
					if(ensure==0x00)
					{
						Show_Str(0,24,128,12,"ָ��������",12,0);	
						OLED_Refresh_Gram();//������ʾ
						i=0;
						processnum=2;//����������
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				//OLED_Clear();//����
				break;

			case 2:	 //�Ա�ָ��
				Show_Str(0,24,128,12,"�Ա�����ָ��        ",12,0);
				OLED_Refresh_Gram();//������ʾ
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					Show_Str(0,24,128,12,"����ָ��һ��       ",12,0);
					OLED_Refresh_Gram();//������ʾ
					processnum=3;//�������Ĳ�
				}
				else  //ָ�Ʋ�ͬ
				{
					Show_Str(0,24,128,12,"�Ա�ʧ�� ����¼    ",12,0);	
					OLED_Refresh_Gram();//������ʾ
					ShowErrMessage(ensure);
					i=0;
					OLED_Clear();//����
					processnum=0;//���ص�һ��		
				}
				delay_ms(1200);
				//OLED_Clear();//����
				break;

			case 3: //����ָ��ģ��
			Show_Str(0,24,128,12,"����ָ��ģ��...    ",12,0);
			OLED_Refresh_Gram();//������ʾ	
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					Show_Str(0,24,128,12,"����ָ��ģ��ɹ�!",12,0);
					OLED_Refresh_Gram();//������ʾ
					processnum=4;//�������岽
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	 //¼��ID
				//OLED_Clear();//����
			Show_Str(0,24,128,12,"�����봢��ID:        ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"ɾ�� ���      ȷ��",12,0);
			OLED_Refresh_Gram();//������ʾ
				do
					ID=GET_NUM(); //��ȡ�û������ID
				while(!(ID<AS608Para.PS_max));//����ID����С��ģ������������ֵ
				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
			
                if(ensure==0x00)  //�洢�ɹ�
				{			
                    OLED_Clear_NOupdate();//����
					Show_Str(0,30,128,16,"¼ָ�Ƴɹ�!",16,0);	
					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
					Show_Str(66,52,128,12,"ʣ��",12,0);
					OLED_ShowNum(90,52,AS608Para.PS_max-ValidN,3,12);
					OLED_Refresh_Gram();//������ʾ
					delay_ms(1500);
					OLED_Clear();	
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}
				OLED_Clear();//����					
				break;				
		}
		delay_ms(400);
		if(i==10)//����5��û�а���ָ���˳�
		{
			OLED_Clear();
			break;
		}				
	}
}

//ˢָ��
int press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char str[256];
	
	
	if(DisErrCnt())return -1;//�����������
	ensure=PS_GetImage();
	
	OLED_Clear_NOupdate();
	Show_Str(0,0,128,16,"���ڼ��ָ��",16,0);
	OLED_Refresh_Gram();//������ʾ
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		
			
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{				
				OLED_Clear_NOupdate();
				Show_Str(20,10,128,24,"������...",24,0);	
				OLED_Refresh_Gram();//������ʾ
				Walkmotor_ON();
				Show_Str(20,10,128,24,"�ѽ�����",24,0);
				OLED_Refresh_Gram();//������ʾ
				OLED_Show_Font(112,18,1);//����				
				//str=mymalloc(SRAMIN,2000);
				sprintf(str,"ID:%d     ƥ���",seach.pageID);
				Show_Str(0,52,128,12,(u8*)str,12,0);	
				sprintf(str,":%d",seach.mathscore);
				Show_Str(96,52,128,12,(u8*)str,12,0);	
				//myfree(SRAMIN,str);
				OLED_Refresh_Gram();//������ʾ
				delay_ms(1800);
				OLED_Clear();
				return 0;
			}
			else {
				sys.errCnt++;//�����������
				if(sys.errCnt>MAXERRTIMES)
					sys.errTime = 30; //30�벻���ٽ���
				ShowErrMessage(ensure);	
				OLED_Refresh_Gram();//������ʾ
				beep_on_mode1();
				OLED_Clear();
				return -1;
			}				
	  }
		else
			ShowErrMessage(ensure);
		
	OLED_Refresh_Gram();//������ʾ
	 delay_ms(2000);
		OLED_Clear();
		
	}
	return -1;	
}
/***********
* ɾ��ָ�� *
***********/
void Del_FR(void)
{
	u8  ensure;
	u16 num;
	OLED_Clear();
	Show_Str(0,0,128,16,"=== ɾ��ָ�� ===",16,0);	
	Show_Str(0,16,128,12,"����ָ��ID��",12,0);
	Show_Str(0,52,128,12,"���� ���    ȷ��ɾ��",12,0);
	OLED_Refresh_Gram();//������ʾ
	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	num=GET_NUM();  //�û����� 
	if(num==0xFFFF)      //����s16���ذ���
		goto MENU ; //������ҳ��
	else if(num==0xFF00)   //����s8�������
		ensure=PS_Empty(); //���ָ�ƿ�
	else  
		ensure=PS_DeletChar(num,1);//ɾ������ָ��
  if(ensure==0)
	{
		OLED_Clear();
		Show_Str(0,20,128,12,"ɾ��ָ�Ƴɹ���",12,0);		
		Show_Str(80,48,128,12,"ʣ��",12,0);		
        OLED_Refresh_Gram();//������ʾ
	}
  else //ɾ��ʧ��
    ShowErrMessage(ensure);	
	OLED_Refresh_Gram();//������ʾ
	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	OLED_ShowNum(110,48,AS608Para.PS_max-ValidN,3,12); //��ʾʣ���¼ȡ����
	delay_ms(1200);
	
MENU:	
	OLED_Clear();
}

/***********
* �޸����� *
***********/
void SetPassworld(void)
{
	int pwd_ch=0;
	int  key_num=0,i=0,satus=0;
	u16 time4=0;
	u8 pwd[6]="      ";
	u8 hidepwd[6]="      "; //��������
	u8 buf[10];
	OLED_Clear();//����
	Show_Str(10,16,128,12," 1   2   3  Bck",12,0);
	Show_Str(10,28,128,12," 4   5   6  Del",12,0);
	Show_Str(10,40,128,12," 7   8   9  Dis",12,0);
	Show_Str(10,52,128,12,"Clr  0  Chg  OK",12,0);
	
	
	Show_Str(5,0,128,16,"������",16,0);
	sprintf((char*)buf,"%d:",pwd_ch+1);
	Show_Str(5,48,128,16,buf,16,0);
	OLED_Refresh_Gram();//������ʾ
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
					pwd[i]=key_num+0x30; //1-3 ת��ΪASCII��ֵ
					hidepwd[i]='*';
					i++;
                    break;
				case 4://����
					OLED_Clear();
					delay_ms(500);
                    return ;
					
                    break;
				case 5:
				case 6:
				case 7:
					pwd[i]=key_num+0x30-1; //4-6 ��Ӧ��������-1 = ��ʾ������(s5-->4)
					hidepwd[i]='*';
					i++;
                    break;
				case 8:
					if( i > 0){
						pwd[--i]=' ';  //��del����  ֱ�Ӹ������������
						hidepwd[i]=' '; 
					}
                    break;
				case 9:
				case 10:
				case 11:
					pwd[i]=key_num+0x30-2; //7-9  ����������ת��
					hidepwd[i]='*';
					i++;
                    break;
				case 12:satus=!satus; break;//DIS  ������������������л���ʾ
				case 13:
					sprintf((char*)buf,"%d:",pwd_ch+1);
					Show_Str(5,48,128,16,buf,16,0);
					pwd_ch = !pwd_ch;
				case 15:
					while(i--){
					pwd[i]=' ';  //����ա���
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
			OLED_Refresh_Gram();//������ʾ
		}
		else 
		{
			OLED_ShowString(70,0,pwd,12);
			OLED_Refresh_Gram();//������ʾ
		}
		
		time4++;
		if(time4%1000==0){
			OLED_Clear();//����
			DisFlag = 1;
			return ;
		}
	}	
	
	
MODIF:
	if(pwd_ch==0)
	{
		memcpy(sys.passwd1,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//���浽�ڲ�FLASH
		
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //��ȡ
		//printf("pwd=%s",sys.passwd1);
	}
	else
	{		
		memcpy(sys.passwd2,pwd,7);
		STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//�������뵽�ڲ�FLASH
//		STMFLASH_Write(0X08090004,(u32*)pwd,2);//�������뵽�ڲ�eeprom
		//STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //��ȡ����2
		//printf("pwd2=%s",sys.passwd1);
	}
	OLED_Clear();//����
	Show_Str(0,48,128,16,"�����޸ĳɹ� ��",16,0);
	OLED_Refresh_Gram();//������ʾ
	delay_ms(1000);
}

/***********
* ����ʱ�� *
***********/
void Set_Time(void)
{
	u16 year;
	u8 mon,dat,wek,hour,min,sec;
	u16 time5=0;
	u8 tbuf[40];
	int key_num;
	int st=0;
	
//�����յ���Ϣ   
	year=calendar.w_year; 
	mon=calendar.w_month;
	dat=calendar.w_date;
	wek=calendar.week;
	hour=calendar.hour;
	min=calendar.min;
	sec=calendar.sec;
	OLED_Clear();
	Show_Str(98,38,128,12,"<--",12,0);
	Show_Str(0,52,128,12,"��  ��   �л�  ȷ��",12,0);
	
	OLED_Refresh_Gram();//������ʾ
	while(1)
	{
		time5++;
		key_num = Button4_4_Scan();	
			if(key_num==12 | time5==3000){ //����ҳ��
				OLED_Clear();//����
				return ;
			}
			if(key_num==13){ //��һ
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
			if(key_num==14){ //��һ
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
			if(key_num==15){  //�л��޸�
				if(st<7)st++;
				if(st==7)st=0;
			}
			if(key_num==16){
				break;
			}
		if(time5%250==0) // 0.25ms�ر�
		{
			switch(st)			//��˸ 
				{
					case 0:OLED_ShowString(0,0,"  ",24);break;
					case 1:OLED_ShowString(36,0,"  ",24);break;
					case 2:OLED_ShowString(72,0,"  ",24);break;
					case 3:OLED_ShowString(110,12,"   ",12);break;
					case 4:OLED_ShowString(68,26,"    ",12);break;
					case 5:OLED_ShowString(98,26,"  ",12);break;
					case 6:OLED_ShowString(116,26,"  ",12);break;
				}
			OLED_Refresh_Gram();//������ʾ
		}
		if(time5%500==0) //0.5ms��ʾ
		{
			time5=0;
			sprintf((char*)tbuf,"%02d:%02d:%02d",hour,min,sec); //��ʽ��Ϊ�ַ���
			OLED_ShowString(0,0,tbuf,24);	
			sprintf((char*)tbuf,"%04d-%02d-%02d",year,mon,dat); 
			OLED_ShowString(68,26,tbuf,12);		
			sprintf((char*)tbuf,"%s",week[wek]); 
			OLED_ShowString(110,12,tbuf,12);	
			OLED_Refresh_Gram();//������ʾ
		}delay_ms(1);
	}
	RTC_Set(year,mon,dat,hour,min,sec); //���޸ĵ���ֵ����ȥ
	OLED_Clear();
	Show_Str(20,48,128,16,"���óɹ���",16,0);
	OLED_Refresh_Gram();//������ʾ
	delay_ms(1000);
}

/***********
* ¼���¿� *
***********/
u8 Add_Rfid(void)
{
	u8 ID;
	u16 time6=0;
	u8 i,key_num,status=1,card_size;
	OLED_Clear();
	Show_Str(0,0,128,16,"^_^ ¼�뿨Ƭ ^_^",16,0);	
	Show_Str(0,20,128,12,"������¿�Ƭ��",12,0);	
	Show_Str(0,52,128,12,"����",12,0);
    OLED_Refresh_Gram();        //������ʾ
	MFRC522_Initializtion();    //��ʼ��MFRC522
	while(1)
	{
		AntennaOn(); //��������
		status = MFRC522_Request(0x52, card_pydebuf);			//Ѱ�� 
		if(status == 0)		//Ѱ���ɹ�
		{
			printf("rc522 ok\r\n");
			Show_Str(0,38,128,12,"�����ɹ���",12,0);
			OLED_Refresh_Gram();//������ʾ
			status=MFRC522_Anticoll(card_numberbuf);    //��ײ����	 ��Ϊ�����кܶ࿨Ƭ		
			card_size=MFRC522_SelectTag(card_numberbuf);	//ѡ��
			status=MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
			status=MFRC522_Write(4, card_writebuf);     //д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
			status=MFRC522_Read(4, card_readbuf);       //����
			//printf("�������ͣ�%#x %#x",card_pydebuf[0],card_pydebuf[1]);
			//�����к��ԣ����һ�ֽ�Ϊ����У����
			printf("�������кţ�");
			for(i=0;i<5;i++)
			{
				printf("%#x ",card_numberbuf[i]);  //��Ƭ����id
			}
			printf("\r\n");
			//��������ʾ����λΪKbits
			//printf("����������%dKbits\n",card_size);
			AntennaOff();  //�ر�����
			
			OLED_Clear_NOupdate();
			Show_Str(0,12,128,12,"�����봢��ID(0-9):  ",12,0);
			Show_Str(122,52,128,12," ",12,0);
			Show_Str(0,52,128,12,"ɾ�� ���      ȷ��",12,0);
			OLED_Refresh_Gram();//������ʾ
				do
					ID=GET_NUM(); //��ȡ�û�����
				while(!(ID<10));  //����ID����С���������
			printf("����¼�뿨Ƭ��%d\r\n",ID);
			OLED_Clear_NOupdate(); //����
			Show_Str(0,38,128,12,"����¼��.",12,0);
			OLED_Refresh_Gram();//������ʾ
			for(i=0;i<5;i++)
			{
				sys.cardid[ID][i] = card_numberbuf[i]; //������id����flash
			}
				
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));//���浽�ڲ�FLASH
			for(i=0;i<10;i++)
			printf("cardid={%X,%X,%X,%X}\r\n",sys.cardid[i][0],sys.cardid[i][1],sys.cardid[i][2],sys.cardid[i][3]);
			Show_Str(0,38,128,12,"¼��ɹ���",12,0);
			OLED_Refresh_Gram();//������ʾ
			delay_ms(1000);
			OLED_Clear();
			return 0;
		}
		key_num = Button4_4_Scan();	
		time6++;
		if(time6%5000==0 | key_num==13) //�����ʱ���ߵ������
		{
			OLED_Clear(); //����
			return 1;
		}
	}
}

/***********
* rfid���� *
***********/
u8 MFRC522_lock(void)  //����
{
	u8 i,j,status=1,card_size;
	u8 count;
	u8 prtfbuf[64];
	
	AntennaOn(); //������
    status=MFRC522_Request(0x52, card_pydebuf);			//Ѱ��
	if(status==0)		//���������
	{
		if(DisErrCnt())return -1;//�����������
		printf("rc522 ok\r\n");
		status = MFRC522_Anticoll(card_numberbuf);			//����ͻ���			��ȡ������
		card_size = MFRC522_SelectTag(card_numberbuf);	//ѡ�� card_numberbuf����������Ҫ��������
		status = MFRC522_Auth(0x60, 4, card_key0Abuf, card_numberbuf);	//�鿨
		status = MFRC522_Write(4, card_writebuf);				//д����д��ҪС�ģ��ر��Ǹ����Ŀ�3��
		status = MFRC522_Read(4, card_readbuf);					//����
		//MFRC522_Halt();															//ʹ����������״̬
		//��������ʾ
		
		//printf("�������ͣ�%#x %#x",card_pydebuf[0],card_pydebuf[1]);
		
		//�����к��ԣ����һ�ֽ�Ϊ����У����
		count=0;
		
		for(j=0;j<10;j++){
		printf("\r\n��%d �����кţ�",j);
			for(i=0;i<5;i++)
			{
				printf("%x=%x    ",card_numberbuf[i],sys.cardid[j][i]);
				if(card_numberbuf[i]==sys.cardid[j][i])count++;  //��ȡ��flash�洢��id��ͬ
			}
		printf("\r\n");
			if(count >= 4) //IC��ƥ��ɹ�
			{
				sys.errCnt = 0; //����������
				OLED_Clear_NOupdate();  
				sprintf(prtfbuf,"RFID:%dƥ��ɹ�",j);
				Show_Str(12,13,128,20,prtfbuf,12,0);  //IC��ƥ��ɹ���ʾ
				OLED_Refresh_Gram();//������ʾ
				delay_ms(500);
				DisUnLock();
				return 0;
			}else count=0;
		}
		{
			sys.errCnt++;//�����������
			
			if(sys.errCnt>MAXERRTIMES) //�����������
				sys.errTime = 30; //30�벻���ٽ���
			OLED_Clear(); 
			Show_Str(12,13,128,20,"��Ƭ����",12,0); 
			OLED_Refresh_Gram();//������ʾ
			beep_on_mode1();
			OLED_Clear(); 
			OLED_Show_Font(56,48,0);//��
			DisFlag = 1;
		}
		
		printf("\n");
	}	
	
	AntennaOff();
	return 1;
}

//��ʾ��Ϣ
void Massige(void)
{
	OLED_Clear();
	Show_Str(0,0,128,12,"��������ϵͳ",12,0); 
	Show_Str(0,24,128,12,"׷�����,�ջ��â����",12,0); 
	Show_Str(0,48,128,12,"������ֹ  �ܶ���Ϣ",12,0); 
    
	OLED_Refresh_Gram();//������ʾ
	delay_ms(3000);
}

 
 //��ʾʱ��
void Display_Data(void)
{
	static u8 t=1;	
	u8 tbuf[40]; //��ʾʱ�仺����
	if(t!=calendar.sec) //���������rtcʱ��
	{
		t=calendar.sec; //������
		sprintf((char*)tbuf,"%02d:%02d:%02d",calendar.hour,calendar.min,calendar.sec);  //ʱ����  ��ʽ��Ϊ�ַ��� 
		OLED_ShowString(0,0,tbuf,24);	
		sprintf((char*)tbuf,"%04d-%02d-%02d",calendar.w_year,calendar.w_month,calendar.w_date);  //������
		OLED_ShowString(68,26,tbuf,12);		
		sprintf((char*)tbuf,"%s",week[calendar.week]); 
		OLED_ShowString(110,12,tbuf,12);
		DisFlag = 1;//������ʾ
	}
}
 

 //������Ϣ
void starting(void)
{
	u8 cnt = 0;
	u8 ensure;
	char str[64]; 			  
	Show_Str(16,12,128,16,"��������ϵͳ",16,0);
//	Show_Str(16,12,128,16,"��������ϵͳ",16,0);
	OLED_Refresh_Gram();//������ʾ
	delay_ms(2000); //��ʾ��ʱ2s
    
/*********************************������Ϣ��ʾ***********************************/
#if USE_FINGERPRINT
	OLED_Clear();
	Show_Str(0,0,128,12,"fingerprint system!",12,0); 
	Show_Str(0,12,128,12,"connect to as608",12,0);
	OLED_Refresh_Gram();//������ʾ
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		cnt++;if(cnt>10)break;
		delay_ms(400);
		Show_Str(0,24,128,12,"connect failed! ",12,0);
	OLED_Refresh_Gram();//������ʾ
		delay_ms(800);
		Show_Str(0,24,128,12,"connect to as608  ",12,0);	
		printf("connect to as608..\r\n");
	OLED_Refresh_Gram();//������ʾ
	}
	if(cnt>10)Show_Str(0,24,128,12,"connect failed!",12,0);	
	
	OLED_Refresh_Gram();//������ʾ
	sprintf(str,"baud:%d  addr:%x",usart2_baund,AS608Addr);
	Show_Str(0,36,128,12,(u8*)str,12,0);
	OLED_Refresh_Gram();//������ʾ
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		printf("ERR:010\r\n");
		//ShowErrMessage(ensure);//��ʾȷ���������Ϣ	
	ensure=PS_ReadSysPara(&AS608Para);  //������ 
//	if(ensure==0x00)
//	{
		sprintf(str,"capacity:%d  Lv: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		Show_Str(0,48,128,12,(u8*)str,12,0);
		OLED_Refresh_Gram();//������ʾ
//	}
//	else
//		ShowErrMessage(ensure);	//��ʾȷ���������Ϣ	
	
//	delay_ms(1000);
//
#endif
	OLED_Clear();
}

 


void SysPartInit(void )   //ϵͳ������ʼ�� 
{
		STMFLASH_Read(SYS_SAVEADDR,(u16*)&sys,sizeof(sys)); //��ȡFlash
		if(sys.HZCFlag != 980706)
		{
			memset(&sys,0,sizeof(sys));
			sys.HZCFlag = 980706;
			strcpy((char *)sys.passwd1, "123456");//������������  �������롰���Ƶ�password1�ڴ�ռ���
			strcpy((char *)sys.passwd2, "980706");//������������
			STMFLASH_Write(SYS_SAVEADDR,(u16*)&sys,sizeof(sys));  //д��FLASH
			printf("��ʼ�����óɹ�\r\n");}
        else {
        printf("��ӭʹ����������\r\n");  }
}
 


 