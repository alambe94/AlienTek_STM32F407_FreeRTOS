#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "key.h"
#include "beep.h"
#include "exti.h"
#include "string.h"
#include "malloc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��16-1
 FreeRTOS�¼���־��ʵ��-�⺯���汾
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
#define EVENTSETBIT_TASK_PRIO	2
//�����ջ��С	
#define EVENTSETBIT_STK_SIZE 	256  
//������
TaskHandle_t EventSetBit_Handler;
//������
void eventsetbit_task(void *pvParameters);

//�������ȼ�
#define EVENTGROUP_TASK_PRIO	3
//�����ջ��С	
#define EVENTGROUP_STK_SIZE 	256  
//������
TaskHandle_t EventGroupTask_Handler;
//������
void eventgroup_task(void *pvParameters);

//�������ȼ�
#define EVENTQUERY_TASK_PRIO	4
//�����ջ��С	
#define EVENTQUERY_STK_SIZE 	256  
//������
TaskHandle_t EventQueryTask_Handler;
//������
void eventquery_task(void *pvParameters);


////////////////////////////////////////////////////////
EventGroupHandle_t EventGroupHandler;	//�¼���־����

#define EVENTBIT_0	(1<<0)				//�¼�λ
#define EVENTBIT_1	(1<<1)
#define EVENTBIT_2	(1<<2)
#define EVENTBIT_ALL	(EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)

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
	EXTIX_Init();						//��ʼ���ⲿ�ж�
	LCD_Init();							//��ʼ��LCD
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
	
    POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(30,30,200,16,16,"FreeRTOS Examp 16-1");
	LCD_ShowString(30,50,200,16,16,"Event Group");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2016/11/25");

	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,130,234,314);	//������
	POINT_COLOR = BLUE;
	LCD_ShowString(30,110,220,16,16,"Event Group Value:0");
	
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
    //�����¼���־��
	EventGroupHandler=xEventGroupCreate();	 //�����¼���־��
	
	//���������¼�λ������
    xTaskCreate((TaskFunction_t )eventsetbit_task,             
                (const char*    )"eventsetbit_task",           
                (uint16_t       )EVENTSETBIT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTSETBIT_TASK_PRIO,        
                (TaskHandle_t*  )&EventSetBit_Handler);   	
    //�����¼���־�鴦������
    xTaskCreate((TaskFunction_t )eventgroup_task,             
                (const char*    )"eventgroup_task",           
                (uint16_t       )EVENTGROUP_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTGROUP_TASK_PRIO,        
                (TaskHandle_t*  )&EventGroupTask_Handler);  
	//�����¼���־���ѯ����
    xTaskCreate((TaskFunction_t )eventquery_task,             
                (const char*    )"eventquery_task",           
                (uint16_t       )EVENTQUERY_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )EVENTQUERY_TASK_PRIO,        
                (TaskHandle_t*  )&EventQueryTask_Handler);    
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


//�����¼�λ������
void eventsetbit_task(void *pvParameters)
{
	u8 key;
	while(1)
	{
		if(EventGroupHandler!=NULL)
		{
			key=KEY_Scan(0);
			switch(key)
			{
				case KEY1_PRES:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
					break;
				case KEY2_PRES:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
					break;	
			}
		}
        vTaskDelay(10); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
	}
}

//�¼���־�鴦������
void eventgroup_task(void *pvParameters)
{
	u8 num;
	EventBits_t EventValue;
	while(1)
	{

		if(EventGroupHandler!=NULL)
		{
			//�ȴ��¼����е���Ӧ�¼�λ
			EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
										   (EventBits_t			)EVENTBIT_ALL,
										   (BaseType_t			)pdTRUE,				
										   (BaseType_t			)pdTRUE,
								           (TickType_t			)portMAX_DELAY);	
			printf("�¼���־���ֵ:%d\r\n",(int)EventValue);
			LCD_ShowxNum(174,110,EventValue,1,16,0);
			
			num++;
			LED1=!LED1;	
			LCD_Fill(6,131,233,313,lcd_discolor[num%14]);
		}
		else
		{
			vTaskDelay(10); //��ʱ10ms��Ҳ����10��ʱ�ӽ���
		}
	}
}

//�¼���ѯ����
void eventquery_task(void *pvParameters)
{	
	u8 num=0;
	EventBits_t NewValue,LastValue;
	while(1)
	{
		if(EventGroupHandler!=NULL)
		{
			NewValue=xEventGroupGetBits(EventGroupHandler);	//��ȡ�¼����
			if(NewValue!=LastValue)
			{
				LastValue=NewValue;
				printf("�¼���־���ֵ:%d\r\n",(int)NewValue);
				LCD_ShowxNum(174,110,NewValue,1,16,0);
			}
		}
		num++;
		if(num==10) 	//ÿ500msLED0��˸һ��
		{
			num=0;
			LED0=!LED0;	
		}
		vTaskDelay(50); //��ʱ50ms��Ҳ����50��ʱ�ӽ���
	}
}

