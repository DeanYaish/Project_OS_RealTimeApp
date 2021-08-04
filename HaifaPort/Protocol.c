#include "Protocol.h"

HANDLE PMutex; //print mutex.

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
	fprintf(stderr, " Haifa Port: %s", str);
	if (!ReleaseMutex(PMutex))
	{
		fprintf(stderr, "Haifa Port: Error PMutex release.\n");
	}
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

/*Function name: NumberValidation.
Description: check if the number of vessels is between the allowed min and max values.
Input: numOfVessels - num of vessels.
Output: print response*/
void NumberValidation(int numOfVessels) {
	char printBuffer[BUFFER];

	if (numOfVessels < MINNUMOFVESSELS || numOfVessels > MAXNUMOFVESSELS)
	{
		sprintf(printBuffer,"Invalid vessles number! Please enter a valid number (between 2-50)!\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	sprintf(printBuffer,"Number of Vessels entered: %d Vessels.\n", numOfVessels);
	PrintWithTimeStamp(printBuffer);
}

/*Function name: EilatResponseValidation.
Description: check if eilat approved the number of vessels.
Input: responsecode - the response code from eilat.
Output: print response*/
void EilatResponseValidation(int responsecode) {
	char printBuffer[BUFFER];
	if (responsecode == 1)
	{
		sprintf(printBuffer, "Eilat Denied transfer request.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	else
	{
		sprintf(printBuffer, "Eilat Approved transfer request.\n");
		PrintWithTimeStamp(printBuffer);
	}
}

/*Function name: AllocateMemoryForThreads.
Description: dynamicly allocate memory for threads.
Input: vessels- pointer to array of vessels, ids - pointer to array of vessels id, size - amount of vessels.
Output: print error if needed*/
void AllocateMemoryForThreads(HANDLE** vessels, int** ids, int size)
{
	char printBuffer[BUFFER];
	*vessels = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!vessels)
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
Input: semaphores- pointer to array of semaphores, size - amount of semaphores.
Output: print error if needed*/
void AllocateMemoryForSemaphores(HANDLE** semaphores, int size)
{
	char printBuffer[BUFFER];
	*semaphores = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!semaphores)
	{
		sprintf(printBuffer, "Error In Allocate Memory for vessel array.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	for (int i = 0; i < size; i++)
	{
		(*semaphores)[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if ((*semaphores)[i] == NULL)
		{
			sprintf(printBuffer, "Creation of Semaphore %d Failed.\n", i);
			PrintWithTimeStamp(printBuffer);
			exit(1);
		}
	}
}
