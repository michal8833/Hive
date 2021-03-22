#include <windows.h>
#include <cstdio>
#include <tchar.h>
#include <ctime>
#include <string>
#include "Flowerbed.h"
#include "defs.h"
#pragma comment(lib, "user32.lib")
#pragma warning(disable:4996)

using namespace std;

void parseArgs(int argc, char *argv[], int &flowerbedsNumber);
void mapping(HANDLE hMapFile, int flowerbedsNumber, SYSTEM_INFO si, Flowerbed** flowerbedsInfo, int** hiveInfo, int** bees);
bool newBee(char *arg, int* bees);
void error(std::string functionName);

int main(int argc, char* argv[]) {

    SYSTEM_INFO si;
    HANDLE hMapFile;
    Flowerbed* flowerbedsInfo;
    int* hiveInfo;
    int* bees;
    int flowerbedsNumber;

    parseArgs(argc, argv, flowerbedsNumber);

    GetSystemInfo(&si);

    srand((unsigned int)time(nullptr));

    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        fileMapping);
    
    if (hMapFile == nullptr)
        error("OpenFileMapping");

    mapping(hMapFile, flowerbedsNumber, si, &flowerbedsInfo, &hiveInfo, &bees);

    HANDLE bees_in_hive = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, TEXT("bees_in_hive"));

    while (true) {
        Sleep(rand() % 100 + 100);
        if (WaitForSingleObject(bees_in_hive, 1) == WAIT_TIMEOUT)
            continue;

        newBee(argv[1], bees);
    }



    UnmapViewOfFile(flowerbedsInfo);
    UnmapViewOfFile(hiveInfo);
    UnmapViewOfFile(bees);
    CloseHandle(hMapFile);

    return 0;
}

void parseArgs(int argc, char *argv[], int &flowerbedsNumber) {
    char *endptr;

    auto errorFunction = [programName = argv[0]]() {
        printf("Usage: %s <flowerbeds_number> <hive_capacity> <period>\n", programName);
        exit(1);
    };

    if(argc != 2)
        errorFunction();

    flowerbedsNumber = strtol(argv[1], &endptr, 0);
    if( !((*argv[1] != '\0') && (*endptr == '\0')) )
        errorFunction();
}

void mapping(HANDLE hMapFile, int flowerbedsNumber, SYSTEM_INFO si, Flowerbed** flowerbedsInfo, int** hiveInfo, int** bees) {
    *flowerbedsInfo = (Flowerbed*)MapViewOfFile(hMapFile, // flowerbedsInfo is a flowerbed array
                                                FILE_MAP_ALL_ACCESS,
                                                0,
                                                0,
                                                flowerbedsNumber * sizeof(Flowerbed));

    *hiveInfo = (int*)MapViewOfFile(hMapFile, // hiveInfo is an array consisting of 3 int values: hiveInfo[0]-the amount of honey in the hive, hiveInfo[1]-the number of bees in the hive, hiveInfo[2]-number of bees outside the hive
                                    FILE_MAP_ALL_ACCESS,
                                    0,
                                    si.dwAllocationGranularity,
                                    3 * sizeof(int));

    *bees = (int*)MapViewOfFile(hMapFile, //bees is an array of bee.exe processes IDs
                                FILE_MAP_ALL_ACCESS,
                                0,
                                2 * si.dwAllocationGranularity,
                                BEES_MAX * sizeof(int));

    if (*flowerbedsInfo == nullptr || *hiveInfo == nullptr || *bees == nullptr) {
        CloseHandle(hMapFile);
        error("MapViewOfFile");
    }
}

bool newBee(char *arg, int* bees) {
    static int beesNumber = 0;

    STARTUPINFO Si;
    PROCESS_INFORMATION Pi;

    ZeroMemory(&Si, sizeof(Si));
    Si.cb = sizeof(Si);
    ZeroMemory(&Pi, sizeof(Pi));

    char buffer[50] = "bee.exe";
    sprintf(buffer, "bee.exe %s", arg);

    printf("A bee is born.\n");

    if (!CreateProcess(nullptr,
                       buffer,
                       nullptr,
                       nullptr,
                       FALSE,
                       0,
                       nullptr,
                       nullptr,
                       &Si,
                       &Pi)
            )
    {
        error("CreateProcess");
    }

    if (beesNumber < BEES_MAX)
        bees[beesNumber++] = GetProcessId(Pi.hProcess);

    CloseHandle(Pi.hProcess);
    CloseHandle(Pi.hThread);

    return true;
}

void error(std::string functionName) {
    printf("Queen: Function %s() error (code: %ld).", functionName.c_str(), GetLastError());
    exit(1);
}
