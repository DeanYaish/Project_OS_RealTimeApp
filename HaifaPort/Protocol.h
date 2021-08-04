#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#define MAXSTRING 50
#define BUFFER 100
#define MINNUMOFVESSELS 2
#define MAXNUMOFVESSELS 50
#define MAXSLEEPTIME 3000 
#define MINSLEEPTIME 5 
#define PMUTEX "pmutex"

extern HANDLE PMutex;

void PrintWithTimeStamp(char* str);
int Random(int max, int min);
void NumberValidation(int numberofships);
void EilatResponseValidation(int responsecode);
void AllocateMemoryForThreads(HANDLE** vessels, int** ids, int size);
void AllocateMemoryForSemaphores(HANDLE** semaphores, int size);

