#include <stdio.h>
#include <stdlib.h>
//sort input nums
//sorry these functions only support ints,as generic programming in C would be pain in the ass
int output_array(int array[],int length){
	for (int i = 0; i < length; i++)
	{
		printf("array[%d]:%d\n",i,array[i]);
	}
}

int input_array(int array[],int length){
	for (int i = 0; i < length; i++)
	{	
		printf("input_array: %dth\n", i);
		scanf("%d",&array[i]);
	}
}

int* insert_sort(int array[],int length){
	int current,j;
	for (int i = 0; i < length; i++)
	{
		current = array[i];
		j = i - 1;
		while (0 <= j && array[j] < current){
			array[j+1] = array[j];
			j--;
		}
		array[j+1] = current;
	}
	return array;
}

int main(int argc, char const *argv[])
{
	#define size 8
	int x[size];
	input_array(x,size);
	insert_sort(x,size);
	output_array(x,size);
	return 0;
}
