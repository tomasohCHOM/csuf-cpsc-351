
// dining_philosophers_monitor_enhanced.cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

const int NUM_PHILOSOPHERS = 5;

// Enum to represent the state of each philosopher
enum class PhilosopherState { THINKING, HUNGRY, EATING };

std::mutex print_mutex;

void safe_print(const std::string& message) {
    std::lock_guard<std::mutex> lock(print_mutex);
    std::cout << message << std::endl;             
}

class DiningPhilosophersMonitor {
private:
    PhilosopherState state[NUM_PHILOSOPHERS];                    
    bool chopsticks[NUM_PHILOSOPHERS];                         
    std::mutex monitor_mutex;                                    
    std::condition_variable condition_vars[NUM_PHILOSOPHERS];     

    // Helper function to get the index of the left neighbor
    int left_neighbor(int philosopher_id) {
        return (philosopher_id + NUM_PHILOSOPHERS - 1) % NUM_PHILOSOPHERS;
    }

    // Helper function to get the index of the right neighbor
    int right_neighbor(int philosopher_id) {
        return (philosopher_id + 1) % NUM_PHILOSOPHERS;
    }

    // Function to check if a philosopher can start eating
    void check_if_philosopher_can_eat(int philosopher_id) {
        if (state[philosopher_id] == PhilosopherState::HUNGRY &&
            !chopsticks[left_neighbor(philosopher_id)] &&
            !chopsticks[right_neighbor(philosopher_id)]) {
            // Both chopsticks are available -> philosopher can eat
            chopsticks[left_neighbor(philosopher_id)] = true;   
            chopsticks[right_neighbor(philosopher_id)] = true; 
            state[philosopher_id] = PhilosopherState::EATING;
            condition_vars[philosopher_id].notify_one();        

            // Log chopstick pick-up
            safe_print("P#" + std::to_string(philosopher_id) + " picked up left chopstick.");
            safe_print("P#" + std::to_string(philosopher_id) + " picked up right chopstick.");
        }
    }

public:
    DiningPhilosophersMonitor() {
        for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
            state[i] = PhilosopherState::THINKING;
            chopsticks[i] = false; // All chopsticks are initially available
        }
    }

    // Function for a philosopher to pick up chopsticks
    void pickup_chopsticks(int philosopher_id) {
        std::unique_lock<std::mutex> lock(monitor_mutex); 

        // Set philosopher's state to HUNGRY
        state[philosopher_id] = PhilosopherState::HUNGRY;
        safe_print("P#" + std::to_string(philosopher_id) + " HUNGRY.");

        // Attempt to pick up chopsticks
        check_if_philosopher_can_eat(philosopher_id);

        // If unable to eat, wait until notified
        while (state[philosopher_id] != PhilosopherState::EATING) {
            // Determine which chopstick is unavailable and log waiting
            if (chopsticks[left_neighbor(philosopher_id)]) {
                safe_print("P#" + std::to_string(philosopher_id) + " WAITING for left chopstick.");
            }
            if (chopsticks[right_neighbor(philosopher_id)]) {
                safe_print("P#" + std::to_string(philosopher_id) + " WAITING for right chopstick.");
            }
            condition_vars[philosopher_id].wait(lock);
        }

        // Philosopher has acquired both chopsticks and starts eating
        safe_print("P#" + std::to_string(philosopher_id) + " EATING.");
    }

    void putdown_chopsticks(int philosopher_id) {
        std::unique_lock<std::mutex> lock(monitor_mutex); 

        // Put down left chopstick
        chopsticks[left_neighbor(philosopher_id)] = false;
        safe_print("P#" + std::to_string(philosopher_id) + " put down left chopstick.");

        // Put down right chopstick
        chopsticks[right_neighbor(philosopher_id)] = false;
        safe_print("P#" + std::to_string(philosopher_id) + " put down right chopstick.");

        // Set philosopher's state to THINKING
        state[philosopher_id] = PhilosopherState::THINKING;
        safe_print("P#" + std::to_string(philosopher_id) + " finished eating and is THINKING again.");

        // Check if neighbors can now eat
        check_if_philosopher_can_eat(left_neighbor(philosopher_id));
        check_if_philosopher_can_eat(right_neighbor(philosopher_id));
    }
};

// Function to simulate philosopher behavior
void philosopher_activity(int philosopher_id, DiningPhilosophersMonitor& monitor, std::mt19937& rng) {
    // Thinking time between 1 and 3 seconds
    std::uniform_int_distribution<int> think_duration_dist(1, 3); 
    // Eating time between 1 and 2 seconds
    std::uniform_int_distribution<int> eat_duration_dist(1, 2);   

    while (true) {
        // Philosopher is THINKING
        safe_print("P#" + std::to_string(philosopher_id) + " THINKING.");
        std::this_thread::sleep_for(std::chrono::seconds(think_duration_dist(rng)));

        // Philosopher is HUNGRY and tries to pick up chopsticks
        monitor.pickup_chopsticks(philosopher_id);

        // Philosopher is EATING
        std::this_thread::sleep_for(std::chrono::seconds(eat_duration_dist(rng)));

        // Philosopher puts down chopsticks and starts THINKING again
        monitor.putdown_chopsticks(philosopher_id);
    }
}

int main() {
    DiningPhilosophersMonitor monitor;                   
    std::vector<std::thread> philosophers;               
    std::random_device rd;                               
    std::mt19937 rng(rd());                               

    for (int i = 0; i < NUM_PHILOSOPHERS; ++i) {
        philosophers.emplace_back(philosopher_activity, i, std::ref(monitor), std::ref(rng));
    }

    for (auto& philosopher_thread : philosophers) {
        philosopher_thread.join();
    }

    return 0;
}

