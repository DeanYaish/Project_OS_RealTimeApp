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

/*Function name: RandomNumOfCranes.
Description: Generate random values between lower and upper and divided by numOfShips.
Input: int lower,int upper, int numOfShips (the divided by).
Output: random int between values.
Algorithm: uses rand to generate the number.*/
int RandomNumOfCranes(int lowerlimit, int upperlimit, int numOfShips)
{
	srand(time(0));
	int num;
	while (TRUE)
	{
		num = (rand() % (upperlimit - lowerlimit + 1)) + lowerlimit;
		if (numOfShips % num == 0)
		{
			return num;
		}
	}
}


/*Function name: ReleaseShips.
Description: release ship to continue
Input: none.
Output: none.
Algorithm: none.*/
void ReleaseShips(HANDLE** barrier)
{
	if (!ReleaseSemaphore(*barrier, 1, NULL))
	{
		printTime();
		fprintf(stderr, "Error: Release Ships sem.V().\n");
	}
}


/*Function name: CraneWork.
Description: The crane unload the quay from the ships.
Input: int craneID;
Output: none.
Algorithm: each Crane will unload the quay of the spacific ship (using vesselObjArr)
after finishing unloading, the crane will release the spacific ship and will be in WAIT state.
.*/
void CraneWork(int craneID, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems)
{
	char printBuffer[SIZE];
	sprintf(printBuffer, "Eilat Port: Crane %d finished unloading %d tons.\n", craneID, (*info)[craneID - 1].weight);
	printTime();
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	if (!ReleaseSemaphore((*vesselSems)[((*info)[craneID - 1].id) - 1], 1, NULL))
	{
		fprintf(stderr, "Error: release sem.V() %d \n", (*info)[craneID - 1].id);
	}

	sprintf(printBuffer, "Eilat Port Crane %d: Vessel %d, we are done unloading, please exit the dock.\n", craneID, (*info)[craneID - 1].id);
	printTime();
	ExclusivePrint(printBuffer);
	WaitForSingleObject((*craneSems)[craneID - 1], INFINITE);
}

/*Function name: UnloadingQuay.
Description: Each vessel unloading the quay from itself.
Input: int vessel ID, int index,
Output: none.
Algorithm: generate random number of weight of quay.
index represents the index of the spacific carne(the first carne found empty).
release the spacific carne to work, and than WAIT until the carne done.*/
void UnloadingQuay(int vesID, int index, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems)
{
	char printBuffer[SIZE];
	int weight = Random(MAX_WEIGHT, MIN_WEIGHT);
	sprintf(printBuffer, "Vessel %d has %d Tons to Unload.\n", vesID, weight);
	printTime();
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	(*info)[index].weight = weight;

	if (!ReleaseSemaphore((*craneSems)[index], 1, NULL))
	{
		printTime();
		fprintf(stderr, "UnloadingQuay:: error %d sem.V()\n", vesID);
	}
	WaitForSingleObject((*vesselSems)[vesID - 1], INFINITE);
	sprintf(printBuffer, "Vessel %d is now empty!\n", vesID);
	printTime();
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
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
	int hours, minutes, seconds;
	time_t now;
	time(&now);
	struct tm* local = localtime(&now);
	hours = local->tm_hour;
	minutes = local->tm_min;
	seconds = local->tm_sec;
	fprintf(stderr, "[%02d:%02d:%02d]", hours, minutes, seconds);
	fprintf(stderr, "%s", PB);
	if (!ReleaseMutex(EPMutex))
	{
		printf("Eilat Port: Error EPMutex release\n");
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

void AllocateMemoryForSemaphores(HANDLE** semaphores,char* errorMsg ,int size)
{
	*semaphores = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!semaphores)
	{
		printTime();
		fprintf(stderr, "Eilat Port: Error In Allocate Memory for %s array\n", errorMsg);
		exit(1);
	}

	for (int i = 0; i < size; i++)
	{
		(*semaphores)[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (!(*semaphores)[i])
		{
			printTime();
			fprintf(stderr, "Eilat Port: Error In Allocate Memory for %s in index %d \n", errorMsg ,i + 1);
			exit(1);
		}
	}
}

void AllocateMemoryForMutex(HANDLE** mutex, char* errorMsg)
{
	*mutex = CreateMutex(NULL, FALSE, NULL);
	if (!*mutex)
	{
		printTime();
		fprintf(stderr, "Eilat Port: Error In Allocate Memory for %s\n", errorMsg);
		exit(1);
	}
}

