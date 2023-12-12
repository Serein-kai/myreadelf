#include <iostream>
using namespace std;
int global_init_var=23;
int global_uninit_var;

void func1(int i){
    cout<<"局部变量的结果为："<<i<<endl;
}

int main()
{
    static int static_var1=25;
    static int static_var2;
    int a=1;
    int b;
    func1(static_var1+static_var2+a+b);

    return 0;
}