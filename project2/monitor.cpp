#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

#define N 5

enum class State { THINKING, HUNGRY, EATING };

std::mutex output_mutex;

void print_message(const std::string &message) {
  std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << message << std::endl;
}

class DiningPhilosophersMonitor {
private:
  State state[N];
  bool chopsticks[N];
  std::mutex mtx;
  std::condition_variable cv[N];

  // Get the left neighbor of philosopher with id 1
  int left_neighbor(int id) {
    return (id + N - 1) % N;
  }
  // Get the right neighbor of philosopher with id 1
  int right_neighbor(int id) {
    return (id + 1) % N;
  }

  void try_eat(int id) {
    if (state[id] == State::HUNGRY && !chopsticks[left_neighbor(id)] && !chopsticks[right_neighbor(id)]) {
      state[id] = State::EATING;

      // Philosopher now starts eating
      print_message("P#" + std::to_string(id) + " EATING.");

      chopsticks[left_neighbor(id)] = true;
      print_message("P#" + std::to_string(id) + " picked up left chopstick.");

      chopsticks[right_neighbor(id)] = true;
      print_message("P#" + std::to_string(id) + " picked up right chopstick.");

      cv[id].notify_all();
    }
  }

public:
  DiningPhilosophersMonitor() {
    for (int i = 0; i < N; ++i) {
      state[i] = State::THINKING;
      chopsticks[i] = false;
    }
  }

  void pickup_chopsticks(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Set state to hungry
    state[id] = State::HUNGRY;
    print_message("P#" + std::to_string(id) + " HUNGRY.");

    try_eat(id);

    while (state[id] != State::EATING) {
      if (chopsticks[left_neighbor(id)]) {
        print_message("P#" + std::to_string(id) + " WAITING for left chopstick.");
      }
      if (chopsticks[right_neighbor(id)]) {
        print_message("P#" + std::to_string(id) + " WAITING for right chopstick.");
      }
      cv[id].wait(lock);
    }
  }

  void putdown_chopsticks(int id) {
    std::unique_lock<std::mutex> lock(mtx);

    chopsticks[left_neighbor(id)] = false;
    print_message("P#" + std::to_string(id) + " put down left chopstick.");

    chopsticks[right_neighbor(id)] = false;
    print_message("P#" + std::to_string(id) + " put down right chopstick.");

    state[id] = State::THINKING;
    print_message("P#" + std::to_string(id) + " finished eating and is THINKING again.");

    cv[id].notify_all();
  }
};

void philosopher(DiningPhilosophersMonitor& monitor, int id) {
  while (true) {
    print_message("P#" + std::to_string(id) + " THINKING.");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // HUNGRY state, tries to pick up chopsticks
    monitor.pickup_chopsticks(id);

    // EATING state
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Puts down chopsticks, THINKING state now
    monitor.putdown_chopsticks(id);
  }
}

int main() {
  DiningPhilosophersMonitor monitor;
  std::vector<std::thread> philosophers;

  for (int i = 0; i < N; ++i) {
    philosophers.emplace_back(philosopher, std::ref(monitor), i);
  }

  for (auto& thread : philosophers) {
    thread.join();
  }

  return 0;
}
