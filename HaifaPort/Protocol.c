#include "Protocol.h"

//Mutex for exclusive printing.
HANDLE EPMutex;


/*Function name: ExclusivePrint.
Description: the function is responsable that only one
print can happen at a time, if 2 threads want to print simultanusly
the first to grab the mutex will print.
for all major prints.
Input: PB- buffer that the prints are written to.
Output: none..*/

/*Function name: printTime.
Description: the function creates the time stamp
for all major prints.
Input: none.
Output: none.
Algorithm: uses time.h library to get current time
from the computer by: hours,minutes,secondes.*/
void PrintWithTimeStamp(char* str)
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
	fprintf(stderr, " Haifa Port: %s", str);
	if (!ReleaseMutex(EPMutex))
	{
		fprintf(stderr, "Haifa Port: Error EPMutex release.\n");
	}

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

void ClosingMutex() {
	CloseHandle(EPMutex);
}

void NumberValidation(int numberofships) {
	char printBuffer[BUFFER];
	if (numberofships < MinVessels || numberofships > MaxVessels)
	{
		sprintf(printBuffer,"Invalid vessles number! Please enter a valid number (between 2-50)!\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	sprintf(printBuffer,"Number of Vessels entered: %d Vessels.\n", numberofships);
	PrintWithTimeStamp(printBuffer);
}

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
