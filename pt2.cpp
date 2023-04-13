#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <functional>

#define NUM_THREADS 8
#define NUM_SENSORS 8
#define NUM_READINGS 60
#define SHARED_MEM_SIZE NUM_SENSORS*NUM_READINGS

using namespace std;

void readTemperatures(int sensorIndex, float* readings);
void generateReadings(float* readings);

mutex sharedMemMutex;
float sharedMem[SHARED_MEM_SIZE];

int main() {
    for (int i = 0; i < SHARED_MEM_SIZE; i++) {
        sharedMem[i] = 0.0f;
    }

    thread threads[NUM_THREADS];
    for (int i = 0; i < NUM_SENSORS; i++) {
        float* readings = new float[NUM_READINGS];
        threads[i] = thread(readTemperatures, i, readings);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    return 0;
}

void readTemperatures(int sensorIndex, float* readings) {
    while (true) {
        generateReadings(readings);

        sharedMemMutex.lock();
        for (int i = 0; i < NUM_READINGS; i++) {
            sharedMem[sensorIndex*NUM_READINGS + i] = *(readings + i);
        }
        sharedMemMutex.unlock();

        this_thread::sleep_for(chrono::seconds(60));
    }
}

void generateReadings(float* readings) {
    for (int i = 0; i < NUM_READINGS; i++) {
        readings[i] = static_cast<float>(rand() % 171 - 100); // units in Fahrenheit
    }
}
