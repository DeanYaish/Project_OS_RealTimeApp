#pragma once
#include "Quay.h"

HANDLE PMutex; //printing mutex

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
Description: calc random number.
Input: max, min - upper and lower limit values.
Output: random int between values.*/
int Random(int max, int min)
{
	srand(time(0));
	return ((rand() % (max - min + 1)) + min);
}

/*Function name: GenerateAmountOfCranes.
Description: Generate random value for number of cranes between min and max value
and check if the number of vessels is dividable by that number.
Input: min,max - limits of the random number, numOfVessels - number of vessels.
Output: num - random integer.*/
int GenerateAmountOfCranes(int min, int max, int numOfVessels)
{
	srand(time(0));
	int num;
	while (1)
	{
		num = (rand() % (max - min + 1)) + min;
		if (numOfVessels % num == 0)
		{
			return num;
		}
	}
}


/*Function name: ReleaseVessels.
Description: release vessels in the barrier.
Input: barrier - pointer to array of semaphores inside the barrier
Output: none.*/
void ReleaseVessels(HANDLE** barrier)
{
	if (!ReleaseSemaphore(*barrier, 1, NULL))
	{
		PrintWithTimeStamp("Error Release Ships sem.V().");
	}
}

/*Function name: CraneUnload.
Description: The crane unload the quay from the ships.
Input: craneID - id of crane, info - pointer to struct info that contain a vessel's id and weight,
vesselSems- pointer to array of vessels semaphores, craneSems - pointer to array of cranes semaphores;
Output: print result of action.*/
void CraneUnload(int craneID, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems)
{
	char printBuffer[SIZE];
	sprintf(printBuffer, "Crane %d - finished unloading %d tons from Vessel %d.\n", craneID, (*info)[craneID - 1].weight, (*info)[craneID - 1].id);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	if (!ReleaseSemaphore((*vesselSems)[((*info)[craneID - 1].id) - 1], 1, NULL))
	{
		sprintf(printBuffer, "Error release sem.V() %d.\n", (*info)[craneID - 1].id);
		PrintWithTimeStamp(printBuffer);
	}

	sprintf(printBuffer, "Crane %d - finished unloading Vessel %d, currently exiting the Quay.\n", craneID, (*info)[craneID - 1].id);
	PrintWithTimeStamp(printBuffer);
	WaitForSingleObject((*craneSems)[craneID - 1], INFINITE);
}

/*Function name: UnloadVessel.
Description: generating random weight and responsible for unloading vessel.
Input: vesselID - id of vessel, index - of vessel in eilat port, info - pointer to struct info that contain a vessel's id and weight,
vesselSems- pointer to array of vessels semaphores, craneSems - pointer to array of cranes semaphores;
Output: print result of action.*/
void UnloadVessel(int vesselID, int index, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems)
{
	char printBuffer[SIZE];
	int weight = Random(MAX_WEIGHT, MIN_WEIGHT);
	sprintf(printBuffer, "Vessel %d - has %d Tons to unload.\n", vesselID, weight);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	(*info)[index].weight = weight;

	if (!ReleaseSemaphore((*craneSems)[index], 1, NULL))
	{
		sprintf(printBuffer, "Unloading vessel error on %d sem.V().", vesselID);
		PrintWithTimeStamp(printBuffer);
	}
	WaitForSingleObject((*vesselSems)[vesselID - 1], INFINITE);
	sprintf(printBuffer, "Vessel %d - is now empty.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
}


/*Function name: PrintWithTimeStamp.
Description: the function creates the time stamp for all major prints as well as making sure that only one thread
prints at a time using the pmutex.
Input: str - the message to print.
Output: prints the message to the cmd.*/
void PrintWithTimeStamp(char* str)
{
	WaitForSingleObject(PMutex, INFINITE);
	int hours, minutes, seconds;
	time_t currentTime;
	time(&currentTime);
	struct tm* timeStamp = localtime(&currentTime);
	hours = timeStamp->tm_hour;
	minutes = timeStamp->tm_min;
	seconds = timeStamp->tm_sec;
	fprintf(stderr, "[%02d:%02d:%02d]", hours, minutes, seconds);
	fprintf(stderr, " Eilat Port: %s", str);
	if (!ReleaseMutex(PMutex))
	{
		fprintf(stderr, "Eilat Port: Error PMutex release.\n");
	}
}

/*Function name: AllocateMemoryForThreads.
Description: dynamicly allocate memory for cranes.
Input: cranes- pointer to array of cranes, ids - pointer to array of cranes id, size - amount of cranes.
Output: print error if needed*/
void AllocateMemoryForThreads(HANDLE** cranes, int** ids, int size)
{
	char printBuffer[SIZE];
	*cranes = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!cranes)
	{
		sprintf(printBuffer, "Error In Allocate Memory for vessel array.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	*ids = (int*)malloc(sizeof(int) * size);
	if (!ids)
	{
		sprintf(printBuffer, "Error In Allocate Memory for vessel array.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
}

/*Function name: AllocateMemoryForSemaphores.
Description: dynamicly allocate memory for semaphores.
Input: semaphores- pointer to array of semaphores, errorMsg -string with error message, size - amount of semaphores.
Output: print error if needed*/
void AllocateMemoryForSemaphores(HANDLE** semaphores,char* errorMsg ,int size)
{
	char printBuffer[SIZE];
	*semaphores = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!semaphores)
	{
		sprintf(printBuffer, "Error In Allocate Memory for %s array.\n", errorMsg);
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}

	for (int i = 0; i < size; i++)
	{
		(*semaphores)[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if (!(*semaphores)[i])
		{
			sprintf(printBuffer, "Error In Allocate Memory for %s in index %d.\n", errorMsg ,i + 1);
			PrintWithTimeStamp(printBuffer);
			exit(1);
		}
	}
}

/*Function name: AllocateMemoryForMutex.
Description: dynamicly allocate memory for a mutex.
Input: mutex - pointer to mutex, errorMsg -string with error message.
Output: print error if needed*/
void AllocateMemoryForMutex(HANDLE** mutex, char* errorMsg)
{
	char printBuffer[SIZE];
	*mutex = CreateMutex(NULL, FALSE, NULL);
	if (!*mutex)
	{
		sprintf(printBuffer, "Error In Allocate Memory for %s.\n", errorMsg);
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
}

