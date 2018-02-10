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
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��20-1
 FreeRTOS�ڴ����ʵ��-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define MALLOC_TASK_PRIO	2
//�����ջ��С	
#define MALLOC_STK_SIZE 	128
//������
TaskHandle_t MallocTask_Handler;
//������
void malloc_task(void *p_arg);

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	LCD_Init();							//��ʼ��LCD
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
	
	POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(30,30,200,16,16,"FreeRTOS Examp 20-1");
	LCD_ShowString(30,50,200,16,16,"Mem Manage");
	LCD_ShowString(30,70,200,16,16,"KEY_UP:Malloc,KEY1:Free");
	LCD_ShowString(30,90,200,16,16,"KEY0:Use Mem");
	LCD_ShowString(30,110,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,130,200,16,16,"2016/11/14");
	
	LCD_ShowString(30,170,200,16,16,"Total Mem:      Bytes");
	LCD_ShowString(30,190,200,16,16,"Free  Mem:      Bytes");
	LCD_ShowString(30,210,200,16,16,"Message:    ");
	POINT_COLOR = BLUE;
	
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
    //����TASK1����
    xTaskCreate((TaskFunction_t )malloc_task,             
                (const char*    )"malloc_task",           
                (uint16_t       )MALLOC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MALLOC_TASK_PRIO,        
                (TaskHandle_t*  )&MallocTask_Handler);   
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}


//MALLOC������ 
void malloc_task(void *pvParameters)
{
	char *buffer;
	u8 times,i,key=0;
	u32 freemem;

	LCD_ShowxNum(110,170,configTOTAL_HEAP_SIZE,5,16,0);//��ʾ�ڴ�������	
    while(1)
    {
		key=KEY_Scan(0);
		switch(key)
		{
			case WKUP_PRES:				
				buffer=pvPortMalloc(30);			//�����ڴ棬30���ֽ�
				printf("���뵽���ڴ��ַΪ:%#x\r\n",(int)buffer);
				break;
			case KEY1_PRES:				
				if(buffer!=NULL)vPortFree(buffer);	//�ͷ��ڴ�
				buffer=NULL;
				break;
			case KEY0_PRES:
				if(buffer!=NULL)					//buffer����,ʹ��buffer
				{
					times++;
					sprintf((char*)buffer,"User %d Times",times);//��buffer����дһЩ����
					LCD_ShowString(94,210,200,16,16,buffer);
				}
				break;
		}
		freemem=xPortGetFreeHeapSize();		//��ȡʣ���ڴ��С
		LCD_ShowxNum(110,190,freemem,5,16,0);//��ʾ�ڴ�������	
		i++;
		if(i==50)
		{
			i=0;
			LED0=~LED0;
		}
        vTaskDelay(10);
    }
} 

