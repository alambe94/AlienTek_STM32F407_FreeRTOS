#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "string.h"
#include "malloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��15-1
 FreeRTOS�����ʱ��ʵ��-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO			1
//�����ջ��С	
#define START_STK_SIZE 			256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TIMERCONTROL_TASK_PRIO	2
//�����ջ��С	
#define TIMERCONTROL_STK_SIZE 	256  
//������
TaskHandle_t TimerControlTask_Handler;
//������
void timercontrol_task(void *pvParameters);

////////////////////////////////////////////////////////
TimerHandle_t 	AutoReloadTimer_Handle;			//���ڶ�ʱ�����
TimerHandle_t	OneShotTimer_Handle;			//���ζ�ʱ�����

void AutoReloadCallback(TimerHandle_t xTimer); 	//���ڶ�ʱ���ص�����
void OneShotCallback(TimerHandle_t xTimer);		//���ζ�ʱ���ص�����

//LCDˢ��ʱʹ�õ���ɫ
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	BEEP_Init();						//��ʼ��������
	LCD_Init();							//��ʼ��LCD
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
    
    POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(30,30,200,16,16,"FreeRTOS Examp 15-1");
	LCD_ShowString(30,50,200,16,16,"KEY_UP:Start Tmr1");
	LCD_ShowString(30,70,200,16,16,"KEY0:Start Tmr2");
	LCD_ShowString(30,90,200,16,16,"KEY1:Stop Tmr1 and Tmr2");

	LCD_DrawLine(0,108,239,108);		//����
	LCD_DrawLine(119,108,119,319);		//����
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,110,115,314); 	//��һ������	
	LCD_DrawLine(5,130,115,130);		//����
	
	LCD_DrawRectangle(125,110,234,314); //��һ������	
	LCD_DrawLine(125,130,234,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,	 "AutoTim:000");
	LCD_ShowString(126,111,110,16,16,"OneTim: 000");
	
	vTraceEnable(TRC_START);//start tracealyzer

	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����������ڶ�ʱ��
    AutoReloadTimer_Handle=xTimerCreate((const char*		)"AutoReloadTimer",
									    (TickType_t			)1000,
							            (UBaseType_t		)pdTRUE,
							            (void*				)1,
							            (TimerCallbackFunction_t)AutoReloadCallback); //���ڶ�ʱ��������1s(1000��ʱ�ӽ���)������ģʽ
    //�������ζ�ʱ��
	OneShotTimer_Handle=xTimerCreate((const char*			)"OneShotTimer",
							         (TickType_t			)2000,
							         (UBaseType_t			)pdFALSE,
							         (void*					)2,
							         (TimerCallbackFunction_t)OneShotCallback); //���ζ�ʱ��������2s(2000��ʱ�ӽ���)������ģʽ					  
    //������ʱ����������
    xTaskCreate((TaskFunction_t )timercontrol_task,             
                (const char*    )"timercontrol_task",           
                (uint16_t       )TIMERCONTROL_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TIMERCONTROL_TASK_PRIO,        
                (TaskHandle_t*  )&TimerControlTask_Handler);    
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//TimerControl��������
void timercontrol_task(void *pvParameters)
{
	u8 key,num;
	while(1)
	{
		//ֻ��������ʱ���������ɹ��˲��ܶ�����в���
		if((AutoReloadTimer_Handle!=NULL)&&(OneShotTimer_Handle!=NULL))
		{
			key = KEY_Scan(0);
			switch(key)
			{
				case WKUP_PRES:     //��key_up���µĻ������ڶ�ʱ��
					xTimerStart(AutoReloadTimer_Handle,0);	//�������ڶ�ʱ��
					printf("������ʱ��1\r\n");
					break;
				case KEY0_PRES:		//��key0���µĻ��򿪵��ζ�ʱ��
					xTimerStart(OneShotTimer_Handle,0);		//�������ζ�ʱ��
					printf("������ʱ��2\r\n");
					break;
				case KEY1_PRES:		//��key1���»��͹رն�ʱ��
					xTimerStop(AutoReloadTimer_Handle,0); 	//�ر����ڶ�ʱ��
					xTimerStop(OneShotTimer_Handle,0); 		//�رյ��ζ�ʱ��
					printf("�رն�ʱ��1��2\r\n");
					break;	
			}
		}
		num++;
		if(num==50) 	//ÿ500msLED0��˸һ��
		{
			num=0;
			LED0=!LED0;	
		}
        vTaskDelay(10); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
	}
}

//���ڶ�ʱ���Ļص�����
void AutoReloadCallback(TimerHandle_t xTimer)
{
	static u8 tmr1_num=0;
	tmr1_num++;									//���ڶ�ʱ��ִ�д�����1
	LCD_ShowxNum(70,111,tmr1_num,3,16,0x80); 	//��ʾ���ڶ�ʱ����ִ�д���
	LCD_Fill(6,131,114,313,lcd_discolor[tmr1_num%14]);//�������
}

//���ζ�ʱ���Ļص�����
void OneShotCallback(TimerHandle_t xTimer)
{
	static u8 tmr2_num = 0;
	tmr2_num++;		//���ڶ�ʱ��ִ�д�����1
	LCD_ShowxNum(190,111,tmr2_num,3,16,0x80);  //��ʾ���ζ�ʱ��ִ�д���
	LCD_Fill(126,131,233,313,lcd_discolor[tmr2_num%14]); //�������
	LED1=!LED1;
    printf("��ʱ��2���н���\r\n");
}

