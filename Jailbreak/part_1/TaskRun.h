#include "private.h"
//自己定义个task_t
typedef int task_t;

//public 
task_t create_secret_task();

#define SecretTaskOptIsVip 0
#define SecretTaskOptVipLevel 1
#define SecretTaskVipLevelMax 9

void get_task_priority(task_t task_id);
bool secret_get_options(task_t task_id,int optkey,void *rek);


//导致UAF 的罪魁祸首 API 注销任务
bool free_task(task_t task_id);