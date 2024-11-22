#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define N 5

sem_t chopsticks[N];
pthread_mutex_t print_lock;

void print_message(const char *message, int id) {
  pthread_mutex_lock(&print_lock);
  printf(message, id);
  pthread_mutex_unlock(&print_lock);
}

void *philosopher(void *num) {
  int id = *((int *) num);
  int left_chopstick = (id + 1) % N;
  int right_chopstick = id;

  while (1) {
    // Philosopher is thinking
    print_message("P#%d THINKING.\n", id);
    sleep(1);

    if (id % 2 == 0) { // If id is even, pick up right chopstick first
      print_message("P#%d picked up right chopstick.\n", id);
      sem_wait(&chopsticks[right_chopstick]);

      print_message("P#%d picked up left chopstick.\n", id);
      sem_wait(&chopsticks[left_chopstick]);
    } else { // If id is odd, pick up left chopstick first
      print_message("P#%d picked up left chopstick.\n", id);
      sem_wait(&chopsticks[left_chopstick]);

      print_message("P#%d picked up right chopstick.\n", id);
      sem_wait(&chopsticks[right_chopstick]);
    }

    // Philosopher now eating
    print_message("P#%d EATING.\n", id);
    sleep(1);

    // Put down left chopstick
    sem_post(&chopsticks[left_chopstick]);
    print_message("P#%d put down left chopstick.\n", id);

    // Put down right chopstick
    sem_post(&chopsticks[right_chopstick]);
    print_message("P#%d put down right chopstick.\n", id);

    print_message("P#%d finished eating and is thinking again.\n", id);
  }
}

int main() {
  pthread_t philosophers[N];
  int ids[N];

  // Initialize semaphores
  for (int i = 0; i < N; ++i) {
    sem_init(&chopsticks[i], 0, 1);
    ids[i] = i;
  }
  pthread_mutex_init(&print_lock, NULL);

  // Create philosopher threads
  for (int i = 0; i < N; ++i) {
    pthread_create(&philosophers[i], NULL, philosopher, &ids[i]);
  }

  // Join threads
  for (int i = 0; i < N; ++i) {
    pthread_join(philosophers[i], NULL);
  }

  // Destroy sempahores and mutex
  for (int i = 0; i < N; ++i) {
    sem_destroy(&chopsticks[i]);
  }
  pthread_mutex_destroy(&print_lock);

  return 0;
}
