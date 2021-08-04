#include "Protocol.h"

//Mutex for controling vessel transfer
HANDLE mutex;
//write pipe from Haifa to Eliat.
HANDLE WriterToEilat;
//Dynamic array for semaphores
HANDLE *semaphores = NULL;
//Global variables
DWORD read, written;

//Vessel creation threads function
DWORD WINAPI initVessels(PVOID Param);

//Send vessels to eilat
void SendVesselToEilat(int vesselID);

//init mutex and semaphores.
void initControllers(int vesselCount);


/*Function name: main.
Description: responsible for initialzing haifa and eilat processes as well as the pipes to connect them. 
initialzing all the controllers (mutex + semaphores) which are responsible for passing information between the processes.
in charge of the creation of all the vessel threads and handling errors.
Input:number of vessels from cmd.
Output:printing information about the vessels as well as the progression of the program.*/
int main(int argc, char* argv[])
{
	char command[MAXSTRING];
	char buffer[MAXSTRING + 1] = "";
	char printBuffer[BUFFER];
	int eilatResponseCode;
	int* vesID = NULL;

	HANDLE ReaderToEilat; //read pipe from Haifa to Eliat.
	HANDLE ReaderToHaifa, WriterToHaifa; //write and read pipes from Eliat to Haifa.	
	HANDLE* vessels = NULL; //Pointer for array of vessels.
	/*init data for process*/
	STARTUPINFO si; 
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL,TRUE };
	ZeroMemory(&pi, sizeof(pi));
	TCHAR ProcessName[256];
	size_t convertedChars = 0;

	PMutex = CreateMutex(NULL, FALSE, PMUTEX); //creating mutex for handling printing

	// checking if there is an input from the user
	if (argc != 2)
	{
		PrintWithTimeStamp("Invalid argument count, please enter a number between 2 to 50!");
		exit(1);
	}
	int vesselCount = atoi(argv[1]);

	//Check validation of entered number of vessels.
	NumberValidation(vesselCount);
	
	/* Create the pipe from Haifa to Eliat */
	if (!CreatePipe(&ReaderToEilat, &WriterToEilat, &sa, 0)) {
		PrintWithTimeStamp("Create Pipe Failed.\n");
		return 1;
	}

	/* Create the pipe from Eliat to Haifa */
	if (!CreatePipe(&ReaderToHaifa, &WriterToHaifa, &sa, 0)) {
		PrintWithTimeStamp("Create Pipe Failed.\n");
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
	mbstowcs_s(&convertedChars, ProcessName, MAXSTRING, command, _TRUNCATE);

	/* create child process (eilat) */
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
		PrintWithTimeStamp("Process Creation Failed.\n");
		return -1;
	}
	//Close unused ends of pipes.
	CloseHandle(ReaderToEilat);
	CloseHandle(WriterToHaifa);

	/*haifa sending number of vessels to eilat process through the pipe*/
	if (!WriteFile(WriterToEilat, argv[1], MAXSTRING, &written, NULL))
	{
		sprintf(printBuffer, "Error writing to pipe.\n");
		PrintWithTimeStamp(printBuffer);
	}
	sprintf(printBuffer, "Sending transfer request to Eilat Port.\n");
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));

	/*haifa receiving eilat response from eilat*/
	if (!ReadFile(ReaderToHaifa, buffer, MAXSTRING, &read, NULL))
	{
		sprintf(printBuffer, "Error reading from pipe.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}

	/*Validate response code*/
	eilatResponseCode = atoi(buffer); //convert code from char to int
	EilatResponseValidation(eilatResponseCode);

	initControllers(vesselCount);
	AllocateMemoryForThreads(&vessels, &vesID, vesselCount);

	/*Create threads for vessels.*/
	for (int i = 0; i < vesselCount; i++)
	{
		vesID[i] = i + 1;
		vessels[i] = CreateThread(NULL, 0, initVessels, vesID[i], NULL, &vesID[i]);
		if (vessels[i] == NULL)
		{
			PrintWithTimeStamp("Error happend while creating vessels threads.\n");
			exit(1);
		}
	}

	/*handle vessels that returned from eilat*/
	for (int i = 0; i < vesselCount; i++)
	{
		if (ReadFile(ReaderToHaifa, buffer, MAXSTRING, &read, NULL))
		{
			if (!ReleaseSemaphore(semaphores[atoi(buffer) - 1], 1, NULL))
			{
				sprintf(printBuffer, "Semaphore release sem[%d] failed.\n", atoi(buffer));
				PrintWithTimeStamp(printBuffer);

			}
			else
			{
				sprintf(printBuffer, "Vessel %d - exiting Canal: Red Sea ==> Med Sea.\n", atoi(buffer));
				PrintWithTimeStamp(printBuffer);
				Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));
			}
			sprintf(printBuffer, "Vessel %d - done sailing.\n", atoi(buffer));
			PrintWithTimeStamp(printBuffer);
			Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));
		}
		else
		{
			sprintf(printBuffer,"Error reading from pipe.\n");
			PrintWithTimeStamp(printBuffer);
		}
	}

	/*wait for vessels to return and closing up*/
	WaitForMultipleObjects(vesselCount, vessels, TRUE, INFINITE);

	CloseHandle(mutex);
	CloseHandle(WriterToEilat);
	CloseHandle(ReaderToHaifa);

	for (int i = 0; i < vesselCount; i++)
	{
		CloseHandle(vessels[i]);
		CloseHandle(semaphores[i]);
	}

	PrintWithTimeStamp("All Vessel Threads are Done.\n");
	Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));

	WaitForSingleObject(pi.hProcess, INFINITE); 	//wait for eilat process to end
	
	PrintWithTimeStamp("Exiting.\n");
	CloseHandle(PMutex);
	free(semaphores);
	free(vessels);
	free(vesID);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	system("pause");
	return 0;
}

/*Function name: initVessels.
Description: thread function for creating vessels and responsible for sending the vessel to eilat.
Input: Param - ID for vessel.
Output: none.*/
DWORD WINAPI initVessels(PVOID Param)
{
	char printBuffer[MAXSTRING];
	int vesselID = (int)Param;
	sprintf(printBuffer, "Vessel %d - started sailing.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));
	SendVesselToEilat(vesselID);
	return 0;
}

/*Function name: SendVesselToEilat.
Description: sending a vessel through the canal and waiting for it to return.
Input: vesselID - ID for vessel.
Output: none.*/
void SendVesselToEilat(int vesselID)
{
	char printBuffer[MAXSTRING];
	char vesID[MAXSTRING];
	_itoa(vesselID, vesID, 10);
	sprintf(printBuffer, "Vessel %d - entring Canal: Med Sea ==> Red Sea.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAXSLEEPTIME, MINSLEEPTIME));
	WaitForSingleObject(mutex, INFINITE);
	if (!WriteFile(WriterToEilat, vesID, MAXSTRING, &written, NULL))
	{
		sprintf(printBuffer, "Error writing to pipe, Vessel %d did problems!\n", vesselID);
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	if (!ReleaseMutex(mutex))
	{
		sprintf(printBuffer, "Unexpected error mutex.V().\n");
		PrintWithTimeStamp(printBuffer);
	}
	WaitForSingleObject(semaphores[vesselID - 1], INFINITE);
}

/*Function name: initControllers.
Description: initialize mutex for the canal and semaphores for each vessel.
Input: numOfVessels - quantity of vessels.
Output: none.*/
void initControllers(int numOfVessels)
{
	char printBuffer[MAXSTRING];
	mutex = CreateMutex(NULL, FALSE, NULL); 	//Creating Canal Mutex.
	if (mutex == NULL)
	{
		sprintf(printBuffer, "Mutex creation Failed.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	
	AllocateMemoryForSemaphores(&semaphores, numOfVessels); 	//Creating semaphore for each Vessel.

}