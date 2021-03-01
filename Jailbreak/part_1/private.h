//该 VIP 等级被记录在 task 的 options 中
struct secret_options {
    bool isVIP;//VIP
    int vipLevel;
};
struct secret_task {
    int tid;
    bool valid;
    struct secret_options *options;//options结构体，里面有记录VIP信息
};


#include <iostream>
#include <string>
#include <map>
using namespace std;
typedef struct task *task_t;

//定义一个map 用来存task相关数据
std::map<task_t,struct secret_task *> taskTable;


