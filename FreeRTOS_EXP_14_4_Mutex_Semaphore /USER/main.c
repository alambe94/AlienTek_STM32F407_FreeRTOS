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
#include "semphr.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��14-4
 FreeRTOS�����ź�������ʵ��-�⺯���汾
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
#define LOW_TASK_PRIO			2
//�����ջ��С	
#define LOW_STK_SIZE 			256  
//������
TaskHandle_t LowTask_Handler;
//������
void low_task(void *pvParameters);

//�������ȼ�
#define MIDDLE_TASK_PRIO 		3
//�����ջ��С	
#define MIDDLE_STK_SIZE  		256 
//������
TaskHandle_t MiddleTask_Handler;
//������
void middle_task(void *pvParameters);

//�������ȼ�
#define HIGH_TASK_PRIO 			4
//�����ջ��С	
#define HIGH_STK_SIZE  			256 
//������
TaskHandle_t HighTask_Handler;
//������
void high_task(void *pvParameters);

//�����ź������
SemaphoreHandle_t MutexSemaphore;	//�����ź���

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
	LCD_ShowString(30,30,200,16,16,"FreeRTOS Examp 14-4");
	LCD_ShowString(30,50,200,16,16,"Mutex Semaphore");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2016/11/25");
	
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
	
	//���������ź���
	MutexSemaphore=xSemaphoreCreateMutex();
	
    //���������ȼ�����
    xTaskCreate((TaskFunction_t )high_task,             
                (const char*    )"high_task",           
                (uint16_t       )HIGH_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )HIGH_TASK_PRIO,        
                (TaskHandle_t*  )&HighTask_Handler);   
    //�����е����ȼ�����
    xTaskCreate((TaskFunction_t )middle_task,     
                (const char*    )"middle_task",   
                (uint16_t       )MIDDLE_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )MIDDLE_TASK_PRIO,
                (TaskHandle_t*  )&MiddleTask_Handler); 
	//���������ȼ�����
    xTaskCreate((TaskFunction_t )low_task,     
                (const char*    )"low_task",   
                (uint16_t       )LOW_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LOW_TASK_PRIO,
                (TaskHandle_t*  )&LowTask_Handler);
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//�����ȼ������������
void high_task(void *pvParameters)
{
	u8 num;
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,110,115,314); 	//��һ������	
	LCD_DrawLine(5,130,115,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,"High Task");
	
	while(1)
	{
		vTaskDelay(500);	//��ʱ500ms��Ҳ����500��ʱ�ӽ���	
		num++;
		printf("high task Pend Sem\r\n");
		xSemaphoreTake(MutexSemaphore,portMAX_DELAY);	//��ȡ�����ź���
		printf("high task Running!\r\n");
		LCD_Fill(6,131,114,313,lcd_discolor[num%14]); 	//�������
		LED1=!LED1;
		xSemaphoreGive(MutexSemaphore);					//�ͷ��ź���
		vTaskDelay(500);	//��ʱ500ms��Ҳ����500��ʱ�ӽ���  
	}
}

//�е����ȼ������������
void middle_task(void *pvParameters)
{
	u8 num;
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(125,110,234,314); //��һ������	
	LCD_DrawLine(125,130,234,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(126,111,110,16,16,"Middle Task");
	while(1)
	{
		num++;
		printf("middle task Running!\r\n");
		LCD_Fill(126,131,233,313,lcd_discolor[13-num%14]); //�������
		LED0=!LED0;
        vTaskDelay(1000);	//��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}

//�����ȼ������������
void low_task(void *pvParameters)
{
	static u32 times;

	while(1)
	{
		xSemaphoreTake(MutexSemaphore,portMAX_DELAY);	//��ȡ�����ź���
		printf("low task Running!\r\n");
		for(times=0;times<20000000;times++)				//ģ������ȼ�����ռ�û����ź���
		{
			taskYIELD();								//�����������
		}
		xSemaphoreGive(MutexSemaphore);					//�ͷŻ����ź���
		vTaskDelay(1000);	//��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}

