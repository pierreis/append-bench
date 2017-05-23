#include <assert.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "atomic.h"
#include "tinycthread.h"

#define NUM_COUNTERS  2
#define TIMEOUT       1000

#define NUM_FILES     10

#define WRITE_SIZE    100

struct periodic_counter {
  uint32_t requests;
};

static atomic_bool done;
static struct periodic_counter counters[NUM_COUNTERS];
static atomic_uintptr_t counter;

int files[NUM_FILES];

void init_counter(struct periodic_counter *counter) {
  counter->requests = 0;
}

int timer(void *arg) {
  sleep(TIMEOUT);
  atomic_store_explicit(&done, true, memory_order_relaxed);
  return 0;
}

int work(void *arg) {

  // Write buffer
  char buf[WRITE_SIZE];

  while(!atomic_load_explicit(&done, memory_order_relaxed)) {

    // Do something incredible here
    int file = files[rand() % NUM_FILES];
    memset(&buf, random(), WRITE_SIZE);
    write(file, &buf, WRITE_SIZE);

    // Load counter
    struct periodic_counter *current_counter = (struct periodic_counter *) atomic_load_explicit(&counter, memory_order_relaxed);

    // Update stats
    ++current_counter->requests;

  }
  return 0;
}

int count(void *arg) {
  uint32_t current_counter = 0;
  usleep(500000);
  while(!atomic_load_explicit(&done, memory_order_relaxed)) {

    // Update counter, and sleep a bit after to clear every concurrent writing process
    usleep(500000);
    uint32_t prev_counter = current_counter;
    current_counter = (current_counter + 1) % NUM_COUNTERS;
    atomic_store_explicit(&counter, (uintptr_t) &counters[current_counter], memory_order_relaxed);
    usleep(500000);

    // Print results
    struct periodic_counter *prev = &counters[prev_counter];
    printf("%u\n", prev->requests);

    // Clear counter
    init_counter(prev);

  }
  return 0;
}

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("usage: %s file_template", argv[0]);
    exit(1);
  }

  // Init stuff.
  atomic_init(&done, false);
  uint32_t i = 0;
  for (; i < NUM_COUNTERS; ++i) {
    init_counter(&counters[i]);
  }
  atomic_init(&counter, (uintptr_t) &counters[0]);
  srand((unsigned int) time(NULL));

  // Open files
  uint32_t base_path_len = (uint32_t) strlen(argv[1]);
  for (i = 0; i < NUM_FILES; ++i) {
    char path[100];
    memcpy(&path[0], argv[1], base_path_len);
    snprintf(&path[base_path_len], 10, "%" PRIu32, i);
    printf("File %u: %s\n", i, (char *) &path);
    files[i] = open((char *) &path, O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    assert(files[i] != -1);
  }

  // Start counter thread
  thrd_t updater;
  thrd_create(&updater, count, NULL);

  // Start worker thread
  thrd_t worker;
  thrd_create(&worker, work, NULL);

  // Start worker thread
  thrd_t timeout;
  thrd_create(&timeout, timer, NULL);

  // Kill the counter thread.
  thrd_join(timeout, NULL);
  thrd_join(worker, NULL);
  thrd_join(updater, NULL);

  return 0;
}