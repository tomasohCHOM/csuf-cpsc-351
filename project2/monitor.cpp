#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

#define N 5

enum class State { THINKING, HUNGRY, EATING };

std::mutex output_mutex;

void safe_print(const std::string &message) {
  std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << message << std::endl;
}

class DiningPhilosophersMonitor {
private:
  std::mutex mtx;
  std::condition_variable cv[N];
  State state[N];

public:
  DiningPhilosophersMonitor() {
    for (int i = 0; i < N; ++i) {
      state[i] = State::THINKING;
    }
  }

  void pickUpChopsticks(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Set state to hungry
    state[id] = State::HUNGRY;
    safe_print("P#" + std::to_string(id) + " HUNGRY.")


  }
};

