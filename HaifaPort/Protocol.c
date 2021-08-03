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
void ExclusivePrint(char* PB)
{
	EPMutex = CreateMutex(NULL, FALSE, EPMUTEX);
	if (EPMutex)
	{
		WaitForSingleObject(EPMutex, INFINITE);
		PrintWithTimeStamp(PB);
		if (!ReleaseMutex(EPMutex))
		{
			printf("Haifa Port: Error EPMutex release\n");
		}
	}

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

void ClosingMutex() {
	CloseHandle(EPMutex);
}

void NumberValidation(int numberofships) {
	printTime();
	if (numberofships < MinVessels || numberofships > MaxVessels)
	{
		printf("Invalid vessles number! Please enter a valid number (between 2-50)!\n");
		exit(1);
	}
	printf("Number of Vessels entered: %d Vessels\n", numberofships);
}

void EilatResponseValidation(int responsecode) {
	char printBuffer[BUFFER];
	if (responsecode == 1)
	{
		sprintf(printBuffer, "Haifa Port: Eilat Denied transfer request\n");
		ExclusivePrint(printBuffer);
		exit(1);
	}
	else
	{
		sprintf(printBuffer, "Haifa Port: Eilat Approved transfer request\n");
		ExclusivePrint(printBuffer);
	}
}

void AllocateMemoryForThreads(HANDLE** vessels, int** ids, int size)
{
	*vessels = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!vessels)
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

void AllocateMemoryForSemaphores(HANDLE** semaphores, int size)
{
	*semaphores = (HANDLE*)malloc(sizeof(HANDLE) * size);
	if (!semaphores)
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error In Allocate Memory for vessel array\n");
		exit(1);
	}
	for (int i = 0; i < size; i++)
	{

		(*semaphores)[i] = CreateSemaphore(NULL, 0, 1, NULL);
		if ((*semaphores)[i] == NULL)
		{
			printTime();
			fprintf(stderr, "Creation of Semaphore %d Failed\n", i);
			exit(1);
		}
	}
}

void PrintWithTimeStamp(char* str)
{
	int hours, minutes, seconds;
	time_t now;
	time(&now);
	struct tm* local = localtime(&now);
	hours = local->tm_hour;
	minutes = local->tm_min;
	seconds = local->tm_sec;
	printf("[%02d:%02d:%02d]", hours, minutes, seconds);
	printf(": %s", str);
}