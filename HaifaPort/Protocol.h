#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <time.h>

#define MAX_STRING 50
#define BUFFER 100
#define MinVessels 2
#define MaxVessels 50
#define MAX_SLEEP_TIME 3000 //3 sec.
#define MIN_SLEEP_TIME 5 //5 millisec.
#define EPMUTEX "epmutex"

extern HANDLE EPMutex;

void ExclusivePrint(char* PB);
void printTime();
int Random(int max, int min);
void ClosingMutex();
void NumberValidation(int numberofships);
void EilatResponseValidation(int responsecode);
void AllocateMemoryForThreads(HANDLE** vessels, int** ids, int size);
void AllocateMemoryForSemaphores(HANDLE** semaphores, int size);
void PrintWithTimeStamp(char* str);
