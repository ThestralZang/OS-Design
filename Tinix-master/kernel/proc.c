
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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


void initializeAllPro()	//重新初始化所有进程，加入到不同的优先级队列中，注意是重新初始化
{
	PROCESS* p;
	firstLen=0;
	secondLen=0;
	thirdLen=0;
	int i;
	for(i=0; i<NR_TASKS+NR_PROCS;i++){
		p=&proc_table[i];
		p->state=kNEW;
		if(p->priority >= 20){
			firstQuene[firstLen]=p;
			firstLen++;
			p->ticks=30 - p->priority;
			p->whichQuene = 1;
		}else if(p->priority >= 10){
			secondQuene[secondLen]=p;
			secondLen++;
			p->ticks=30 - p-> priority;
			p->whichQuene=2;
		}
		else{
			thirdQuene[thirdLen]=p;
			thirdLen++;
			p->ticks=30 - p->priority;
			p->whichQuene=3;
		}
	}
	// PROCESS* p;
	// firstLen=0;
	// secondLen=0;
	// firstHead=0;
	// int i;
	// for (i=0; i<NR_TASKS+NR_PROCS; i++)
	// {
	// 	p=&proc_table[i]; 
	// 	if (p->state!=kRUNNABLE) continue; 
	// 	if (p->priority>=20)
	// 	{
	// 		firstQuene[firstLen]=p;
	// 		firstLen++;
	// 		p->ticks=2;
	// 		p->whichQuene=1;
			
	// 	}
	// 	else if(p->priority>=10)
	// 	{
	// 		secondQuene[secondLen]=p;
	// 		secondLen++;
	// 		p->ticks=p->priority;
	// 		p->whichQuene=2;
	// 	}
	// 	else{
	// 		thirdQuene[thirdLen]=p;
	// 		thirdLen++;
	// 		p->ticks=p->priority;
	// 		p->whichQuene=3;
	// 	}
	// }
}

/*======================================================================*
                              schedule
 *======================================================================*/


PUBLIC void schedule()
{
	PROCESS*	p;
	int 		greatest_priority=0;
	int 		i;
	int         max_resource_num=5;
	
    //find the 5 highest priority processes 
    int m=0; int n=0;
    int numOfCanBeRunnable = 0;
    int numOfLeftResource = max_resource_num;
    int processNumWithHighestPriority[max_resource_num];
    int numOfUsingResource = 0;
    for(m=0; m<NR_TASKS+NR_PROCS; m++){
    	if(proc_table[m].state == kNEW){
    		numOfCanBeRunnable++;
    	}else if(proc_table[m].state != kTERMINALED){
    		numOfLeftResource--;
    		processNumWithHighestPriority[numOfUsingResource++]=m;
    	}
    }
    int numOfToBeRunnable = 0;
    if(numOfLeftResource < numOfCanBeRunnable) numOfToBeRunnable = numOfLeftResource;
    else numOfToBeRunnable = numOfCanBeRunnable;

    for (m=0;m < numOfToBeRunnable; m++){
    	int highestPriority = 0;
    	for(n=0; n<NR_PROCS+NR_TASKS; n++){
    		if(proc_table[n].priority > highestPriority && proc_table[n].state == kNEW){
    			int isContained = 0;
    			int l = 0;
    			for(;l<m+numOfUsingResource;l++){
    				if(processNumWithHighestPriority[l] == n){
    					isContained = 1;
    					break;
    				}
    			}
    			if(isContained == 0){
    				highestPriority = proc_table[n].priority;
    				processNumWithHighestPriority[m] = n;
    			}
    		}
    	}
    }
    for( m=0; m<max_resource_num; m++){
    	int num = processNumWithHighestPriority[m];
    	if(proc_table[num].state ==kNEW){
    		proc_table[num].state == kRUNNABLE;          //当有空闲资源时，kNEW可转换为kRUNNABLE
    	}
    }

    //判断要操作哪个队列
    int queneToBeOperated = 0;
    int isAllTerminaled1 = 1;
    for(m=0; m<firstLen; m++){
    	if(firstQuene[m] -> state != kTERMINALED){
    		isAllTerminaled1 = 0;
    		break;
    	}
    }
    int isAllTerminaled2 = 1;
    for(m=0; m<secondLen; m++){
    	if(secondQuene[m] -> state != kTERMINALED){
    		isAllTerminaled2 = 0;
    		break;
    	}
    }
    int isAllTerminaled3 = 1;	
    for(m=0; m<thirdLen; m++){
    	if(thirdQuene[m] -> state != kTERMINALED){
    		isAllTerminaled3 = 0;
    		break;
    	}
    }
    if(firstLen != 0 && isAllTerminaled1 == 0)
    	queneToBeOperated = 1;
    else if(secondLen != 0 && isAllTerminaled2 == 0)
    	queneToBeOperated = 2;
    else if(thirdLen != 0 && isAllTerminaled3 == 0)
    	queneToBeOperated = 3;
    else{
    	initializeAllPro();     //如果进程全部死亡，那么将重新全部初始化，再来一遍
        return ;
    }

    
    //第一级队列 优先级调度 kRUNNABLE to kRUNNING
    if(queneToBeOperated == 1 && ticks%2 == 0){
    	int highestPriorityInQuene1 = 0;
    	int pos = 0;
    	for(m=0; m<firstLen; m++){
    		if(firstQuene[m] -> priority > highestPriorityInQuene1 && firstQuene[m] -> state == kRUNNABLE){
    			highestPriorityInQuene1 = firstQuene[m] -> priority;
    			pos = m;
    		}
    	}
    	firstQuene[pos] -> state = kRUNNING;
    	p_proc_ready = firstQuene[pos];
    	return;
    }

    //第二级队列 优先级调度 kRUNNABLE to kRUNNING
    if(queneToBeOperated == 2 && ticks%4 == 0){
    	int highestPriorityInQuene2 = 0;
    	int pos = 0;
    	for(m=0; m<secondLen; m++){
    		if(secondQuene[m] -> priority > highestPriorityInQuene2 && secondQuene[m] -> state == kRUNNABLE){
    			highestPriorityInQuene2 = secondQuene[m] -> priority;
    			pos = m;
    		}
    	}
    	secondQuene[pos] -> state = kRUNNING;
    	p_proc_ready = secondQuene[pos];
    	return;
    }

    //第三级队列 FCFS kRUNNABLE to kRUNNING
    if(queneToBeOperated == 3 && ticks%8 == 0){
        int pos=0;
    	for(m = 0; m< thirdLen; m++){
    		if(thirdQuene[m]->state == kRUNNABLE){
    			pos = m;
    			break;
    		}
    	}
    	thirdQuene[pos] -> state = kRUNNING;
    	p_proc_ready = thirdQuene[pos];
    	return;
    }


	// while (!greatest_priority) 
	// {
	// 	if (firstLen-firstHead>0)
	// 	{		
	// 		p_proc_ready=firstQuene[firstHead];	//µÚÒ»¸ö¶ÓÁÐ°´ÕÕÏÈµ½ÏÈµÃ
	// 		greatest_priority=p_proc_ready->ticks;
	// 		break;
	// 	}
	// 	else						//µÚ¶þ¸ö¶ÓÁÐ°´ÕÕÓÅÏÈ¼¶
	// 	{
	// 		for (i=0; i<secondLen; i++)		//µÚ¶þ¸ö¶ÓÁÐÔöÉèÅÐ¶ÏÊÇ·ñÎªrunnable×´Ì¬
	// 		{
	// 			p=secondQuene[i];
	// 			if (p->state!=kRUNNABLE || p->ticks==0) continue;
	// 			if (p->ticks > greatest_priority) 
	// 			{
	// 				greatest_priority = p->ticks;
	// 				p_proc_ready = p;
	// 			}
	// 			/*{	ÏÂÃæ´úÂë±»¿¨ËÀ¡£¡£¡£ÎÒÒ²²»¶®ÎªÊ²Ã´¡£¡£
	// 				if (p->priority>greatest_priority && p->ticks!=0)
	// 				{
	// 					greatest_priority=p->priority;
	// 					p_proc_ready=p;
	// 				}
	// 			}*/
	// 		}
	// 	}
	// 	if (!greatest_priority)	initializeAllPro();
	// }
	// p_proc_ready->state=kRUNNING;
}


/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

