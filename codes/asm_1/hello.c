//高级页面仔 iOS汇编入门教程（一）ARM64汇编基础 例子
#include <stdio.h>

int test(int a,int b)
{
    int res = a + b;
    return res;
}

//main入口
int main()
{
    int res = test(1,2);
    return 0;
}