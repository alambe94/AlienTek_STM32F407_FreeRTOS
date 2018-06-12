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
#include "queue.h"
#include "limits.h"
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��17-3
 FreeRTOS����֪ͨģ����Ϣ����-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define TASK1_TASK_PRIO		2
//�����ջ��С	
#define TASK1_STK_SIZE 		256  
//������
TaskHandle_t Task1Task_Handler;
//������
void task1_task(void *pvParameters);

//�������ȼ�
#define KEYPROCESS_TASK_PRIO 3
//�����ջ��С	
#define KEYPROCESS_STK_SIZE  256 
//������
TaskHandle_t Keyprocess_Handler;
//������
void Keyprocess_task(void *pvParameters);

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
	LCD_ShowString(10,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(10,30,200,16,16,"FreeRTOS Examp 17-3");
	LCD_ShowString(10,50,200,16,16,"Task Notify Maibox");
	LCD_ShowString(10,70,220,16,16,"KEY_UP:LED1  KEY2:BEEP");
	LCD_ShowString(10,90,200,16,16,"KEY0:Refresh LCD");
	
	POINT_COLOR = BLACK;
	LCD_DrawRectangle(5,125,234,314);	//������
	
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

    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //����������������
    xTaskCreate((TaskFunction_t )Keyprocess_task,     
                (const char*    )"keyprocess_task",   
                (uint16_t       )KEYPROCESS_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )KEYPROCESS_TASK_PRIO,
                (TaskHandle_t*  )&Keyprocess_Handler); 
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//task1������
void task1_task(void *pvParameters)
{
	u8 key,i=0;
    BaseType_t err;
	while(1)
	{
		key=KEY_Scan(0);            			//ɨ�谴��
        if((Keyprocess_Handler!=NULL)&&(key))   
        {
			err=xTaskNotify((TaskHandle_t	)Keyprocess_Handler,		//��������֪ͨ��������
							(uint32_t		)key,						//����ֵ֪ͨ
							(eNotifyAction	)eSetValueWithOverwrite);	//��д�ķ�ʽ��������֪ͨ
			if(err==pdFAIL)
			{
				printf("����֪ͨ����ʧ��\r\n");
			}
        }
        i++;
        if(i==50)
        {
            i=0;
            LED0=!LED0;
        }
        vTaskDelay(10);           //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
	}
}


//Keyprocess_task����
void Keyprocess_task(void *pvParameters)
{
	u8 num;
	uint32_t NotifyValue;
	BaseType_t err;
	
	while(1)
	{
		err=xTaskNotifyWait((uint32_t	)0x00,				//���뺯����ʱ���������bit
							(uint32_t	)ULONG_MAX,			//�˳�������ʱ��������е�bit
							(uint32_t*	)&NotifyValue,		//��������ֵ֪ͨ
							(TickType_t	)portMAX_DELAY);	//����ʱ��
		if(err==pdTRUE)				//��ȡ����֪ͨ�ɹ�
		{
			switch((u8)NotifyValue)
			{
                case WKUP_PRES:		//KEY_UP����LED1
                    LED1=!LED1;
					break;
				case KEY2_PRES:		//KEY2���Ʒ�����
                    BEEP=!BEEP;
					break;
				case KEY0_PRES:		//KEY0ˢ��LCD����
                    num++;
					LCD_Fill(6,126,233,313,lcd_discolor[num%14]);
                    break;
			}
		}
	}
}
