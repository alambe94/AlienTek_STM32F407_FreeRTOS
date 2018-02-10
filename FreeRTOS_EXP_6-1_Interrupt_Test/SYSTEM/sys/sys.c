#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//ϵͳʱ�ӳ�ʼ��	
//����ʱ������/�жϹ���/GPIO���õ�
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//��
//////////////////////////////////////////////////////////////////////////////////  

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
void WFI_SET(void) {
	__asm volatile("WFI \n");
}
//�ر������ж�(���ǲ�����fault��NMI�ж�)
void INTX_DISABLE(void) {
	__asm volatile("CPSID   I \n"
			"BX      LR \n");
}
//���������ж�
void INTX_ENABLE(void) {
	__asm volatile("CPSIE   I \n"
			"BX      LR \n");
}
//����ջ����ַ
//addr:ջ����ַ
void MSR_MSP(u32 addr) {
	__asm volatile("MSR MSP, r0  \n"  //set Main Stack value
			"BX  r14 \n");
}

