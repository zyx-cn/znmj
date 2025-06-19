#include "walkmotor.h"
#include "delay.h"
/********************************
A  	B  	C  		 D
D3 	D5 	D7		 XCLK
PC9 PB6 PE6 	 PA8
********************************/

/* ������� */

void Walkmotor_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB,PE�˿�ʱ��

	//GPIO��ʼ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_10;				 //LED0-->PB.5 �˿�����
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_15;				 //LED0-->PB.5 �˿�����
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
	
	GPIO_ResetBits(GPIOB,GPIO_Pin_11|GPIO_Pin_10);						 //PB.5 �����
	GPIO_ResetBits(GPIOA, GPIO_Pin_12|GPIO_Pin_15);
}

/* ����������� */
void Walkmotor_ON(void)
{
#if (CTRLUSE==BJDJ)
	int i;
	for(i=0; i<256; i++)
	{
		Aout=1;Bout=0;Cout=0;Dout=0;delay_ms(3);
		Aout=0;Bout=1;Cout=0;Dout=0;delay_ms(3);
		Aout=0;Bout=0;Cout=1;Dout=0;delay_ms(3);
		Aout=0;Bout=0;Cout=0;Dout=1;delay_ms(3);
	}Aout=0;Bout=0;Cout=0;Dout=0;
#elif (CTRLUSE==JDQ)
	Cout=1;
#endif
}

/* �رղ������ */
void Walkmotor_OFF(void)
{
#if (CTRLUSE==BJDJ)
	int i;
	for(i=0; i<256; i++)
	{
		Aout=0;Bout=0;Cout=0;Dout=1;delay_ms(5);
		Aout=0;Bout=0;Cout=1;Dout=0;delay_ms(5);
		Aout=0;Bout=1;Cout=0;Dout=0;delay_ms(5);
		Aout=1;Bout=0;Cout=0;Dout=0;delay_ms(5);
	}Aout=0;Bout=0;Cout=0;Dout=0;
#elif (CTRLUSE==JDQ)
	Cout=0;
#endif
}
	

