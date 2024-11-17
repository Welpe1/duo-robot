#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "mmio.h"
#include "delay.h"
#include "printf.h"
#include "rtos_cmdqu.h"
#include "cvi_mailbox.h"
#include "intr_conf.h"
#include "top_reg.h"
#include "memmap.h"
#include "comm.h"
#include "cvi_spinlock.h"

#include "duo_reg.h"
#include "duo_oled.h"
#include "duo_uart.h"
#include "duo_check.h"
#include "duo_mpu6050.h"
#include "duo_gpio.h"
#include "duo_pwm.h"
#include "arch_helpers.h"
#include "time.h"


DEFINE_CVI_SPINLOCK(mailbox_lock, SPIN_MBOX);

QueueHandle_t queue_handle;

/* mailbox */
volatile struct mailbox_set_register *mbox_reg;
volatile struct mailbox_done_register *mbox_done_reg;
volatile unsigned long *mailbox_context; // mailbox buffer context is 64 Bytess

volatile uint8_t *value=NULL;
uintptr_t addr;
uint32_t gFreertos_Data=0;
uint32_t gLinux_Data=0;

extern const uint8_t BMP1[];
extern const uint8_t BMP2[];
extern const uint8_t BMP3[];
extern const uint8_t BMP4[];

TaskHandle_t check_task_handle;
TaskHandle_t oled_task_handle;



void my_set_bit(uint64_t *value,uint8_t position,uint8_t bit)
{
    if(bit){
        *value |= (1 << position);
    }else{
        *value &=~(1 << position);
    }
}


void oled_task()
{
    struct pwm_config PWM10;
    pwm_init(&PWM10,10);
    while(1) 
    {
    //printf("oled_task running\r\n");

        oled_clear();
        oled_update();
        vTaskDelay(50);

        oled_full();
        oled_update();
        vTaskDelay(50);

        oled_show_image(0,0,128,64,BMP1);
        oled_update();
        vTaskDelay(50);

        oled_show_image(0,0,128,64,BMP2);
        oled_update();
        vTaskDelay(50);

        oled_show_image(0,0,128,64,BMP3);
        oled_update();
        vTaskDelay(50);

        oled_show_image(0,0,128,64,BMP4);
        oled_update();
        vTaskDelay(50);

        uint16_t i=0;
        for(i=0;i<180;i++)
        {
            set_angle(&PWM10,i);
            vTaskDelay(5);
        }
        i=0;




    }
}

void mailbox_task(void)
{
	cmdqu_t from_linux;		//接收来自linux的数据
	cmdqu_t *cmdq;
	cmdqu_t *to_linux;		//向linux发送数据
	int ret = 0;
	int flags;		//存储中断标志。
	int valid;		//检查mailbox的有效性

	unsigned int reg_base = MAILBOX_REG_BASE;

	cmdq = &from_linux;

	mbox_reg = (struct mailbox_set_register *) reg_base;
	mbox_done_reg = (struct mailbox_done_register *) (reg_base + 2);
	mailbox_context = (unsigned long *) (MAILBOX_REG_BUFF);

	cvi_spinlock_init();
	printf(" prvCmdQuRunTask_mailbox run\n");

	while(1) {
		xQueueReceive(queue_handle, &from_linux, portMAX_DELAY);
		switch (from_linux.cmd_id) {

            case CMD_TEST_B:
                printf("cmd_test_b\r\n");
                from_linux.cmd_id = CMD_TEST_B;
				from_linux.resv.valid.rtos_valid = 1;
				from_linux.resv.valid.linux_valid = 0;
				addr=(uintptr_t)from_linux.param_ptr;
                value=(volatile uint8_t*)addr;
	            printf("addr=%p\r\n",addr);
              
				goto send_label;
	
			case CMD_TEST_C:
                printf("cmd_test_c\r\n");
				from_linux.cmd_id = CMD_TEST_C;
				from_linux.resv.valid.rtos_valid = 1;
				from_linux.resv.valid.linux_valid = 0;
				gLinux_Data=(volatile uint32_t)from_linux.param_ptr;
				printf("gLinux_Data=%x\n",gLinux_Data);
				goto send_label;

			default:
send_label:
				to_linux = (cmdqu_t *) mailbox_context;		//指向mailbox寄存器

				drv_spin_lock_irqsave(&mailbox_lock, flags);
				if (flags == MAILBOX_LOCK_FAILED) {
					printf("[%s][%d] drv_spin_lock_irqsave failed! ip_id = %d , cmd_id = %d\n" , cmdq->ip_id , cmdq->cmd_id);
					break;
				}

				for (valid = 0; valid < MAILBOX_MAX_NUM; valid++) {
					if (to_linux->resv.valid.linux_valid == 0 && to_linux->resv.valid.rtos_valid == 0) {
						/**
						 * 本rtos已经使用了一个mailbox（linux向rtos传递信息），需要找一个新的mailbox（rtos向linux返回信息）
						 * 遍历MAILBOX_MAX_NUM个mailbox，找到一个既没有被linux使用也没有被rtos使用的mailbox
						 * 
						 */
						// mailbox buffer context is 4 bytes write access
						int *ptr = (int *)to_linux;

						from_linux.resv.valid.rtos_valid = 1;
						*ptr = ((from_linux.ip_id << 0) | (from_linux.cmd_id << 8) | (from_linux.block << 15) |
								(from_linux.resv.valid.linux_valid << 16) |
								(from_linux.resv.valid.rtos_valid << 24));

						//小核向大核传递数据
						to_linux->param_ptr = gFreertos_Data;
						gFreertos_Data=0;

						// clear mailbox
						mbox_reg->cpu_mbox_set[SEND_TO_CPU1].cpu_mbox_int_clr.mbox_int_clr = (1 << valid);
						// trigger mailbox valid to rtos
						mbox_reg->cpu_mbox_en[SEND_TO_CPU1].mbox_info |= (1 << valid);
						mbox_reg->mbox_set.mbox_set = (1 << valid);
						break;
					}
					to_linux++;
				}
				drv_spin_unlock_irqrestore(&mailbox_lock, flags);
				if (valid >= MAILBOX_MAX_NUM) {
				    printf("No valid mailbox is available\n");
				    return -1;
				}
				gFreertos_Data=0;
				break;


		}
	}
}

void check_task()
{
    uint32_t ret=0;
    check_system_init();
    //iic0
    check_system_push(PINMUX_BASE + PINMUX_GP0,3);
    check_system_push(PINMUX_BASE + PINMUX_GP1,3);
    //pwm10
    check_system_push(PINMUX_BASE + PINMUX_GP2,7);

	while(1)
	{
		printf("check_task running\r\n");
		if(!check_system_run())
		{
            check_system_destroy();
            printf("soft i2c0 get ready\r\n");

            oled_init();
            xTaskCreate(oled_task, "oled_task", 1024 * 8, NULL, 1, NULL);
            vTaskDelete(NULL);
		  
		}
		vTaskDelay(50);

	}

}



/**
 * 用于处理来自Linux系统的mailbox中断
 * 当Linux系统向RTOS（实时操作系统）发送命令时，会触发这个中断
 * 
 */
void prvQueueISR(void)
{
	printf("prvQueueISR\n");
	unsigned char set_val;		//存储mailbox中断设置寄存器的值 (set_val!=0，表示有中断发生)
	unsigned char valid_val;		//检查特定的mailbox条目是否有效
	int i;
	cmdqu_t *cmdq;
	BaseType_t YieldRequired = pdFALSE;

	set_val = mbox_reg->cpu_mbox_set[RECEIVE_CPU].cpu_mbox_int_int.mbox_int;

	if (set_val) {//如果set_val不为0，表示有中断发生
		for(i = 0; i < MAILBOX_MAX_NUM; i++) {		//遍历所有mailbox
			valid_val = set_val  & (1 << i);		//找到有效的mailbox

			if (valid_val) {
				cmdqu_t rtos_cmdq;
				cmdq = (cmdqu_t *)(mailbox_context) + i;		//计算cmdq指针，使其指向当前有效的mailbox条目

			
				/* mailbox buffer context is send from linux, clear mailbox interrupt */
				mbox_reg->cpu_mbox_set[RECEIVE_CPU].cpu_mbox_int_clr.mbox_int_clr = valid_val;		//清除mailbox中断标志
				// need to disable enable bit
				mbox_reg->cpu_mbox_en[RECEIVE_CPU].mbox_info &= ~valid_val;

				// copy cmdq context (8 bytes) to buffer ASAP
				*((unsigned long *) &rtos_cmdq) = *((unsigned long *)cmdq);		//将mailbox条目的内容复制到rtos_cmdq中
				/* need to clear mailbox interrupt before clear mailbox buffer */
				*((unsigned long*) cmdq) = 0;		//清除mailbox条目的内容（将其设置为0）

				/* mailbox buffer context is send from linux*/
				if (rtos_cmdq.resv.valid.linux_valid == 1) {	//检查rtos_cmdq的linux_valid标志，如果为1，则表示命令来自Linux系统，需要将其发送到RTOS的任务队列中

					xQueueSendFromISR(queue_handle, &rtos_cmdq, &YieldRequired);

					portYIELD_FROM_ISR(YieldRequired);
				} 
			}
		}
	}
}

void prvDetectISR(void)
{
    printf("GPIOA 23 IRQ");
    my_set_bit(&gFreertos_Data,0,1);
    // gFreertos_Data=0x01;
    gpio_clear_it(GPIOA);
    
}


struct timeval cur_t;
struct timeval last_t;
// void prvTouchISR(void)
// {
//     printf("GPIOA 24 IRQ");

//     if(cur_t.tv_sec-last_t.tv_sec>1)
//     {
//         gettimeofday(&cur_t, NULL);
//         my_set_bit(&gFreertos_Data,1,1);

//     }
//     last_t=cur_t;

  


//     printf("cur_t=%d\r\n",cur_t.tv_sec);


//     gFreertos_Data=0x01;
//     gpio_clear_it(GPIOA);
    
// }


void main_cvirtos(void)
{
	printf("create cvi task\n");
	//value = (uint8_t *)malloc(4 * sizeof(uint8_t));
    gpio_irq_init(GPIOA,23,2);
    gpio_irq_init(GPIOA,24,2);

	request_irq(MBOX_INT_C906_2ND,prvQueueISR, 0, "mailbox", (void *)0);
    request_irq(GPIO0_INTR_FLAG,prvDetectISR,0,"gpio_isr",(void*)0);
	queue_handle = xQueueCreate(30, sizeof(cmdqu_t));

	if(queue_handle != NULL)
	{
		xTaskCreate(mailbox_task,"mailbox_task",configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5,NULL);		//创建任务
        xTaskCreate(check_task, "check_task", 1024 * 8, NULL, 1, NULL);
	}
	
    vTaskStartScheduler();
	while(1);
}
