#include "../include/Flower.h"

Flower::Flower() {
    nectarUnits = rand() % 20 + 5;
    occupied = false;
}

int Flower::getNectarUnits() {
    return nectarUnits;
}

void Flower::setNectarUnits(int nectarUnits) {
    this->nectarUnits = nectarUnits;
}

bool Flower::isOccupied() {
    return occupied;
}

void Flower::setOccupied(bool occupied) {
    this->occupied = occupied;
}
