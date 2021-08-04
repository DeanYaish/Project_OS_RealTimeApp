#ifndef QUAY_H
#define QUAY_H

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#define MAX_STRING 50
#define SIZE 100
#define MaxVessels 50
#define MinVessels 2
#define MAX_SLEEP_TIME 3000
#define MIN_SLEEP_TIME 5
#define MAX_WEIGHT 50
#define MIN_WEIGHT 5


extern HANDLE EPMutex;

//struct to hold relevent info of vessel.
typedef struct VesselInfos {
	int id;
	int weight;
} VesselInfo;


int isPrime(int n);
int Random(int max, int min);
int RandomNumOfCranes(int lowerlimit, int upperlimit, int numOfShips);
void ReleaseShips(HANDLE** barrier);
void CraneWork(int craneID, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems);
void UnloadingQuay(int vesID, int index, VesselInfo** info, HANDLE** vesselSems, HANDLE** craneSems);
void AllocateMemoryForThreads(HANDLE** cranes, int** ids, int size);
void AllocateMemoryForSemaphores(HANDLE** semaphores, char* errorMsg, int size);
void AllocateMemoryForMutex(HANDLE** mutex, char* errorMsg);
void PrintWithTimeStamp(char* str);

#endif