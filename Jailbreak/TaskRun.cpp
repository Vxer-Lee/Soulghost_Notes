#include "private.h"
#include <stdlib.h>

//对外暴露这个函数 SDK 公开API
task_t create_secret_task(){
    struct secret_task *task = (struct secret_task *)calloc(1,sizeof(struct secret_task));
    //随机生成一个tid
    task->tid = arc4random();

    while( taskTable.find(task->tid = arc4random())  != taskTable.end());//这里有多少个map???? ,应该是就一个
    taskTable[task->tid] = task;//把task任务分配给他

    //分配options
    struct secret_options *options = (struct secret_options *)calloc(1,sizeof(struct secret_options));
    task->options = options;//设置options
    //分配VIP
    options->isVIP = false;
    options->vipLevel = 0;
    return task->tid;//只暴露tid ,把tid返回给用户
}

//其他方便操作task_id task的封装API

//获取任务优先级
void get_task_priority(task_t task_id){
    //根据task_id 获取task实例
    struct secret_task *task = get_task(task_id);
    if(!task){
        return (0);
    }
    return task->options->isVIP ? (SecretTaskVipLevelMax - task->options->vipLevel) :(0);
}
//获取options 设置信息
bool secret_get_options(task_t task_id,int optkey,void *rek){
    struct secret_task *task = get_task(task_id);
    if(!task){
        return (0);
    }
    switch (optkey)
    {
    case SecretTaskOptIsVip:
        *(reinterpret_cast <bool *>(ret)) = task->options->isVIP;
        break;
    case SecretTaskOptVipLevel:
        *(reinterpret_cast <bool *>(ret)) = task->options->vipLevel;
        break;
    default:
        break;
    }
    return true;
}
//

//还提供了注销任务的API 就是这里导致了UAF
bool free_task(task_t task_id){
    struct secret_task *task = get_task(task_id);
    if(!task){
        return (0);
    }

    //这一句代码有问题，只是释放了task所指向的options内存，并没有把task->options指针清空
    free(task->options);
    task->valid = false;
    return true;
}


