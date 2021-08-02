#include "app_queue.h"

/*
移植说明：
1、app_queue.c中有两个不同的队列。一组是以app_batch开头的队列，另一组是以app_queue开头。 app_queue
开头的队列其实也能完成app_batch的功能，但是在处理批量数据时，app_batch接口效率更高。

2、什么是批量数据？指的是队列中所有元素的数据类型统一为以下几种：
2.1、unsigend char 或者 char (1字节)
2.2、unsigned shor int 或者 shor int (2字节)
2.3、unsigned int 或者 int (4字节)

比如：
uint16_t test[1024];
该数组中每个元素为队列的一个元素，每个元素类型为unsigned short int
*/

//批量队列的使用，请参考app_queue.h中的描述。这里只给出以app_queue开头的队列的使用方法

#define APP_Q_MAX 10

typedef struct 
{
    uint8_t *p;
    uint16_t len; 
}q_buf_t;

typedef struct 
{
    pos_t pos;   //定义队列时，pos_t类型的数据必须定义且必须在结构体第一个位置
    q_buf_t q_buf[APP_Q_MAX]; //应用层的队列，队列中任何数据类型都可以
}app_q_t;

app_q_t app_q;
uint8_t a[10];

main()
{
    q_buf_t test;
    q_buf_t test_tmp,test_target,test_replace;
    
    //队列初始化
    app_queue_init(&app_q, APP_Q_MAX, sizeof(q_buf_t)); 

    test.p = &a;
    test.len = 10;
    //入队
    app_enqueue(&app_q,&test);
    //出队
    app_enqueue(&app_q,&test_tmp);

    //我们着重看一下app_queue_traverse()以及app_queue_modify_speci_element()
    //的使用方法，因为这两者配合可以高效实现类似从队列中删除指定元素的效果
    uint8_t idx;
    uint16_t len;

    test_replace.p = 0;
    test_target.p = &a;
    len = app_queue_get_len(&app_q);
    for(uint8_t idx = 0; idx < len; idx++)
    {
        //取出队列中索引为idx的元素(队头idx=0，最后一个索引位置为队尾)
        app_queue_traverse(&app_q, &temp, idx);
        if(temp.p == test_target.p)
        {  
            //可以看到，这里找到要替换的目标后，用test_replace的数据去替换
            //目标数据.当调用app_dequeue(&app_q，&temp)出队的时候,碰到temp.p = 0
            //时不执行相应的代码即实现了队列删除功能.这里要注意，temp.p = 0是具有
            //唯一性的，程序正常执行的时候除非自己故意设置否则是不会出现的

            //另外，你也可以在q_buf_t 中再定一个标识变量专门用于身份识别
            app_queue_modify_speci_element(&app_q, idx, &test_replace); 
        }
    }
}

