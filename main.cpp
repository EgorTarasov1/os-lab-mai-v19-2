#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <windows.h>

const int MAX_THREADS = 64;

struct ThreadData {
    std::vector<int>* arr;
    int start;
    int end;
    bool even;
};

DWORD WINAPI sortThread(LPVOID param) {
    ThreadData* data = static_cast<ThreadData*>(param);
    std::vector<int>& arr = *(data->arr);
    int start = data->start;
    int end = data->end;
    bool even = data->even;

    for (int i = start; i < end; i += 2) {
        if (even && i + 1 < arr.size()) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
            }
        } else if (!even && i + 1 < arr.size()) {
            if (arr[i] > arr[i + 1]) {
                std::swap(arr[i], arr[i + 1]);
            }
        }
    }
    return 0;
}

void batcherSort(std::vector<int>& arr, int numThreads) {
    int n = arr.size();
    bool sorted = false;

    std::vector<HANDLE> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);

    while (!sorted) {
        sorted = true;

        for (int phase = 0; phase < 2; ++phase) {
            int elementsPerThread = (n + numThreads - 1) / numThreads;

            for (int i = 0; i < numThreads; ++i) {
                threadData[i].arr = &arr;
                threadData[i].start = i * elementsPerThread + phase;
                threadData[i].end = std::min((i + 1) * elementsPerThread, n);
                threadData[i].even = (phase == 0);

                threads[i] = CreateThread(NULL, 0, sortThread, &threadData[i], 0, NULL);
            }

            WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

            for (int i = 0; i < numThreads; ++i) {
                CloseHandle(threads[i]);
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            if (arr[i] > arr[i + 1]) {
                sorted = false;
                break;
            }
        }
    }
}

void printArray(const std::vector<int>& arr) {
    for (int num : arr) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <num_threads>" << std::endl;
        return 1;
    }

    int numThreads = std::atoi(argv[1]);
    if (numThreads <= 0 || numThreads > MAX_THREADS) {
        std::cerr << "Number of threads should be between 1 and " << MAX_THREADS << std::endl;
        return 1;
    }

    const int arraySize = 20; 
    std::vector<int> arr(arraySize);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    for (int& num : arr) {
        num = dis(gen);
    }

    std::cout << "Original array:" << std::endl;
    printArray(arr);

    batcherSort(arr, numThreads);

    std::cout << "Sorted array:" << std::endl;
    printArray(arr);

    return 0;
}