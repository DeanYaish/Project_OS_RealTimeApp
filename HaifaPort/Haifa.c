#include "Protocol.h"

//Mutex for canal - Haifa to Eilat.
HANDLE mutex;
//read form pipe Haifa to Eliat.
HANDLE WriterToEilat;
//Array of semaphores - 1 semphore for each vessel.
HANDLE *semaphores = NULL;
//Global variables
DWORD read, written;

//Vessels leave Haifa Port.
DWORD WINAPI Start(PVOID Param);

//Vessels enter canal.
void GoToEilat(int vesselID);

//Initializes Global variables and mutexes,semaphores.
void initGolbalData(int numOfShips);


/*Function name: main.
Description: the main responeable for creation of the child process
and communication pipes, after that creates vessels threads if Eilat approved
transfer and whan all vessels saild they WAIT.
when the vessels starts to return, they exit WAIT and finish there voyage back
to haifa, after all vessels are back ,closing there threads and closing all handels
and waiting or child process to finish, when finished closing parent process and finish
program.
Input: reading enterd value.
Output: 0 - end of program run.*/
int main(int argc, char* argv[])
{
	char command[MAX_STRING];
	char buffer[MAX_STRING + 1] = "";
	char printBuffer[BUFFER];
	int eilatResponseCode;
	int* vesID = NULL;
	//read from pipe Haifa to Eliat.
	HANDLE ReaderToEilat;
	//Pipe from Eilat to Haifa.
	HANDLE ReaderToHaifa, WriterToHaifa;
	//Variable to hold the Array of vessles threads.
	HANDLE* vessels = NULL;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL,TRUE };
	ZeroMemory(&pi, sizeof(pi));
	TCHAR ProcessName[256];
	size_t convertedChars = 0;

	if (argc != 2)
	{
		printf("Invalid argument count, please enter a number between 2 to 50!");
		exit(1);
	}
	int numOfShips = atoi(argv[1]);
	//Check validation of entered number of vessels.
	NumberValidation(numOfShips);
	
	/* Create the pipe from Haifa to Eliat */
	if (!CreatePipe(&ReaderToEilat, &WriterToEilat, &sa, 0)) {
		fprintf(stderr, "Create Pipe Failed\n");
		return 1;
	}

	/* Create the pipe from Eliat to Haifa */
	if (!CreatePipe(&ReaderToHaifa, &WriterToHaifa, &sa, 0)) {
		fprintf(stderr, "Create Pipe Failed\n");
		return 1;
	}

	/* Establish the START_INFO structure for Eliat process */
	GetStartupInfo(&si);
	si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

	/* Redirect the standard input to the read end of the pipe */
	si.hStdOutput = WriterToHaifa;
	si.hStdInput = ReaderToEilat;
	si.dwFlags = STARTF_USESTDHANDLES;

	sprintf(command, "EilatPort.exe");
	mbstowcs_s(&convertedChars, ProcessName, MAX_STRING, command, _TRUNCATE);

	/* create the child process */
	if (!CreateProcess(NULL,
		ProcessName,
		NULL,
		NULL,
		TRUE, /* inherit handles */
		0,
		NULL,
		NULL,
		&si,
		&pi))
	{
		fprintf(stderr, "Process Creation Failed\n");
		return -1;
	}
	//Close Unused ends of pipes.
	CloseHandle(ReaderToEilat);
	CloseHandle(WriterToHaifa);

	/* Haifa now wants to write to the pipe */
	if (!WriteFile(WriterToEilat, argv[1], MAX_STRING, &written, NULL))
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error writing to pipe.\n");
	}
	sprintf(printBuffer, "Haifa Port: Sending transfer request to Eilat Port.\n");
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	/*read from the pipe */
	if (!ReadFile(ReaderToHaifa, buffer, MAX_STRING, &read, NULL))
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error reading from pipe.\n");
		exit(1);
	}
	
	eilatResponseCode = atoi(buffer); //4.1 + 3.1
	/*Validation of response code from eilat port*/
	EilatResponseValidation(eilatResponseCode);
	initGolbalData(numOfShips);
	AllocateMemoryForThreads(&vessels, &vesID, numOfShips);
	//Creating the threads for the vessles.
	for (int i = 0; i < numOfShips; i++)
	{
		vesID[i] = i + 1;
		vessels[i] = CreateThread(NULL, 0, Start, vesID[i], NULL, &vesID[i]);
		if (vessels[i] == NULL)
		{
			fprintf(stderr, "Error happend while creating vessles threads\n");
			exit(1);
		}
	}

	for (int i = 0; i < numOfShips; i++)
	{
		if (ReadFile(ReaderToHaifa, buffer, MAX_STRING, &read, NULL))
		{
			if (!ReleaseSemaphore(semaphores[atoi(buffer) - 1], 1, NULL))
			{
				fprintf(stderr, "Semaphore release sem[%d] faild\n", atoi(buffer));
			}
			else
			{
				sprintf(printBuffer, "Vessel %d - exiting Canal: Red Sea ==> Med. Sea\n", atoi(buffer));
				ExclusivePrint(printBuffer);
				Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
			}
			sprintf(printBuffer, "Vessel %d - done sailing @ Haifa Port\n", atoi(buffer));
			ExclusivePrint(printBuffer);
			Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
		}
		else
		{
			fprintf(stderr, "Haifa: Error reading from pipe.\n");
		}

	}

	//Waiting for all vessels to return.
	WaitForMultipleObjects(numOfShips, vessels, TRUE, INFINITE);

	CloseHandle(mutex);
	CloseHandle(WriterToEilat);
	CloseHandle(ReaderToHaifa);

	for (int i = 0; i < numOfShips; i++)
	{
		CloseHandle(vessels[i]);
		CloseHandle(semaphores[i]);
	}

	sprintf(printBuffer, "Haifa Port: All Vessel Threads are Done\n");
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	//Waiting for Eilat to exit.
	WaitForSingleObject(pi.hProcess, INFINITE);

	printTime();
	printf("Haifa Port: Exiting...\n");
	CloseHandle(EPMutex);
	free(semaphores);
	free(vessels);
	free(vesID);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	system("pause");
	return 0;
}

/*Function name: Start.
Description: after the thread creation for each vessel this method start
the vessel voyage from Haifa Port,also takes the mutex for the vessel that
sails and enters the canal.
Input: Param which holds the number of vesseles enterd.
Output: none - returns to main when done.
Algorithm: printing the vessel start sail indication and grab the cana mutex
that only one vessel can pass at a time.*/
DWORD WINAPI Start(PVOID Param)
{
	char printBuffer[MAX_STRING];
	int vesselID = (int)Param;
	sprintf(printBuffer, "Vessel %d - starts sailing @ Haifa Port\n", vesselID);
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	GoToEilat(vesselID);
	return 0;
}

/*Function name: GoToEilat.
Description: this function send each vessel(after his thread creation) to the canal
and write to the pipe(Haifa writes to Eilat) the vessel ID that pass in the canal.
Input: int the number of vesseles enterd.
Output: none.
Algorithm: responsable for printing the canal indication(on entrence) and writing to the pipe,also check
that the writing was good and release the mutex on the canal that the current vessel took.*/
void GoToEilat(int vesselID)
{
	char printBuffer[MAX_STRING];
	char vesID[MAX_STRING];
	_itoa(vesselID, vesID, 10);
	sprintf(printBuffer, "Vessel %d - entring Canal: Med. Sea ==> Red Sea\n", vesselID);
	ExclusivePrint(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	//mutex.P.
	WaitForSingleObject(mutex, INFINITE);
	if (!WriteFile(WriterToEilat, vesID, MAX_STRING, &written, NULL))
	{
		printTime();
		fprintf(stderr, "Haifa Port: Error writing to pipe, Vessel %d did problems!\n", vesselID);
		exit(1);
	}
	if (!ReleaseMutex(mutex))
	{
		printTime();
		fprintf(stderr, "Haifa Port: Unexpected error mutex.V()\n");
	}
	WaitForSingleObject(semaphores[vesselID - 1], INFINITE);
}

/*Function name: initGolbalData.
Description: the function initializes the mutex for the canal
and the semaphores for each vessel.
Input: int the number of vesseles enterd.
Output: none.
Algorithm: create mutex and check the creation,same thing for semaphore for each vessele.*/
void initGolbalData(int numOfShips)
{
	//Creating Canal Mutex.
	mutex = CreateMutex(NULL, FALSE, NULL);
	if (mutex == NULL)
	{
		printTime();
		fprintf(stderr, "Mutex creation Failed\n");
		exit(1);
	}
	
	//Creating Shemaphore for each Vessel.
	AllocateMemoryForSemaphores(&semaphores, numOfShips);
}