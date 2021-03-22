#ifndef FLOWERBED_H
#define FLOWERBED_H

#include "Flower.h"

class Flowerbed {
public:
    Flowerbed();

    Flower* getFlowers();
    int getFlowersNumber();
    long volatile* getTotalNectarUnits();
    int getDistanceFromHive();

private:
    Flower flowers[100];
    int flowersNumber;
    long volatile totalNectarUnits;
    int distanceFromHive;
};

#endif //FLOWERBED_H
