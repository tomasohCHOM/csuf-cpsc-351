#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

const int N = 5;

enum class State { THINKING, HUNGRY, EATING };

std::mutex output_mutex;

void print_message(const std::string &message) {
  std::lock_guard<std::mutex> lock(output_mutex);
  std::cout << message << std::endl;
}

class DiningPhilosophersMonitor {
private:
  State state[N];
  std::mutex mtx;
  std::condition_variable cv[N];

  // Get the left neighbor of philosopher with given id
  int left_neighbor(int id) {
    return (id + N - 1) % N;
  }
  // Get the right neighbor of philosopher with given id
  int right_neighbor(int id) {
    return (id + 1) % N;
  }

  void try_eat(int id) {
    if (state[id] == State::HUNGRY &&
        state[left_neighbor(id)] != State::EATING &&
        state[right_neighbor(id)] != State::EATING) {
      // Philosopher now starts eating
      print_message("P#" + std::to_string(id) + " picked up left chopstick.");
      print_message("P#" + std::to_string(id) + " picked up right chopstick.");

      state[id] = State::EATING;
      cv[id].notify_all();
    }
  }

public:
  DiningPhilosophersMonitor() {
    for (int i = 0; i < N; ++i) {
      state[i] = State::THINKING;
    }
  }

  void pickup_chopsticks(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Set state to hungry
    state[id] = State::HUNGRY;
    print_message("P#" + std::to_string(id) + " HUNGRY.");

    // Attempt to eat if both chopsticks available
    try_eat(id);
    // If unable to eat, wait until the other philosophers stop eating
    if (state[id] != State::EATING) {
      print_message("P#" + std::to_string(id) + " WAITING for chopsticks.");
      cv[id].wait(lock, [this, id]() { return state[id] == State::EATING; });
    }
  }

  void putdown_chopsticks(int id) {
    std::unique_lock<std::mutex> lock(mtx);

    print_message("P#" + std::to_string(id) + " put down left chopstick.");
    print_message("P#" + std::to_string(id) + " put down right chopstick.");

    // Finished eating, return to THINKING state
    print_message("P#" + std::to_string(id) + " finished eating and is THINKING again.");
    state[id] = State::THINKING;
    // Check whether neighbors can start eating now
    try_eat(left_neighbor(id));
    try_eat(right_neighbor(id));
  }
};

void philosopher(DiningPhilosophersMonitor& monitor, int id) {
  while (true) {
    print_message("P#" + std::to_string(id) + " THINKING.");
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // HUNGRY state, tries to pick up chopsticks
    monitor.pickup_chopsticks(id);

    // EATING state
    print_message("P#" + std::to_string(id) + " EATING.");
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
