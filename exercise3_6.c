#include <stdio.h>
/*6. 编写程序:
比较两个字符串S1 和S2，如果S1>S2，输出一个正数；如果S1=S2，输出0；如果S1<S2，输出一个负数；
输出的正，负数值为两个字符串相应位置字符ASCII 码值的差值，当两个字符串完全一样时，则认为S1=S2。
请使用gets 或puts 函数输入，输出字符串。不能使用string.h 中的系统函数。*/

int strcmp(char *str1, char *str2){
	while (*str1 != '\0' && *str2 != '\0') {
		if (*str1 != *str2) break;
		str1++;
		str2++;
	}
	return *str1 - *str2;
}

int main(int argc, char const *argv[])
{
	#define BUFFSIZE 20
	char str1[BUFFSIZE], str2[BUFFSIZE];
	int ret;
	//use fgets instead of gets as gets are terribly vulnerable
	fgets(str1, sizeof(str1), stdin);
	fgets(str2, sizeof(str2), stdin);
	ret = strcmp(str1,str2);
	printf("%d\n", ret);
	return 0;
}