#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "delay.h"
#include "air32f10x.h"
#include "math.h"
#include "EventRecorder.h"

int main(void)
{
    EventRecorderInitialize(EventRecordAll, 1U); //初始化事件记录器
    EventRecorderStart();                        //开始事件记录

    printf("hello world");
    return 0;
}
