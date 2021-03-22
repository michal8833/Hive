#include "../include/Flowerbed.h"

Flowerbed::Flowerbed() {
    flowersNumber = rand() % 100 + 1;

    totalNectarUnits = 0;
    for (int i=0; i<flowersNumber; i++)
        totalNectarUnits += flowers[i].getNectarUnits();

    distanceFromHive = rand() % 240 + 10;
}

Flower* Flowerbed::getFlowers() {
    return flowers;
}

int Flowerbed::getFlowersNumber() {
   return flowersNumber;
}

long volatile* Flowerbed::getTotalNectarUnits() {
    return &totalNectarUnits;
}

int Flowerbed::getDistanceFromHive() {
    return distanceFromHive;
}

