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
/************************************************
 ALIENTEK ̽����STM32F407������ FreeRTOSʵ��13-1
 FreeRTOS���в���ʵ��-�⺯���汾
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


//������Ϣ���е�����
#define KEYMSG_Q_NUM    1  		//������Ϣ���е�����  
#define MESSAGE_Q_NUM   4   	//�������ݵ���Ϣ���е����� 
QueueHandle_t Key_Queue;   		//����ֵ��Ϣ���о��
QueueHandle_t Message_Queue;	//��Ϣ���о��

//LCDˢ��ʱʹ�õ���ɫ
int lcd_discolor[14]={	WHITE, BLACK, BLUE,  BRED,      
						GRED,  GBLUE, RED,   MAGENTA,       	 
						GREEN, CYAN,  YELLOW,BROWN, 			
						BRRED, GRAY };

//������LCD����ʾ���յ��Ķ��е���Ϣ
//str: Ҫ��ʾ���ַ���(���յ�����Ϣ)
void disp_str(char* str)
{
	LCD_Fill(5,230,110,245,WHITE);					//�������ʾ����
	LCD_ShowString(5,230,100,16,16,str);
}

//����������
void freertos_load_main_ui(void)
{
	POINT_COLOR = RED;
	LCD_ShowString(10,10,200,16,16,"ATK STM32F103/407");	
	LCD_ShowString(10,30,200,16,16,"FreeRTOS Examp 13-1");
	LCD_ShowString(10,50,200,16,16,"Message Queue");
	LCD_ShowString(10,70,220,16,16,"KEY_UP:LED1 KEY0:Refresh LCD");
	LCD_ShowString(10,90,200,16,16,"KEY1:SendMsg KEY2:BEEP");
	
	POINT_COLOR = BLACK;
	LCD_DrawLine(0,107,239,107);		//����
	LCD_DrawLine(119,107,119,319);		//����
	LCD_DrawRectangle(125,110,234,314);	//������
	POINT_COLOR = RED;
	LCD_ShowString(0,130,120,16,16,"DATA_Msg Size:");
	LCD_ShowString(0,170,120,16,16,"DATA_Msg rema:");
	LCD_ShowString(0,210,100,16,16,"DATA_Msg:");
	POINT_COLOR = BLUE;
}

//��ѯMessage_Queue�����е��ܶ���������ʣ���������
void check_msg_queue(void)
{
	char *p;
	u8 msgq_remain_size;	//��Ϣ����ʣ���С
    u8 msgq_total_size;     //��Ϣ�����ܴ�С
    
    taskENTER_CRITICAL();   //�����ٽ���
    msgq_remain_size=uxQueueSpacesAvailable(Message_Queue);//�õ�����ʣ���С
    msgq_total_size=uxQueueMessagesWaiting(Message_Queue)+uxQueueSpacesAvailable(Message_Queue);//�õ������ܴ�С���ܴ�С=ʹ��+ʣ��ġ�
	p=mymalloc(SRAMIN,20);	//�����ڴ�
	sprintf((char*)p,"Total Size:%d",msgq_total_size);	//��ʾDATA_Msg��Ϣ�����ܵĴ�С
	LCD_ShowString(10,150,100,16,16,p);
	sprintf((char*)p,"Remain Size:%d",msgq_remain_size);	//��ʾDATA_Msgʣ���С
	LCD_ShowString(10,190,100,16,16,p);
	myfree(SRAMIN,p);		//�ͷ��ڴ�
    taskEXIT_CRITICAL();    //�˳��ٽ���
}

int main(void)
{ 
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4
	delay_init(168);					//��ʼ����ʱ����
	uart_init(115200);     				//��ʼ������
	LED_Init();		        			//��ʼ��LED�˿�
	KEY_Init();							//��ʼ������
	BEEP_Init();						//��ʼ��������
	LCD_Init();							//��ʼ��LCD
	TIM9_Int_Init(5000,16800-1);		//��ʼ����ʱ��9������500ms
	my_mem_init(SRAMIN);            	//��ʼ���ڲ��ڴ��
    freertos_load_main_ui();        	//������UI
	
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
	
	//������Ϣ����
    Key_Queue=xQueueCreate(KEYMSG_Q_NUM,sizeof(u8));        //������ϢKey_Queue
    Message_Queue=xQueueCreate(MESSAGE_Q_NUM,USART_REC_LEN); //������ϢMessage_Queue,��������Ǵ��ڽ��ջ���������
	
    //����TASK1����
    xTaskCreate((TaskFunction_t )task1_task,             
                (const char*    )"task1_task",           
                (uint16_t       )TASK1_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )TASK1_TASK_PRIO,        
                (TaskHandle_t*  )&Task1Task_Handler);   
    //����TASK2����
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
		key=KEY_Scan(0);            	//ɨ�谴��
        if((Key_Queue!=NULL)&&(key))   	//��Ϣ����Key_Queue�����ɹ�,���Ұ���������
        {
            err=xQueueSend(Key_Queue,&key,10);
            if(err==errQUEUE_FULL)   	//���Ͱ���ֵ
            {
                printf("����Key_Queue���������ݷ���ʧ��!\r\n");
            }
        }
        i++;
        if(i%10==0) check_msg_queue();//��Message_Queue���е�����
        if(i==50)
        {
            i=0;
            LED0=!LED0;
        }
        vTaskDelay(10);                           //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
	}
}


//Keyprocess_task����
void Keyprocess_task(void *pvParameters)
{
	u8 num,key;
	while(1)
	{
        if(Key_Queue!=NULL)
        {
            if(xQueueReceive(Key_Queue,&key,portMAX_DELAY))//������ϢKey_Queue
            {
                switch(key)
                {
                    case WKUP_PRES:		//KEY_UP����LED1
                        LED1=!LED1;
                        break;
                    case KEY2_PRES:		//KEY2���Ʒ�����
                        BEEP=!BEEP;
                        break;
                    case KEY0_PRES:		//KEY0ˢ��LCD����
                        num++;
                        LCD_Fill(126,111,233,313,lcd_discolor[num%14]);
                        break;
                }
            }
        } 
		vTaskDelay(10);      //��ʱ10ms��Ҳ����10��ʱ�ӽ���	
	}
}

