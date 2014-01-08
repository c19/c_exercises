#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void print_introduction(){
	printf("\nto play simple simon.|观看屏幕的数字序列|");
	printf("\nwatch the screen for a sequence of digits.|仔细观看，数字只显示一秒钟！| ");
	printf("\nwatch carefully ,as the digits are only displayed for a second!|电脑将删除他们，然后提示你|");
	printf("\nto enter the same sequence.输入相同的序列");
	printf("\nwhen you do,you must put spaces between the digits, 当你这样做的时候，必须在数字之间加空格.");
	printf("\n good luck!\npress enter to play ,祝你好运，按回车进入游戏\n");
}

int gen_rand(int min, int max){
	return rand() % (max-min) + min;
}

int game_round(int level){
	int number = gen_rand(level/10,level);
	printf("%d", number);
	fflush(stdout);
	sleep(1);
	printf("%c[2K\nNow type the numbers just shown\n", 27);
	int attempt;
	scanf("%d", &attempt);
	if (attempt == number)
	{
		printf("success~!\n");
		return 0;
	}else
	{
		printf("wrong answer...\n");
		return 1;
	}
}

int main(int argc, char const *argv[])
{
	int level = 1000;
	srand(time(0));
	print_introduction();
	while (game_round(level) == 0){
		level *= 10;
		printf("into next level!\n");
	}
}