#pragma once
#include "Quay.h"

HANDLE EPMutex;

/*Function name: isPrime.
Description: check if the given number is prime.
Input: int num.
Output: 0 if the number is not prime, 1 if the number is prime.
Algorithm: .*/
int isPrime(int n)
{
	for (int i = 2; i <= n / 2; i++)
	{
		if (n % i == 0)
		{
			return 0;
		}
	}
	return 1;
}


/*Function name: printTime.
Description: the function creates the time stamp
for all major prints.
Input: none.
Output: none.
Algorithm: uses time.h library to get current time
from the computer by: hours,minutes,secondes.*/
void printTime()
{
	int hours, minutes, seconds;
	time_t now;
	time(&now);
	struct tm* local = localtime(&now);
	hours = local->tm_hour;
	minutes = local->tm_min;
	seconds = local->tm_sec;
	printf("[%02d:%02d:%02d]", hours, minutes, seconds);
}


/*Function name: Random.
Description: Generate random values between min and max(for sleep time).
Input: int max,int min.
Output: random int between values.
Algorithm: uses rand to generate the number.*/
int Random(int max, int min)
{
	srand(time(0));
	return ((rand() % (max - min + 1)) + min);
}

/*Function name: ExclusivePrint.
Description: the function is responsable that only one
print can happen at a time, if 2 threads want to print simultanusly
the first to grab the mutex will print.
for all major prints.
Input: PB- buffer that the prints are written to.
Output: none..*/
void ExclusivePrint(char* PB)
{
	WaitForSingleObject(EPMutex, INFINITE);
	printTime();
	fprintf(stderr, "%s", PB);
	if (!ReleaseMutex(EPMutex))
	{
		printf("Eilat Port: Error EPMutex releas\n");
	}
}

void AllocateMemoryForThreads(HANDLE** cranes, int** ids, int size)
{
	*cranes = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!cranes)
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error In Allocate Memory for vessel array\n");
		exit(1);
	}
	*ids = (int*)malloc(sizeof(int) * size);
	if (!ids)
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error In Allocate Memory for vessel array\n");
		exit(1);
	}

}