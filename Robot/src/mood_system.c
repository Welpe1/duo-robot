#include "robot_utils.h"
#include "mood_system.h"
#include "oledfont.h"


void happy_increase(struct Mood_Info *stMood_Info)
{
    /**
     * happy+=2
     * sad-=2
     * peace,fear,angry不变
     * 
     */
    uint16_t sad = stMood_Info->pr_pointer[SAD];  
    uint16_t fear = stMood_Info->pr_pointer[FEAR];  
    uint16_t peace = stMood_Info->pr_pointer[PEACE];  
    if (sad < fear - THRESHOLD_DIFF) stMood_Info->pr_pointer[SAD] += 2;  
    if (peace < sad - THRESHOLD_DIFF) stMood_Info->pr_pointer[PEACE] += 2;  
    
}

void peace_increase(struct Mood_Info *stMood_Info)
{
    /**
     * peace+=2
     * fear-=1 angry-=1
     * happy,sad不变
     * 
     */
    uint16_t sad = stMood_Info->pr_pointer[SAD];  
    uint16_t fear = stMood_Info->pr_pointer[FEAR];  
    uint16_t angry = stMood_Info->pr_pointer[ANGRY]; 
    if(angry < MAX_MOOD_TYPES - THRESHOLD_DIFF) stMood_Info->pr_pointer[ANGRY]+=1;
    if(fear < angry - THRESHOLD_DIFF) stMood_Info->pr_pointer[FEAR]+=2;
    if(sad < fear - THRESHOLD_DIFF) stMood_Info->pr_pointer[SAD]+=2;
}

void sad_increase(struct Mood_Info *stMood_Info)
{
    /**
     * happy-=2
     * sad+=2
     * peace,fear,angry不变
     * 
     */

    uint16_t peace = stMood_Info->pr_pointer[PEACE];  
    uint16_t sad = stMood_Info->pr_pointer[SAD];  
    if(peace > 0 + THRESHOLD_DIFF)  stMood_Info->pr_pointer[PEACE]-=2;
    if(sad > peace + THRESHOLD_DIFF) stMood_Info->pr_pointer[SAD]-=2; 
}

void fear_increase(struct Mood_Info *stMood_Info)
{
    /**
     * peace-=2
     * fear+=2
     * happy,sad,angry不变
     * 
     */
    uint16_t peace = stMood_Info->pr_pointer[PEACE];  
    uint16_t sad = stMood_Info->pr_pointer[SAD];  
    uint16_t fear = stMood_Info->pr_pointer[FEAR];  
    if(sad > peace + THRESHOLD_DIFF) stMood_Info->pr_pointer[SAD]-=2; 
    if(fear > sad + THRESHOLD_DIFF) stMood_Info->pr_pointer[FEAR]-=2;

}

void angry_increase(struct Mood_Info *stMood_Info)
{
    /**
     * peace-=1
     * happy-=1
     * angry+=2
     * sad,fear不变
     */
    uint16_t peace = stMood_Info->pr_pointer[PEACE]; 
    uint16_t sad = stMood_Info->pr_pointer[SAD];  
    uint16_t fear = stMood_Info->pr_pointer[FEAR];  
    uint16_t angry = stMood_Info->pr_pointer[ANGRY]; 
    if(peace > 0 + THRESHOLD_DIFF)  stMood_Info->pr_pointer[PEACE]-=1;
    if(sad > peace + THRESHOLD_DIFF) stMood_Info->pr_pointer[SAD]-=2; 
    if(fear > sad + THRESHOLD_DIFF) stMood_Info->pr_pointer[FEAR]-=2;
    if(angry > fear + THRESHOLD_DIFF) stMood_Info->pr_pointer[ANGRY]-=2;

}

void mood_rand_convert(struct Mood_Info *stMood_Info)
{

    uint16_t random = rand() % 1000;
    uint16_t p_num=stMood_Info->pr_pointer[SAD]-stMood_Info->pr_pointer[HAPPY];       //积极情绪数 happy+peace
    uint16_t n_num=stMood_Info->pr_pointer[SPECIAL]-stMood_Info->pr_pointer[SAD];         //消极情绪数 sad+fear+angry
    uint16_t n_random= p_num+rand()%n_num;
    uint8_t mood_temp=0;

    /**
     * 根据上一时刻的状态与当前随机数决定下一时刻的状态
     * 
     * */
    switch (stMood_Info->last_state)
    {
        case HAPPY:
        {
            /**
             * 上一状态为happy时
             * 下一状态只能是peace happy
             * 转换为peace的概率为 peace/(happy+peace)
             * 
             */
            int p_random= rand()%p_num;
            if(p_random > stMood_Info->pr_pointer[PEACE]) stMood_Info->state=PEACE;
            break;
        };
        case PEACE:
        {
            /**
             * 上一状态为peace时
             * 下一状态可以任意转换
             * 按各情绪占比转换
             * 
             */
            for(mood_temp=0;mood_temp<MAX_MOOD_TYPES-1;mood_temp++)
            {
                if(random >= stMood_Info->pr_pointer[mood_temp] && random <=stMood_Info->pr_pointer[mood_temp+1])
                break;
            }
            stMood_Info->state=mood_temp;
            break;
        };
        case SAD:
        {
            /**
             * 上一状态为sad时
             * 下一状态是sad angry fear peace
             * 转换为peace的概率为  1/3
             * 转换为sad的概率为    2/3 * (sad/(sad+angry+fear))
             * 转换为fear的概率为   2/3 * (fear/(sad+angry+fear))
             * 转换为angry的概率为  2/3 * (angry/(sad+angry+fear))
             * 
             */
            if(n_random % 3 >= 1)
            {
                for(mood_temp=2;mood_temp<MAX_MOOD_TYPES-1;mood_temp++)
                {
                    if(n_random >= stMood_Info->pr_pointer[mood_temp] && n_random <=stMood_Info->pr_pointer[mood_temp+1])
                    break;
                }
                stMood_Info->state=mood_temp;

            }else{
                stMood_Info->state = PEACE;
            }
        

            break;
        };
        case FEAR:
        {
            /**
             * 上一状态为fear时
             * 下一状态是sad peace fear
             * 转换为peace的概率为  angry/(sad+angry+fear)
             * 转换为sad的概率为    sad/(sad+angry+fear)
             * 转换为fear的概率为   fear/(sad+angry+fear)
             * 
             */

            for(mood_temp=2;mood_temp<MAX_MOOD_TYPES-1;mood_temp++)
            {
                if(n_random >= stMood_Info->pr_pointer[mood_temp] && n_random <=stMood_Info->pr_pointer[mood_temp+1])
                break;
            }
            stMood_Info->state=mood_temp;
            if(mood_temp == ANGRY) stMood_Info->state = PEACE;

            break;
        }
        case ANGRY:
        {
            /**
             * 上一状态为angry时
             * 下一状态是sad peace angry
             * 转换为peace的概率为  fear/(sad+angry+fear)
             * 转换为sad的概率为    sad/(sad+angry+fear)
             * 转换为angry的概率为  angry/(sad+angry+fear)
             * 
             */

            for(mood_temp=2;mood_temp<MAX_MOOD_TYPES-1;mood_temp++)
            {
                if(n_random >= stMood_Info->pr_pointer[mood_temp] && n_random <=stMood_Info->pr_pointer[mood_temp+1])
                break;
            }
            stMood_Info->state=mood_temp;
            if(mood_temp == FEAR) stMood_Info->state = PEACE;
            
            break;
        };
        case SPECIAL:
        {
            stMood_Info->state = PEACE;
            break;
        };
        
    }



    /**
     * 决定下一帧是动态表情(1/3)还是静态表情(2/3)
     */
    if(random % 3 < 2){   //静态
    printf("随机决定下一帧是静态表情\r\n");
        stMood_Info->stEMO_Info.state = STATIC;
        stMood_Info->stEMO_Info.static_select = rand() % 10;    //0~9
        stMood_Info->stEMO_Info.select_pointer=emo_static_group[stMood_Info->state][stMood_Info->stEMO_Info.static_select]; 

    }else{  //动态
        printf("随机决定下一帧是动态表情\r\n");
        stMood_Info->stEMO_Info.state = DYNAMICS;
        stMood_Info->stEMO_Info.dynamics_select = rand() % 5;    //0~4
        stMood_Info->stEMO_Info.dynamics_count=emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_select][0];
        printf("动态随机选择器=%d\r\n",stMood_Info->stEMO_Info.dynamics_select);
        printf("动态表情计数器=%d\r\n",stMood_Info->stEMO_Info.dynamics_count);
        // stMood_Info->stEMO_Info.select_pointer=emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_select][1]; 
    }

}

void mood_pr_limit(struct Mood_Info *stMood_Info)
{
    uint16_t p_num=stMood_Info->pr_pointer[SAD]-stMood_Info->pr_pointer[HAPPY];       //积极情绪数 happy+peace
    uint16_t n_num=stMood_Info->pr_pointer[SPECIAL]-stMood_Info->pr_pointer[SAD];         //消极情绪数 sad+fear+angry

    stMood_Info->pr_pointer[SAD]=LIMIT(stMood_Info->pr_pointer[SAD],600,800);
    //printf("stMood_Info->pr_pointer[SAD]=%d\r\n",stMood_Info->pr_pointer[SAD]);

    p_num=stMood_Info->pr_pointer[SAD]-stMood_Info->pr_pointer[HAPPY];       //积极情绪数 happy+peace
    n_num=stMood_Info->pr_pointer[SPECIAL]-stMood_Info->pr_pointer[SAD];         //消极情绪数 sad+fear+angry

    stMood_Info->pr_pointer[PEACE]=LIMIT(stMood_Info->pr_pointer[PEACE],0.3*p_num,0.6*p_num);
    stMood_Info->pr_pointer[ANGRY]=LIMIT(stMood_Info->pr_pointer[ANGRY],MAX_PR_NUM-0.25*n_num,MAX_PR_NUM-0.1*n_num);
    stMood_Info->pr_pointer[FEAR]=LIMIT(stMood_Info->pr_pointer[FEAR],stMood_Info->pr_pointer[ANGRY]-0.25*n_num,stMood_Info->pr_pointer[ANGRY]-0.1*n_num);

}

/**
 * 此函数决定下一时刻oled显示的表情
 * 根据上一时刻的状态与当前随机数决定下一时刻的状态
 * 如果有事件触发则事件优先级最高，处理完毕后事件触发清0
 * 当事件触发后，情绪会改变(mood++)
 */

void mood_update(struct Mood_Operation *stMood_Operation,struct Mood_Info *stMood_Info)
{

    
    stMood_Info->last_state=stMood_Info->state;   //记录上一时刻的情绪状态
  
    stMood_Info->stEMO_Info.last_state=stMood_Info->stEMO_Info.state;
 
    if(stMood_Info->stEMO_Info.last_state ==0)
    {
        printf("上一时刻表情状态=静态\r\n");
    }
    
    else
    {
        printf("上一时刻表情状态=动态\r\n");
    }
    

   

    if(stMood_Info->stEMO_Info.last_state == STATIC || stMood_Info->event_trigger){
        printf("上一帧是静态表情\r\n");
        //stMood_Info->stEMO_Info.last_state = STATIC;
        if(stMood_Info->event_trigger){
            printf("有事件触发\r\n");
            stMood_Info->state = SPECIAL;
            for(uint8_t i=0;i<5;i++){
                if(read_bit(stMood_Info->event_trigger,i)){
                    stMood_Operation->mood_increase[i](stMood_Info);
                    stMood_Info->stEMO_Info.select_pointer=emo_static_group[stMood_Info->state][i+stMood_Info->stEMO_Info.static_select % 2];     
                }

            }
        
           stMood_Operation->mood_pr_limit(stMood_Info);
            for(int i=0;i<MAX_MOOD_TYPES-1;i++){
                stMood_Info->pr[i]=(float)(stMood_Info->pr_pointer[i+1]-stMood_Info->pr_pointer[i])/(MAX_PR_NUM+1);
            }
            stMood_Info->event_trigger=0;
           

        }else{
           
            stMood_Operation->mood_rand_convert(stMood_Info);  
      
        }
    }else if(stMood_Info->stEMO_Info.last_state == DYNAMICS){  //上一帧是动态
    
 

        stMood_Info->stEMO_Info.dynamics_count--;
        
        stMood_Info->stEMO_Info.select_pointer=emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_select][emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_select][0]-stMood_Info->stEMO_Info.dynamics_count];
        printf("动态帧指针为%d\r\n",stMood_Info->stEMO_Info.select_pointer);
        printf("剩余动态帧数为%d\r\n",stMood_Info->stEMO_Info.dynamics_count);
        if(stMood_Info->stEMO_Info.dynamics_count == 0)
        {
            stMood_Info->stEMO_Info.state = 0;
            printf("动态帧显示完毕\r\n");
            
        }

    }

    printf("\r\n");

    
}

void mood_system_init(struct Mood_Operation *mood_operation,struct Mood_Info *stMood_Info)
{

    for(int i=0;i<MAX_MOOD_TYPES;i++)   //0~5  
	{
        strcpy(stMood_Info->name[i], mood_types[i]); 
        stMood_Info->pr_pointer[i]= mood_pr_end[i];
        if(i > 0) stMood_Info->pr[i-1]=(float)(stMood_Info->pr_pointer[i]-stMood_Info->pr_pointer[i-1])/(MAX_PR_NUM+1);
        
	}

    mood_operation->mood_increase[0]=happy_increase;
    mood_operation->mood_increase[1]=peace_increase;
    mood_operation->mood_increase[2]=sad_increase;
    mood_operation->mood_increase[3]=fear_increase;
    mood_operation->mood_increase[4]=angry_increase;
    mood_operation->mood_update=mood_update;
    mood_operation->mood_rand_convert=mood_rand_convert;
    mood_operation->mood_pr_limit=mood_pr_limit;

    stMood_Info->state = PEACE;
    stMood_Info->last_state = PEACE;
    stMood_Info->stEMO_Info.last_state == STATIC;


}

















// void happy_increase(struct Mood_Info *stMood_Info)
// {
//     /**
//      * happy+=2
//      * sad-=2
//      * peace,fear,angry不变
//      * 
//      */

//     stMood_Info->pr_start[SAD]+=2;
//     stMood_Info->pr_end[PEACE]+=2;
//     stMood_Info->pr_start[PEACE]+=2;
//     stMood_Info->pr_end[HAPPY]+=2;

// }

// void peace_increase(struct Mood_Info *stMood_Info)
// {
//     /**
//      * peace+=2
//      * fear-=1 angry-=1
//      * happy,sad不变
//      * 
//      */

//     stMood_Info->pr_start[ANGRY]+=1;
//     stMood_Info->pr_end[FEAR]+=1;
//     stMood_Info->pr_start[FEAR]+=2;
//     stMood_Info->pr_end[SAD]+=2;
//     stMood_Info->pr_start[SAD]+=2;
//     stMood_Info->pr_end[PEACE]+=2;




// }

// void sad_increase(struct Mood_Info *stMood_Info)
// {
//     /**
//      * happy-=2
//      * sad+=2
//      * peace,fear,angry不变
//      * 
//      */
//    stMood_Info->pr_end[HAPPY]-=2;
//    stMood_Info->pr_start[PEACE]-=2;
//    stMood_Info->pr_end[PEACE]-=2;
//    stMood_Info->pr_start[SAD]-=2;
    
// }

// void fear_increase(struct Mood_Info *stMood_Info)
// {
//     /**
//      * peace-=2
//      * fear+=2
//      * happy,sad,angry不变
//      * 
//      */
//    stMood_Info->pr_end[PEACE]-=2;
//    stMood_Info->pr_start[SAD]-=2;
//    stMood_Info->pr_end[SAD]-=2;
//    stMood_Info->pr_start[FEAR]-=2;
    
// }

// void angry_increase(struct Mood_Info *stMood_Info)
// {
//     /**
//      * peace-=1
//      * happy-=1
//      * angry+=2
//      * sad,fear不变
//      * 
//      */
//    stMood_Info->pr_end[HAPPY]-=1;
//    stMood_Info->pr_start[PEACE]-=1;
//    stMood_Info->pr_end[PEACE]-=2;
//    stMood_Info->pr_start[SAD]-=2;
//    stMood_Info->pr_end[SAD]-=2;
//    stMood_Info->pr_start[FEAR]-=2;
//    stMood_Info->pr_end[FEAR]-=2;
//    stMood_Info->pr_start[ANGRY]-=2;
    
// }

// void mood_state_convert(struct Mood_Info *stMood_Info,int16_t random)
// {

//     int16_t p_num=stMood_Info->pr_end[PEACE]-stMood_Info->pr_start[HAPPY]+1;       //积极情绪数 happy+peace
//     int16_t n_num=stMood_Info->pr_end[ANGRY]-stMood_Info->pr_start[SAD]+1;         //消极情绪数 sad+fear+angry
//     int16_t n_random= p_num+rand()%n_num;
//     uint8_t mood_temp=0;


//     /**
//      * 根据上一时刻的状态与当前随机数决定下一时刻的状态
//      * 
//      * */
//     switch (stMood_Info->last_state)
//     {
//         case HAPPY:
//         {
//             /**
//              * 上一状态为happy时
//              * 下一状态只能是peace happy
//              * 转换为peace的概率为 peace/(happy+peace)
//              * 
//              */
//             int p_random= rand()%p_num;
//             if(p_random > stMood_Info->pr_end[HAPPY]) stMood_Info->state=PEACE;
//             break;
//         };
//         case PEACE:
//         {
//             /**
//              * 上一状态为peace时
//              * 下一状态可以任意转换
//              * 按各情绪占比转换
//              * 
//              */
//             for(mood_temp=0;mood_temp<MAX_MOOD_TYPES;mood_temp++)
//             {
//                 if(random >= stMood_Info->pr_start[mood_temp] && random <=stMood_Info->pr_end[mood_temp])
//                 break;
//             }
//             stMood_Info->state=mood_temp;
//             break;
//         };
//         case SAD:
//         {
//             /**
//              * 上一状态为sad时
//              * 下一状态是sad angry fear peace
//              * 转换为peace的概率为  1/3
//              * 转换为sad的概率为    2/3 * (sad/(sad+angry+fear))
//              * 转换为fear的概率为   2/3 * (fear/(sad+angry+fear))
//              * 转换为angry的概率为  2/3 * (angry/(sad+angry+fear))
//              * 
//              */
//             if(n_random % 3 >= 1)
//             {
//                 for(mood_temp=2;mood_temp<MAX_MOOD_TYPES;mood_temp++)
//                 {
//                     if(n_random >= stMood_Info->pr_start[mood_temp] && n_random <=stMood_Info->pr_end[mood_temp])
//                     break;
//                 }
//                 stMood_Info->state=mood_temp;

//             }else{
//                 stMood_Info->state = PEACE;
//             }
        

//             break;
//         };
//         case FEAR:
//         {
//             /**
//              * 上一状态为fear时
//              * 下一状态是sad peace fear
//              * 转换为peace的概率为  angry/(sad+angry+fear)
//              * 转换为sad的概率为    sad/(sad+angry+fear)
//              * 转换为fear的概率为   fear/(sad+angry+fear)
//              * 
//              */

//             for(mood_temp=2;mood_temp<MAX_MOOD_TYPES;mood_temp++)
//             {
//                 if(n_random >= stMood_Info->pr_start[mood_temp] && n_random <=stMood_Info->pr_end[mood_temp])
//                 break;
//             }
//             stMood_Info->state=mood_temp;
//             if(mood_temp == ANGRY) stMood_Info->state = PEACE;

//             break;
//         }
//         case ANGRY:
//         {
//             /**
//              * 上一状态为angry时
//              * 下一状态是sad peace angry
//              * 转换为peace的概率为  fear/(sad+angry+fear)
//              * 转换为sad的概率为    sad/(sad+angry+fear)
//              * 转换为angry的概率为  angry/(sad+angry+fear)
//              * 
//              */

//             for(mood_temp=2;mood_temp<MAX_MOOD_TYPES;mood_temp++)
//             {
//                 if(n_random >= stMood_Info->pr_start[mood_temp] && n_random <=stMood_Info->pr_end[mood_temp])
//                 break;
//             }
//             stMood_Info->state=mood_temp;
//             if(mood_temp == FEAR) stMood_Info->state = PEACE;
            
//             break;
//         };
       
//     }

    
//     //动态表情
//     if(random%30>=10)
//     {
//         stMood_Info->stEMO_Info.type=DYNAMICS; 
//         stMood_Info->stEMO_Info.dynamics_group=random%EMO_DYNAMICS_MAX;
//         stMood_Info->stEMO_Info.dynamics_count=emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_group][0];

//     }else{//静态表情
//         stMood_Info->stEMO_Info.type=STATIC; 
//         stMood_Info->stEMO_Info.dynamics_count=0;
//         stMood_Info->stEMO_Info.select=stMood_Info->state*30+random%10;
//     }



    




// }



// /**
//  * 积极情绪0.6~0.8 
//  * happy 0.3~0.6
//  * peace 0.4~0.7
//  * 
//  * 消极情绪0.2~0.4
//  * sad 0.5~0.8
//  * angry 0.1~0.25
//  * fear 0.1 ~0.25
//  * 
//  */
// void mood_pr_limit(struct Mood_Info *stMood_Info)
// {

//     LIMIT(stMood_Info->pr_end[PEACE],599,799);
//     LIMIT(stMood_Info->pr_end[HAPPY],stMood_Info->pr_end[PEACE]*0.3,stMood_Info->pr_end[PEACE]*0.6);  

//     LIMIT(stMood_Info->pr_end[SAD],stMood_Info->pr_end[PEACE],799);
        


// }

// void mood_system_init(struct Mood_Operation *stMood_Operation,struct Mood_Info *stMood_Info)
// {
//    stMood_Info->
   
   
//    pr_start[0]=0;
//    stMood_Info->pr_end[MAX_MOOD_TYPES-1]=MAX_PR_NUM;

//     for(uint8_t i=0;i<MAX_MOOD_TYPES;i++)
// 	{
//         strcpy(stMood_Info->name[i], mood_types[i]); 
//        stMood_Info->pr_end[i]= mood_pr_end[i];
//         if(i<MAX_MOOD_TYPES-1)stMood_Info->pr_start[i+1]=stMood_Info->pr_end[i]+1;
//        stMood_Info->pr[i]=((float)(stMood_Info->pr_end[i]-stMood_Info->pr_start[i]))/(MAX_PR_NUM+1);

// 	}


//     stMood_Info->state=PEACE;
//     stMood_Info->last_state=PEACE;
//     stMood_Info->stEMO_Info.type == STATIC;

//     stMood_Operation->mood_increase[0]=happy_increase;
//     stMood_Operation->mood_increase[1]=peace_increase;
//     stMood_Operation->mood_increase[2]=sad_increase;
//     stMood_Operation->mood_increase[3]=fear_increase;
//     stMood_Operation->mood_increase[4]=angry_increase;
//     stMood_Operation->mood_update=mood_update;
//     stMood_Operation->mood_state_convert=mood_state_convert;
//     stMood_Operation->mood_pr_limit=mood_pr_limit;
   

// }



// /**
//  * 此函数决定了下一时刻应该显示什么表情
//  * 根据上一时刻的状态与当前随机数决定下一时刻的状态
//  * 如果有事件触发则事件优先级最高，处理完毕后事件触发清0
//  */

// void mood_update(struct Mood_Operation *stMood_Operation,struct Mood_Info *stMood_Info)
// {

//     int16_t random=rand()%1000; 
//     stMood_Info->last_state=stMood_Info->state;   //记录上一时刻的情绪状态

//     if(stMood_Info->event_trigger == 0) //没有事件触发
//     {
//         //只有上一时刻的表情为静态表情才可以进行情绪转换，否则必须要等动态表情显示完毕
//         if(stMood_Info->stEMO_Info.type == STATIC)
//         {
//             stMood_Operation->mood_state_convert(stMood_Info,random);   //情绪转换

//         }else{
//             //动态表情没有显示结束
//             stMood_Info->stEMO_Info.select=emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_group][emo_dynamics_group[stMood_Info->state][stMood_Info->stEMO_Info.dynamics_group][0]-stMood_Info->stEMO_Info.dynamics_count+1];
//             stMood_Info->stEMO_Info.dynamics_count--;
//             if(stMood_Info->stEMO_Info.dynamics_count == 0)
//             {
//                 stMood_Info->stEMO_Info.type == STATIC;
//             }
            
//         }
    

        

//     }else{
    
//         for(uint8_t i=0;i<16;i++)
//         {
//             if(read_bit(stMood_Info->event_trigger,i)) 
//             {
//                 stMood_Operation->mood_increase[i](stMood_Info);
//                 stMood_Info->stEMO_Info.select=150+i*10+random%10;    //特殊表情
                
//             }

//         }

    

//         stMood_Info->event_trigger=0;
//         stMood_Info->state=PEACE;
//         stMood_Info->stEMO_Info.type=STATIC;
//         stMood_Info->stEMO_Info.dynamics_count=0;
//     }


//     //计算概率
//     for(int i=0;i<MAX_MOOD_TYPES;i++)
// 	{
//        stMood_Info->pr[i]=((float)(stMood_Info->pr_end[i]-stMood_Info->pr_start[i]))/(MAX_PR_NUM+1);
// 	}
// } 



