#include <stdio.h>

int main() {
    char name[100];
    
    printf("Hello, World!\n");
    
    // 新增：获取用户输入
    printf("请输入你的名字: ");
    scanf("%s", name);
    
    printf("你好, %s! 欢迎学习C语言!\n", name);
    
    return 0;
}
