#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

#define N 5

sem_t mutex;
sem_t chopsticks[N];

void *philosopher(void *num) {
  int id = *((int *) num);
  while (1) {
    printf("P#%d THINKING.\n", id);
    sleep(rand() % 3);

    sem_wait(&chopsticks[id]);
    printf("P#%d picked up right chopstick.\n", id);

    sem_wait(&chopsticks[(id + 1) % N]);
    printf("P#%d picked up left chopstick.\n", id);

    printf("P#%d EATING.\n", id);
    sleep(rand() % 3);

    sem_post(&chopsticks[id]);
    printf("P#%d put down right chopstick.\n", id);

    sem_post(&chopsticks[(id + 1) % N]);
    printf("P#%d put down left chopstick.\n", id);

    printf("P#%d finished eating and is thinking again.\n", id);
  }
}

int main() {
  pthread_t philosophers[N];
  int philosophers_ids[N];

  sem_init(&mutex, 0, 1);
  for (int i = 0; i < N; ++i) {
    sem_init(&chopsticks[i], 0, 1);
  }

  for (int i = 0; i < N; ++i) {
    philosophers_ids[i] = i;
    pthread_create(&philosophers[i], NULL, philosopher, &philosophers_ids[i]);
  }

  for (int i = 0; i < N; ++i) {
    pthread_join(philosophers[i], NULL);
  }

  return 0;
}
