void test(void)
{
    int tmp;
    __asm__(
        "mov r1,%0\n\t"
        :
        : "r"(tmp)
        : "r1"
    );
}

int main()
{
    test();
}