# Hive

## Short description

Project consists of 3 programs("hive", "queen" and "bee") that simulate the way the hive works. It uses the following functionalities provided by Windows OS:
* creating processes;
* semaphores;
* mutexes;
* shared memory;

"hive" is the main program. It creates hive and flowerbeds. Then it creates "queen" process.<br/>
"queen" is creating "bee" processes during the simulation.<br/>
"bee" repeatedly flies to the flowerbed, harvests the nectar, returns to hive and leaves the harvested honey in the hive. The bee uses a unit of honey for every
100 meters of flight so it fills its goitres with honey before leaving the hive.

## Build and run

**Build:**
```
mkdir build
cd build
cmake ..
cmake --build .
```
**Run:**<br/>
Go to build/Debug directory and execute only "hive.exe" with proper arguments.
