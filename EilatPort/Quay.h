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
#define EPMUTEX "epmutex"


extern HANDLE EPMutex;

//struct to hold relevent info of vessel.
struct VesselObj {
	int id;
	int weight;
};


int isPrime(int n);
void ExclusivePrint(char* PB);
void printTime();
int Random(int max, int min);
void AllocateMemoryForThreads(HANDLE** cranes, int** ids, int size);

#endif