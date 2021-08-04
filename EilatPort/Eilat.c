#include "Quay.h"


DWORD read, written;
CHAR buffer[MAX_STRING];
HANDLE WriteHandle;
//Mutexes for Global veriables.
HANDLE ADTMutex;
//Barrier Mutex.
HANDLE EnterBarrierMutex;
//Barrier semaphore.
HANDLE barrierSem;
HANDLE *CraneSem;
HANDLE *VesSem;

//Global variables
int numOfVesselsInBarrier = 0;
int numOfVesselsInADT = 0;
int numOfCranesGlobal = 0;
int numOfShipsGlobal = 0;
int flag = 0;

// array of vesselObj to hold the ship's id and weight.
//will be initialize by the number of cranes.
// each index in the array will represents a Crane by the Cranes ID-1.
VesselInfo* vesselObjArr;

// Vessels Thread's function.
DWORD WINAPI StartEilat(PVOID Param);
// Cranes Thread's function.
DWORD WINAPI StartCrane(PVOID Param);

//Initializes Global variables and mutexes,semaphores.
void initGlobalData();

//Barrier
void EnterBarrier(int vesID);

//Enter ADT.
void EnterADT(int vesID);

//Exit ADT.
int ExitADT(int vesID);


/*Function name: main.
Description: The Main Process of Eilat Port, responsable for reciveing
the vesseles from Haifa Port and creating each of them thread.
create the cranes threads, and at the end releaseing all handels for them
and closing the process.
Input: none.
Output: none.
Algorithm: recives from Haifa port the number of Vessels, and approve\refuse the transfer.
if approve: recive each Ship and Start each vessels a thread.
Creating the cranes, each Crane will be in the index -1 of it's ID.
the ship threads also will be in index -1 of its ID.
.*/
void main(VOID)
{
	HANDLE ReadHandle;
	HANDLE* vessels;
	HANDLE* cranes;
	int* cranesIds;
	int* vesselIds;
	ReadHandle = GetStdHandle(STD_INPUT_HANDLE);
	WriteHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	char printBuffer[SIZE];
	EPMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "epmutex");


	/* now read from the pipe */
	if (!ReadFile(ReadHandle, buffer, MAX_STRING, &read, NULL))
	{

		PrintWithTimeStamp("Error reading from pipe.\n");
	}

	numOfShipsGlobal = atoi(buffer);

	sprintf(printBuffer, "Haifa requests permission to transfer %d Vessels.\n", numOfShipsGlobal);
	PrintWithTimeStamp(printBuffer);

	//Check if the Vessels number is prime and sends Eilat response.
	int res = isPrime(numOfShipsGlobal);
	_itoa(res, buffer, 10);
	if (res == 0)
	{
		if (!WriteFile(WriteHandle, buffer, MAX_STRING, &written, NULL))
		{
			PrintWithTimeStamp("Error writing to pipe.\n");
		}
		else
		{
			PrintWithTimeStamp("Approve the transfer request.\n");
			Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
		}
	}
	else
	{
		if (!WriteFile(WriteHandle, buffer, MAX_STRING, &written, NULL))
		{
			PrintWithTimeStamp("Error writing to pipe.\n");
		}
		else
		{
			PrintWithTimeStamp("Deny the transfer request.\n");
			Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
		}
	}

	
	int index;

	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	initGlobalData();
	sprintf(printBuffer, "Number of Cranes: %d.\n", numOfCranesGlobal);
	PrintWithTimeStamp(printBuffer);

	//Creating Dynamic array of cranes by the calculated number of cranes.
	AllocateMemoryForThreads(&cranes, &cranesIds, numOfCranesGlobal);
	
	
	for (int i = 0; i < numOfCranesGlobal; i++)
	{
		cranesIds[i] = (i + 1);
		cranes[i] = CreateThread(NULL, 0, StartCrane, cranesIds[i], NULL, &cranesIds[i]);
		sprintf(printBuffer, "Crane %d Ready for work.\n", i + 1);
		PrintWithTimeStamp(printBuffer);
	}

	//Creating Dynamic array of vessels by the given number of ships.
	AllocateMemoryForThreads(&vessels, &vesselIds, numOfShipsGlobal);

	for (int i = 0; i < numOfShipsGlobal; i++)
	{
		if (!(ReadFile(ReadHandle, buffer, MAX_STRING, &read, NULL)))
		{
			PrintWithTimeStamp("Error reading from pipe.\n");
			exit(1);
		}
		else
		{
			vesselIds[i] = atoi(buffer);
			index = vesselIds[i] - 1;
			vessels[index] = CreateThread(NULL, 0, StartEilat, vesselIds[i], NULL, &vesselIds[i]);
		}
	}

	WaitForMultipleObjects(numOfShipsGlobal, vessels, TRUE, INFINITE);

	//Close the Handles.
	for (int i = 0; i < numOfShipsGlobal; i++)
	{
		CloseHandle(vessels[i]);
	}
	for (int i = 0; i < numOfCranesGlobal; i++)
	{
		CloseHandle(CraneSem[i]);
		sprintf(printBuffer, "Crane %d has been shutdown.\n", i + 1);
		PrintWithTimeStamp(printBuffer);

	}
	CloseHandle(barrierSem);
	CloseHandle(EnterBarrierMutex);
	free(vessels);
	free(vesselIds);
	free(cranes);

	sprintf(printBuffer, "All Vessel Threads are done.\n");
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	sprintf(printBuffer, "Exiting.\n");
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	CloseHandle(ReadHandle);
	CloseHandle(WriteHandle);
	CloseHandle(EPMutex);
}

/*Function name: StartCrane.
Description: The Thread function.
Input: PVOID Param (CraneID);
Output: none.
Algorithm: each crane starts in WAIT state, untill a ship will release it to work.
and each crane will work few time (depends on the number of vessels and cranes)
.*/
DWORD WINAPI StartCrane(PVOID Param)
{
	int craneID = (int)Param;
	WaitForSingleObject(CraneSem[craneID - 1], INFINITE);
	for (int i = 0; i < (numOfShipsGlobal) / (numOfCranesGlobal); i++)
	{
		CraneWork(craneID, &vesselObjArr, &VesSem, &CraneSem);
	}
	return 0;
}


/*Function name: StartEilat.
Description: The Thread fucntion. each ship starts here, returns from the unloading and sail back to Haifa.
Input: PVOID Param. (vessel ID)
Output: none.
Algorithm: the ship will enter the Barrier through the function, and checking if the same ship exited from the ADT
if the same ship came back from the ADT, we send the ship back to haifa.
if not the same ship came back from the ADT, send an error message and exit the program.*/
DWORD WINAPI StartEilat(PVOID Param)
{
	WriteHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	char printBuffer[SIZE];
	int vesselID = (int)Param;
	sprintf(printBuffer, "Vessel %d - exiting Canal: Med Sea ==> Red Sea.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	sprintf(printBuffer, "Vessel %d - docking at the port.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));

	EnterBarrier(vesselID);
	EnterADT(vesselID);
	int returnVesID = ExitADT(vesselID);

	_itoa(vesselID, buffer, 10);

	if (returnVesID != vesselID)
	{
		sprintf(printBuffer, "Error, not the same ship came back from ADT.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	else if (!WriteFile(WriteHandle, buffer, MAX_STRING, &written, NULL))
	{
		sprintf(printBuffer, "Error writing to pipe\n");
		PrintWithTimeStamp(printBuffer);
	}
	sprintf(printBuffer, "Vessel %d - entering Canal: Red Sea ==> Med Sea.\n", vesselID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	return 0;
}

/*Function name: EnterBarrier.
Description: Each ship is enting the Barrier.
Input: int vesID.
Output: none.
Algorithm: taking mutex for using gloval variable
increment the number of vessels in Barrier
checking the conditions to enter the ADT
if the ship cant enter the ADT, the ship in WAIT state
if the ship can enter the ship will release M-1 ships to enter the ADT.*/
void EnterBarrier(int vesID)
{
	char printBuffer[SIZE];
	WaitForSingleObject(EnterBarrierMutex, INFINITE);
	sprintf(printBuffer, "Vessel %d - entered the Barrier.\n", vesID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	numOfVesselsInBarrier++;

	if (numOfVesselsInBarrier < numOfCranesGlobal || numOfVesselsInADT != 0)
	{
		if (!ReleaseMutex(EnterBarrierMutex))
		{
			sprintf(printBuffer, "EnterBarrier Unexpected error on mutex %d release.\n", vesID);
			PrintWithTimeStamp(printBuffer);
		}

		sprintf(printBuffer, "Vessle %d - waiting to enter the ADT.\n", vesID);
		PrintWithTimeStamp(printBuffer);
		Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
		WaitForSingleObject(barrierSem, INFINITE);
	}
	else
	{
		for (int i = 0; i < numOfCranesGlobal - 1; i++)
		{
			ReleaseShips(&barrierSem);
		}

		if (!ReleaseMutex(EnterBarrierMutex))
		{
			sprintf(printBuffer, "EnterBarrier Unexpected error on mutex %d release.\n", vesID);
			PrintWithTimeStamp(printBuffer);
		}
	}
}


/*Function name: EnterADT.
Description: Each vessel enter the ADT.
Input: int vessel ID.
Output: none.
Algorithm: taking mutex for using global variable
updating the number of ships in the ADT and the Barrier.
searching for the first empty carne, and set the ID in the vesselObjArr as the vessel's ID
and moving forawrd to UnloadingQuay.
.*/
void EnterADT(int vesID)
{
	WaitForSingleObject(ADTMutex, INFINITE);
	numOfVesselsInBarrier--;
	numOfVesselsInADT++;
	char printBuffer[SIZE];
	int index = 0;
	for (index = 0; index < numOfCranesGlobal; index++)
	{
		if (vesselObjArr[index].id == -1)
		{
			sprintf(printBuffer, "Vessel %d - inside ADT.\n", vesID);
			PrintWithTimeStamp(printBuffer);
			vesselObjArr[index].id = vesID;
			sprintf(printBuffer, "Crane %d - currently unloading Vessel %d.\n", index + 1, vesID);
			PrintWithTimeStamp(printBuffer);
			Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
			break;
		}
	}
	if (!ReleaseMutex(ADTMutex))
	{
		sprintf(printBuffer, "EnterBarrier Unexpected error on mutex release.\n");
		PrintWithTimeStamp(printBuffer);
	}
	UnloadingQuay(vesID, index, &vesselObjArr, &VesSem, &CraneSem);
}



/*Function name: ExitADT.
Description: Each vessel leaves the ADT. the last ship to exit will release the M ships in the Barrier.
Input: int vessel ID.
Output: returns the vesselID - to cheack the same Ship returned after all the work.
Algorithm: flag - counting the ships that leave the ADT, if flag%numOfCranesGlobal == 0, means its the last ship to leave and will release the M ships.
and change all the IDs in the vesseObjArr to -1 (symbol as empty)
and decrease the numOfVesselsInADT by 1 (each ships)
mutex - for using global variable.*/
int ExitADT(int vesID)
{
	char printBuffer[SIZE];
	sprintf(printBuffer, "Vessel %d - leaving the ADT.\n", vesID);
	PrintWithTimeStamp(printBuffer);
	Sleep(Random(MAX_SLEEP_TIME, MIN_SLEEP_TIME));
	WaitForSingleObject(ADTMutex, INFINITE);
	flag++;
	if (numOfVesselsInBarrier >= numOfCranesGlobal && flag % numOfCranesGlobal == 0)
	{
		for (int i = 0; i < numOfCranesGlobal; i++)
		{
			vesselObjArr[i].id = -1;
			ReleaseShips(&barrierSem);
		}
	}

	numOfVesselsInADT--;
	if (!ReleaseMutex(ADTMutex))
	{
		sprintf(printBuffer, "EnterBarrier Unexpected error on mutex release.\n");
		PrintWithTimeStamp(printBuffer);
	}
	return vesID;
}


/*Function name: initGolbalData.
Description: the function initializes the global mutexs for the Barriar,
and the semaphores for each Crane and vessel.
Input: int the number of vesseles and cranes.
Output: none.
Algorithm: create mutexs and check the creation,same thing for semaphores.*/
void initGlobalData()
{
	char printBuffer[SIZE];
	numOfCranesGlobal = RandomNumOfCranes(MinVessels, (numOfShipsGlobal - 1), numOfShipsGlobal);
	AllocateMemoryForMutex(&ADTMutex, "ADTMutex");
	AllocateMemoryForMutex(&EnterBarrierMutex, "EnterBarrierMutex");
	AllocateMemoryForSemaphores(&CraneSem, "Cranes Semaphores", numOfCranesGlobal);
	AllocateMemoryForSemaphores(&VesSem, "Vessels Semaphores", numOfShipsGlobal);
	barrierSem = CreateSemaphore(NULL, 0, numOfCranesGlobal, NULL);
	vesselObjArr = (VesselInfo*)malloc(numOfCranesGlobal * sizeof(VesselInfo));
	if (!barrierSem)
	{
		sprintf(printBuffer, "Error In Allocate Memory for Barrier Semaphore.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}
	if (!vesselObjArr)
	{
		sprintf(printBuffer, "Error In Allocate Memory VesselObjArray.\n");
		PrintWithTimeStamp(printBuffer);
		exit(1);
	}

	for (int i = 0; i < numOfCranesGlobal; i++)
	{
		vesselObjArr[i].id = -1;
	}
}
