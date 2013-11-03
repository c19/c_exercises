#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/*
draw a circle with stars
*/

void set_array(char array[],int length){
	for (int i = 0; i < length; i++)
	{
		array[i] = ' ';
	}
}

int main(int argc, char const *argv[])
{
	#define size 50
	#define r (size/2)
	double pi = 3.14159265359;
	double theta;
	char sqaure[size+1][size];
	int length = size*size;
	set_array((char *)sqaure,length);
	int x,y;
	for (int i = 0; i <= 100; i++)
	{
		theta = pi/(double)50*i;
		x = r + r*cos(theta);
		y = r + r*sin(theta);
		sqaure[x][y] = '*';
	}
	for (int i = 0; i < size; i+=2)   //quick fix,for print with doesn't match height.
	{
		for (int j = 0; j < size; j++)
		{
			printf("%c",sqaure[i][j]);
		}
		printf("\n");
	}
	return 0;
}