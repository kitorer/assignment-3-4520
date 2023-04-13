#include <iostream>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <mutex>

using namespace std;

mutex shared_mem_mutex;

const int SHARED_MEM_SIZE = 720; // Number of temperature readings per hour
const int NUM_SENSORS = 8;
const int NUM_THREADS = 8;
const int MAX_TEMP = 70;
const int MIN_TEMP = -100;

int shared_mem[SHARED_MEM_SIZE] = { 0 };
int curr_mem_index = 0;

int generate_random_temperature() {
    return (rand() % (MAX_TEMP - MIN_TEMP + 1)) + MIN_TEMP;
}

void generate_temperature_readings(int sensor_id) {
    while (true) {
        int temp_reading = generate_random_temperature();

        shared_mem_mutex.lock();
        shared_mem[curr_mem_index] = temp_reading;
        curr_mem_index++;
        shared_mem_mutex.unlock();

        this_thread::sleep_for(chrono::minutes(1));
    }
}

void compile_hourly_report() {
    while (true) {
        this_thread::sleep_for(chrono::hours(1));

        vector<int> temp_readings(SHARED_MEM_SIZE);
        shared_mem_mutex.lock();
        copy(shared_mem, shared_mem + SHARED_MEM_SIZE, temp_readings.begin());
        shared_mem_mutex.unlock();

        sort(temp_readings.begin(), temp_readings.end());
        cout << "Top 5 lowest temperatures: ";
        for (int i = 0; i < 5; i++) {
            cout << temp_readings[i] << "F ";
        }
        cout << endl;

        sort(temp_readings.rbegin(), temp_readings.rend());
        cout << "Top 5 highest temperatures: ";
        for (int i = 0; i < 5; i++) {
            cout << temp_readings[i] << "F ";
        }
        cout << endl;

        int max_diff = 0;
        int start_index = 0;
        for (int i = 0; i < SHARED_MEM_SIZE - 10; i++) {
            int diff = temp_readings[i + 10] - temp_readings[i];
            if (diff > max_diff) {
                max_diff = diff;
                start_index = i;
            }
        }
        cout << "Largest temperature difference: " << max_diff << "F" << endl;
        cout << "10-minute interval with largest temperature difference: ";
        for (int i = start_index; i < start_index + 10; i++) {
            cout << temp_readings[i] << "F ";
        }
        cout << endl;
    }
}

int main() {
    srand(time(NULL));

    vector<thread> sensor_threads;
    for (int i = 0; i < NUM_SENSORS; i++) {
        sensor_threads.push_back(thread(generate_temperature_readings, i));
    }
    thread report_thread(compile_hourly_report);

    for (int i = 0; i < NUM_SENSORS; i++) {
        sensor_threads[i].join();
    }

    report_thread.join();

    return 0;
}