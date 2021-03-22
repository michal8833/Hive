#include <windows.h>
#include <cstdio>
#include <tchar.h>
#include <ctime>
#include <string>
#include "Flowerbed.h"
#include "defs.h"
#pragma warning(disable:4996)
using namespace std;

void parseArgs(int argc, char *argv[], int &flowerbedsNumber, int &hiveCapacity, int &period);
void mapping(HANDLE hMapFile, int flowerbedsNumber, SYSTEM_INFO si, Flowerbed** flowerbedsInfo, int** hiveInfo, int** bees);
void killSwarm(int *bees, PROCESS_INFORMATION &Pi);
void error(std::string functionName);

int main(int argc, char *argv[]) {

    HANDLE hMapFile;
    Flowerbed* flowerbedsInfo;
    int* hiveInfo;
    int* bees;
    Flowerbed* flowerbeds;
    STARTUPINFO Si;
    PROCESS_INFORMATION Pi;
    SYSTEM_INFO si;
    int flowerbedsNumber;
    int hiveCapacity;
    int period;

    parseArgs(argc, argv, flowerbedsNumber, hiveCapacity, period);

    GetSystemInfo(&si);

    srand((unsigned int)time(nullptr));

    flowerbeds = new Flowerbed[flowerbedsNumber];

    printf("Meadow at the beginning of the program:\n");
    for (int i = 0; i < flowerbedsNumber; i++)
        printf("Flowerbed %d: %d flowers, %ld nectar units.\n", i + 1, flowerbeds[i].getFlowersNumber(), *(flowerbeds[i].getTotalNectarUnits()));
    printf("-----------------------------------------------------------------------------------------------------------\n");

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        SIZE_OF_BUF,
        fileMapping);

    if (hMapFile == nullptr)
        error("CreateFileMapping");

    mapping(hMapFile, flowerbedsNumber, si, &flowerbedsInfo, &hiveInfo, &bees);

    CopyMemory(flowerbedsInfo, flowerbeds, flowerbedsNumber * sizeof(Flowerbed));

    CreateMutex(nullptr, false, TEXT("entry_door")); // hive entry door synchronization(at a time, only one bee passes through the door)
    CreateMutex(nullptr, false, TEXT("exit_door")); // hive exit door synchronization(at a time, only one bee passes through the door)
    CreateSemaphore(nullptr, hiveCapacity, hiveCapacity, TEXT("bees_in_hive")); // a semaphore controlling the number of bees in the hive
    CreateMutex(nullptr, false, TEXT("honey_in_hive")); // honey in hive access synchronization
    CreateMutex(nullptr, false, TEXT("checking_availability_of_flower")); // flowers access synchronization
    CreateMutex(nullptr, false, TEXT("number_of_bees_in_and_out_of_hive")); // hiveInfo[1] i hiveInfo[2] access synchronization

    ZeroMemory(&Si, sizeof(Si));
    Si.cb = sizeof(Si);
    ZeroMemory(&Pi, sizeof(Pi));

    char buffer[50];
    sprintf(buffer, "queen.exe %s", argv[1]);

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

    Sleep(period);

    killSwarm(bees, Pi );

    printf("-----------------------------------------------------------------------------------------------------------\n");
    printf("Meadow at the end of the program:\n");
    for (int i = 0; i < flowerbedsNumber; i++)
        printf("Flowerbed %d: %d flowers, %ld nectar units.\n", i + 1, flowerbedsInfo[i].getFlowersNumber(), *(flowerbedsInfo[i].getTotalNectarUnits()));
    printf("\nNumber of bees in the hive: %d\nNumber of bees outside the hive: %d\n", hiveInfo[1], hiveInfo[2]);
    printf("\nThe amount of honey collected in the hive: %d\n", hiveInfo[0]);


    UnmapViewOfFile(flowerbedsInfo);
    UnmapViewOfFile(hiveInfo);
    UnmapViewOfFile(bees);
    CloseHandle(hMapFile);

    return 0;
}

void parseArgs(int argc, char *argv[], int &flowerbedsNumber, int &hiveCapacity, int &period) {
    char *endptr;

    auto errorFunction = [programName = argv[0]]() {
        printf("Usage: %s <flowerbeds_number> <hive_capacity> <period>\n", programName);
        exit(1);
    };

    if(argc != 4)
        errorFunction();

    flowerbedsNumber = strtol(argv[1], &endptr, 0);
    if( !((*argv[1] != '\0') && (*endptr == '\0')) )
        errorFunction();

    hiveCapacity = strtol(argv[2], &endptr, 0);
    if( !((*argv[2] != '\0') && (*endptr == '\0')) )
        errorFunction();

    period = strtol(argv[3], &endptr, 0);
    if( !((*argv[3] != '\0') && (*endptr == '\0')) )
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
    (*hiveInfo)[0] = 0;
    (*hiveInfo)[1] = 0;
    (*hiveInfo)[2] = 0;

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

void killSwarm( int *bees, PROCESS_INFORMATION &Pi ) {
    TerminateProcess( Pi.hProcess, 0 ); // queen.exe
    CloseHandle( Pi.hProcess );
    CloseHandle( Pi.hThread );

    for (int i = 0; i < BEES_MAX; i++) {
        HANDLE process = OpenProcess( PROCESS_ALL_ACCESS, false, bees[i] );
        TerminateProcess( process, 0 ); //bee.exe
        CloseHandle( process );
    }
}

void error(std::string functionName) {
    printf("Hive: Function %s() error (code: %ld).", functionName.c_str(), GetLastError());
    exit(1);
}
