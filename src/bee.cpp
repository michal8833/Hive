#include <windows.h>
#include <cstdio>
#include <ctime>
#include <unordered_map>
#include "Flowerbed.h"
#include "defs.h"
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "winmm.lib")
#pragma warning(disable:4996)

using namespace std;

void parseArgs(int argc, char *argv[], int &flowerbedsNumber);
void initializeGlobalVariables();
void mapping(HANDLE hMapFile, int flowerbedsNumber, SYSTEM_INFO si, Flowerbed** flowerbedsInfo, int** hiveInfo);
void beeFliesOutOfHive(int* hiveInfo);
void flight(int distanceToFlowerbed);
void collectingNectarFromFlowerbed(int* hiveInfo, Flowerbed* flowerbedsInfo, int flowerbedIndex);
void returnToHive(int* hiveInfo, int distanceToFlowerbed);
void error(std::string functionName);

std::unordered_map<std::string, HANDLE> sync;
struct BeeInfo {
    int goitresNumber;
    int currentPayload;
} beeInfo;

int main(int argc, char* argv[]) {

    SYSTEM_INFO si;
    HANDLE hMapFile;
    Flowerbed* flowerbedsInfo;
    int* hiveInfo;
    int flowerbedsNumber;

    parseArgs(argc, argv, flowerbedsNumber);

    GetSystemInfo(&si);

    srand((unsigned int)time(nullptr));

    initializeGlobalVariables();

    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        fileMapping);

    if (hMapFile == nullptr)
        error("OpenFileMapping");

    mapping(hMapFile, flowerbedsNumber, si, &flowerbedsInfo, &hiveInfo);

    WaitForSingleObject(sync["number_of_bees_in_and_out_of_hive"], INFINITE);
    hiveInfo[1]++; // a bee is in hive
    if (!ReleaseMutex(sync["number_of_bees_in_and_out_of_hive"]))
        error("ReleaseMutex");

    do {
        int flowerbedIndex = rand() % flowerbedsNumber; // randomly chosen flowerbed
        int distanceToFlowerbed = flowerbedsInfo[flowerbedIndex].getDistanceFromHive();

        beeFliesOutOfHive(hiveInfo);

        flight(distanceToFlowerbed); // flight to flowerbed

        collectingNectarFromFlowerbed(hiveInfo, flowerbedsInfo, flowerbedIndex);

        returnToHive(hiveInfo, distanceToFlowerbed);

    } while (true);

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

void initializeGlobalVariables() {
    sync["bees_in_hive"] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, false, TEXT("bees_in_hive"));
    sync["entry_door"] = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("entry_door"));
    sync["exit_door"] = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("exit_door"));
    sync["honey_in_hive"] = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("honey_in_hive"));
    sync["checking_availability_of_flower"] = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("checking_availability_of_flower"));
    sync["number_of_bees_in_and_out_of_hive"] = OpenMutex(MUTEX_ALL_ACCESS, false, TEXT("number_of_bees_in_and_out_of_hive"));

    beeInfo.goitresNumber = 5;
    beeInfo.currentPayload = 0;
}

void mapping(HANDLE hMapFile, int flowerbedsNumber, SYSTEM_INFO si, Flowerbed** flowerbedsInfo, int** hiveInfo) {
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

    if (*flowerbedsInfo == nullptr || *hiveInfo == nullptr) {
        CloseHandle(hMapFile);
        error("MapViewOfFile");
    }
}

void beeFliesOutOfHive(int* hiveInfo) {
    WaitForSingleObject(sync["exit_door"], INFINITE);
    Sleep(2);

    WaitForSingleObject(sync["number_of_bees_in_and_out_of_hive"], INFINITE);
    hiveInfo[1]--; // the bee comes out of the hive
    hiveInfo[2]++;
    if (!ReleaseMutex(sync["number_of_bees_in_and_out_of_hive"]))
        error("ReleaseMutex");

    if (!ReleaseMutex(sync["exit_door"]))
        error("ReleaseMutex");

    if (!ReleaseSemaphore(sync["bees_in_hive"], 1, nullptr))
        error("ReleaseSemaphore");

    // there is a 10% chance that a bee will die outside the hive
    if ((rand() % 10 + 1) == 10) {
        WaitForSingleObject(sync["number_of_bees_in_and_out_of_hive"], INFINITE);
        hiveInfo[2]--;
        if (!ReleaseMutex(sync["number_of_bees_in_and_out_of_hive"]))
            error("ReleaseMutex");

        printf("A bee dies.\n");
        exit(0);
    }
}

void flight(int distance) {
    int remainingDistance = distance;
    while (remainingDistance > 0) {
        if ((distance - remainingDistance) % 100 == 0)
            beeInfo.goitresNumber--; // a bee uses a unit of honey for every 100 meters of flight
        remainingDistance -= 5;
        Sleep(1);
    }
}

void collectingNectarFromFlowerbed(int* hiveInfo, Flowerbed* flowerbedsInfo, int flowerbedIndex) {
    int initialTime = timeGetTime();
    WaitForSingleObject(sync["checking_availability_of_flower"], INFINITE);
    bool hasMutex = true;

    for (int flower = 0; flower < flowerbedsInfo[flowerbedIndex].getFlowersNumber(); flower++) {
        if ((beeInfo.currentPayload == BEE_PAYLOAD_MAX) || (timeGetTime() - initialTime >= 20))
            break;
        if (!hasMutex) {
            WaitForSingleObject(sync["checking_availability_of_flower"], INFINITE);
            hasMutex = true;
        }
        if ( (flowerbedsInfo[flowerbedIndex].getFlowers()[flower].isOccupied()) || (flowerbedsInfo[flowerbedIndex].getFlowers()[flower].getNectarUnits() == 0) )
            continue;

        flowerbedsInfo[flowerbedIndex].getFlowers()[flower].setOccupied(true);

        if (!ReleaseMutex(sync["checking_availability_of_flower"]))
            error("ReleaseMutex");

        hasMutex = false;

        // the bee has taken the flower and will be harvesting nectar

        while ((beeInfo.currentPayload != BEE_PAYLOAD_MAX) && (flowerbedsInfo[flowerbedIndex].getFlowers()[flower].getNectarUnits() > 0)) {
            flowerbedsInfo[flowerbedIndex].getFlowers()[flower].setNectarUnits(flowerbedsInfo[flowerbedIndex].getFlowers()[flower].getNectarUnits() - 1);
            InterlockedDecrement(flowerbedsInfo[flowerbedIndex].getTotalNectarUnits());
            if (*(flowerbedsInfo[flowerbedIndex].getTotalNectarUnits()) == 0)
                printf("All the nectar from flowerbed %d has been harvested.\n", flowerbedIndex + 1);
            beeInfo.currentPayload++;
        }
    }
    if (hasMutex) {
        if (!ReleaseMutex(sync["checking_availability_of_flower"]))
            error("ReleaseMutex");
    }

    while (timeGetTime() - initialTime < 20);
}

void returnToHive(int* hiveInfo, int distanceToFlowerbed) {
    flight(distanceToFlowerbed); // flight to hive

    WaitForSingleObject(sync["bees_in_hive"], INFINITE);

    WaitForSingleObject(sync["entry_door"], INFINITE);
    Sleep(2);

    WaitForSingleObject(sync["number_of_bees_in_and_out_of_hive"], INFINITE);
    hiveInfo[1]++; // a bee enters the hive
    hiveInfo[2]--;
    if (!ReleaseMutex(sync["number_of_bees_in_and_out_of_hive"]))
        error("ReleaseMutex");

    if (!ReleaseMutex(sync["entry_door"]))
        error("ReleaseMutex");

    WaitForSingleObject(sync["honey_in_hive"], INFINITE);
    int m = hiveInfo[0];
    hiveInfo[0] += beeInfo.currentPayload; // the bee leaves the harvested honey in the hive
    beeInfo.currentPayload = 0;
    if (m / 1000 != hiveInfo[0] / 1000)
        printf("%d nectar units have been harvested.\n", hiveInfo[0]/1000 * 1000);
    while (beeInfo.goitresNumber < 5) {
        hiveInfo[0] -= 1;
        beeInfo.goitresNumber++;
    }
    if (!ReleaseMutex(sync["honey_in_hive"]))
        error("ReleaseMutex");
}

void error(std::string functionName) {
    printf("Bee: Function %s() error (code: %ld).", functionName.c_str(), GetLastError());
    exit(1);
}
