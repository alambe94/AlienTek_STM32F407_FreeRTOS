#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "timer.h"
#include "lcd.h"
#include "FreeRTOS.h"
#include "task.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��4-2
 FreeRTOS���񴴽���ɾ��(��̬����)-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������������ջ
static StackType_t IdleTaskStack[configMINIMAL_STACK_SIZE];
//����������ƿ�
static StaticTask_t IdleTaskTCB;

//��ʱ�����������ջ
static StackType_t TimerTaskStack[configTIMER_TASK_STACK_DEPTH];
//��ʱ������������ƿ�
static StaticTask_t TimerTaskTCB;

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//�����ջ
StackType_t StartTaskStack[START_STK_SIZE];
//������ƿ�
StaticTask_t StartTaskTCB;
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TASK1_TASK_PRIO		2
//�����ջ��С	
#define TASK1_STK_SIZE 		128  
//�����ջ
StackType_t Task1TaskStack[TASK1_STK_SIZE];
//������ƿ�
StaticTask_t Task1TaskTCB;
//������
TaskHandle_t Task1Task_Handler;
//������
void task1_task(void *pvParameters);

//�������ȼ�
#define TASK2_TASK_PRIO		3
//�����ջ��С	
#define TASK2_STK_SIZE 		128 
//�����ջ
StackType_t Task2TaskStack[TASK2_STK_SIZE];
//������ƿ�
StaticTask_t Task2TaskTCB;
//������
TaskHandle_t Task2Task_Handler;
//������
void task2_task(void *pvParameters);

//LCDˢ��ʱʹ�õ���ɫ
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

//��ȡ��������������ջ��������ƿ��ڴ棬��Ϊ������ʹ�õ�
//��̬�ڴ棬��˿�������������ջ��������ƿ���ڴ��Ӧ��
//���û����ṩ��FreeRTOS�ṩ�˽ӿں���vApplicationGetIdleTaskMemory()
//ʵ�ִ˺������ɡ�
//ppxIdleTaskTCBBuffer:������ƿ��ڴ�
//ppxIdleTaskStackBuffer:�����ջ�ڴ�
//pulIdleTaskStackSize:�����ջ��С
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, 
								   StackType_t **ppxIdleTaskStackBuffer, 
								   uint32_t *pulIdleTaskStackSize)
{
	*ppxIdleTaskTCBBuffer=&IdleTaskTCB;
	*ppxIdleTaskStackBuffer=IdleTaskStack;
	*pulIdleTaskStackSize=configMINIMAL_STACK_SIZE;
}

//��ȡ��ʱ����������������ջ��������ƿ��ڴ�
//ppxTimerTaskTCBBuffer:������ƿ��ڴ�
//ppxTimerTaskStackBuffer:�����ջ�ڴ�
//pulTimerTaskStackSize:�����ջ��С
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, 
									StackType_t **ppxTimerTaskStackBuffer, 
									uint32_t *pulTimerTaskStackSize)
{
	*ppxTimerTaskTCBBuffer=&TimerTaskTCB;
	*ppxTimerTaskStackBuffer=TimerTaskStack;
	*pulTimerTaskStackSize=configTIMER_TASK_STACK_DEPTH;
}

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	LCD_Init();							//��ʼ��LCD
	
	vTraceEnable(TRC_START);//start tracealyzer


    POINT_COLOR = RED;
	LCD_ShowString(30,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(30,30,200,16,16,"FreeRTOS Examp 6-2");
	LCD_ShowString(30,50,200,16,16,"Task Creat and Del");
	LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
	LCD_ShowString(30,90,200,16,16,"2016/11/25");
	
    //������ʼ����
	StartTask_Handler=xTaskCreateStatic((TaskFunction_t	)start_task,		//������
										(const char* 	)"start_task",		//��������
										(uint32_t 		)START_STK_SIZE,	//�����ջ��С
										(void* 		  	)NULL,				//���ݸ��������Ĳ���
										(UBaseType_t 	)START_TASK_PRIO, 	//�������ȼ�
										(StackType_t*   )StartTaskStack,	//�����ջ
										(StaticTask_t*  )&StartTaskTCB);	//������ƿ�              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����TASK1����
	Task1Task_Handler=xTaskCreateStatic((TaskFunction_t	)task1_task,		
										(const char* 	)"task1_task",		
										(uint32_t 		)TASK1_STK_SIZE,	
										(void* 		  	)NULL,				
										(UBaseType_t 	)TASK1_TASK_PRIO, 	
										(StackType_t*   )Task1TaskStack,	
										(StaticTask_t*  )&Task1TaskTCB);	
    //����TASK2����
	Task2Task_Handler=xTaskCreateStatic((TaskFunction_t	)task2_task,		
										(const char* 	)"task2_task",		
										(uint32_t 		)TASK2_STK_SIZE,	
										(void* 		  	)NULL,				
										(UBaseType_t 	)TASK2_TASK_PRIO, 	
										(StackType_t*   )Task2TaskStack,	
										(StaticTask_t*  )&Task2TaskTCB);
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//task1������
void task1_task(void *pvParameters)
{
	u8 task1_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(5,110,115,314); 	//��һ������	
	LCD_DrawLine(5,130,115,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(6,111,110,16,16,"Task1 Run:000");
	while(1)
	{
		task1_num++;	//����ִ1�д�����1 ע��task1_num1�ӵ�255��ʱ������㣡��
		LED0=!LED0;
		printf("����1�Ѿ�ִ�У�%d��\r\n",task1_num);
		if(task1_num==5) 
		{
			if(Task2Task_Handler != NULL)		//����2�Ƿ���ڣ�	
			{
				vTaskDelete(Task2Task_Handler);	//����1ִ��5��ɾ������2
				Task2Task_Handler=NULL;			//����������
				printf("����1ɾ��������2!\r\n");
			}
		}
		LCD_Fill(6,131,114,313,lcd_discolor[task1_num%14]); //�������
		LCD_ShowxNum(86,111,task1_num,3,16,0x80);	//��ʾ����ִ�д���
        vTaskDelay(1000);                           //��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}

//task2������
void task2_task(void *pvParameters)
{
	u8 task2_num=0;
	
	POINT_COLOR = BLACK;

	LCD_DrawRectangle(125,110,234,314); //��һ������	
	LCD_DrawLine(125,130,234,130);		//����
	POINT_COLOR = BLUE;
	LCD_ShowString(126,111,110,16,16,"Task2 Run:000");
	while(1)
	{
		task2_num++;	//����2ִ�д�����1 ע��task1_num2�ӵ�255��ʱ������㣡��
        LED1=!LED1;
		printf("����2�Ѿ�ִ�У�%d��\r\n",task2_num);
		LCD_ShowxNum(206,111,task2_num,3,16,0x80);  //��ʾ����ִ�д���
		LCD_Fill(126,131,233,313,lcd_discolor[13-task2_num%14]); //�������
        vTaskDelay(1000);                           //��ʱ1s��Ҳ����1000��ʱ�ӽ���	
	}
}


