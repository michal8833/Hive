#ifndef FLOWER_H
#define FLOWER_H

#include <cstdlib>

class Flower {
public:
    Flower();

    int getNectarUnits();
    void setNectarUnits(int nectarUnits);
    bool isOccupied();
    void setOccupied(bool occupied);

private:
    int nectarUnits;
    bool occupied; //synchronizacja dostepu do kwiatow
};

#endif //FLOWER_H
