#include "thread_library.c"
#include <stdio.h>
void print1()
{
	int i;
	for(i=1;i<=10;i++)
	{
		printf("thread1\n");
	}
}
void print2()
{
	int i;
	for(i=1;i<=10;i++)
	{
		printf("thread2\n");
	}
}
void print3()
{
	int i;
	for(i=1;i<=10;i++)
	{
		printf("thread3\n");
	}
}
int main()
{
	struct mjthread t1,t2,t3;
	thread_create(&t1,print1);
	thread_create(&t2,print2);
	thread_create(&t3,print3);
	printf("all threads done\n");
	while(1)
	{

	}
	return 0;
}