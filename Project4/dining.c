// Build: gcc -pthread -o dining dining.c
// Run:   ./dining <N 3..20> <algorithm 1..4>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "common.h"
#include "common_threads.h"
#include "zemaphore.h"

#define MAX_N 20

int N, algo, rogue = 0;
int think_us[MAX_N], eat_us[MAX_N], gap_us[MAX_N];
long eat_count[MAX_N];

Zem_t forks[MAX_N];     // Alg 1, 3, 4
Zem_t table;            // Alg 3: init N-1
Zem_t state_lock;       // Alg 2, 4: protects fork_free, is_waiting
int fork_free[MAX_N];
Zem_t self[MAX_N];      // Alg 2: per-philosopher wakeup, init 0
int is_waiting[MAX_N];

int left(int p)  { return p; }
int right(int p) { return (p + 1) % N; }

void think(int p) { usleep(think_us[p]); }
void eat(int p)   { eat_count[p]++; usleep(eat_us[p]); }

void *philosopher1(void *arg) {         // left, then right
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        // TODO
        eat(p);
        // TODO
    }
}

void *philosopher2(void *arg) {         // both forks or neither
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        // TODO
        eat(p);
        // TODO
    }
}

void *philosopher3(void *arg) {         // table semaphore, right then left
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        // TODO
        eat(p);
        // TODO
    }
}

void *philosopher4(void *arg) {         // rogue takes any free fork first
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        // TODO
        eat(p);
        // TODO
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) { fprintf(stderr, "usage: %s N algorithm\n", argv[0]); return 1; }
    N = atoi(argv[1]);
    algo = atoi(argv[2]);
    if (N < 3 || N > MAX_N || algo < 1 || algo > 4) { fprintf(stderr, "bad args\n"); return 1; }

    for (int i = 0; i < N; i++) {
        think_us[i] = eat_us[i] = 1000;
        gap_us[i] = 0;
        fork_free[i] = 1;
        Zem_init(&forks[i], 1);
        Zem_init(&self[i], 0);
    }
    Zem_init(&table, N - 1);
    Zem_init(&state_lock, 1);

    void *(*fn[])(void *) = { philosopher1, philosopher2, philosopher3, philosopher4 };
    pthread_t t[MAX_N];
    for (int i = 0; i < N; i++) {
        long long id = i;
        Pthread_create(&t[i], NULL, fn[algo - 1], (void *)id);
    }

    // Watchdog: report deadlock if no one eats for 5 seconds, else stop after 20.
    long last = -1;
    int stalled = 0;
    for (int s = 0; s < 20 && stalled < 5; s++) {
        sleep(1);
        long total = 0;
        for (int i = 0; i < N; i++) total += eat_count[i];
        stalled = (total == last) ? stalled + 1 : 0;
        last = total;
    }
    if (stalled >= 5) printf("DEADLOCK SUSPECTED\n");
    for (int i = 0; i < N; i++) printf("philosopher %d: %ld\n", i, eat_count[i]);
    exit(0);
}
