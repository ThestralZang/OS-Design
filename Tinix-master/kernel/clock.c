
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"



/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//disp_str("#");
	ticks++;
	p_proc_ready->ticks--;
	if(p_proc_ready -> ticks == 0){
		p_proc_ready -> state == kTERMINALED;   //如果某进程运行完毕，则 kRUNNING to kTERMINALED
		return;
	}

	if (k_reenter != 0) {
		//disp_str("!");
		return;
	}

    //降级 kRUNNING to kRUNNABLE
    if(p_proc_ready->whichQuene == 1){
    	if(ticks%2 == 0 && p_proc_ready->ticks >0){
    		p_proc_ready->whichQuene=2;
    		secondQuene[secondLen]=p_proc_ready;
    		firstLen--;
    		secondLen++;
    		firstHead++;
    	}
    	else{
    		return;
    	}
    }
    else if(p_proc_ready->whichQuene == 2){
    	if(ticks%4 == 0 && p_proc_ready->ticks >0){
    		p_proc_ready->whichQuene=3;
    		thirdQuene[thirdLen]=p_proc_ready;
    		secondLen--;
    		thirdLen++;
    		firstHead++;
    	}
    	else{
    		return;
    	}
    }
    else{

    }
    p_proc_ready->state=kRUNNABLE;
    schedule();
	// if (p_proc_ready->ticks > 0) {        //当这里的ticks大于0是不会降级的，对应于时间片没用完的情况
	// 	return;

	// }
	// if (p_proc_ready->whichQuene==1)	//当ticks小于零，就会降级
	// {	
	// 	p_proc_ready->whichQuene=2;
	// 	p_proc_ready->ticks=2;
	// 	secondQuene[secondLen]=p_proc_ready;
	// 	secondLen++;
	// 	firstHead++;
	// }
	// else if(p_proc_ready->whichQuene==2){
	// 	p_proc_ready->whichQuene=3;
	// 	p_proc_ready->ticks=2;
	// 	thirdQuene[thirdLen]=p_proc_ready;
	// 	thirdLen++;
	// 	firstHead++;
	// }
	// else					//·ñÔòÊÇµÚ¶þ¸ö¶ÓÁÐµÄ
	// {
		
	// }
	// p_proc_ready->state=kRUNNABLE;
	// schedule();           //降级后进行调度，确定下一步将执行那个进程
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (t_8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (t_8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);	/* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);				/* 让8259A可以接受时钟中断 */
}


