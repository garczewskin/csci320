// Build: gcc -pthread -o dining dining.c
// Run:   ./dining <N 3..20> <algorithm 1..4> [think_us eat_us gap_us slow_list rogue_right]
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "common_threads.h"
#include "zemaphore.h"

#define MAX_N 20

int N, algo, rogue = 0, rogue_right = 0;
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
        Zem_wait(&forks[left(p)]);
        usleep(gap_us[p]);
        Zem_wait(&forks[right(p)]);
        eat(p);
        Zem_post(&forks[left(p)]);
        Zem_post(&forks[right(p)]);
    }
}

int both_free(int p) { return fork_free[left(p)] && fork_free[right(p)]; }
void take(int p)     { fork_free[left(p)] = fork_free[right(p)] = 0; }
void *philosopher2(void *arg) {         // both forks or neither
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        Zem_wait(&state_lock);
        if (both_free(p)) {
            take(p);
            Zem_post(&state_lock);
        } else {
            is_waiting[p] = 1;
            Zem_post(&state_lock);
            Zem_wait(&self[p]);
        }        
        eat(p);    
        Zem_wait(&state_lock);
        fork_free[left(p)] = fork_free[right(p)] = 1;
        int nb[2] = { (p + N - 1) % N, (p + 1) % N };
        for (int i = 0; i < 2; i++) {
            int q = nb[i];
            if (is_waiting[q] && both_free(q)) {
                take(q);
                is_waiting[q] = 0;
                Zem_post(&self[q]);
            }
        }
        Zem_post(&state_lock);
    }
}

void *philosopher3(void *arg) {         // table semaphore, right then left
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        Zem_wait(&table);
        Zem_wait(&forks[right(p)]);
        usleep(gap_us[p]);
        Zem_wait(&forks[left(p)]);        
        eat(p);
        Zem_post(&forks[left(p)]);
        Zem_post(&forks[right(p)]);
        Zem_post(&table);    
    }
}

void set_free(int f, int v) { Zem_wait(&state_lock); fork_free[f] = v; Zem_post(&state_lock); }
void grab(int f) { Zem_wait(&forks[f]); set_free(f, 0); }
void drop(int f) { set_free(f, 1); Zem_post(&forks[f]); }
void *philosopher4(void *arg) {         // rogue takes any free fork first
    int p = (int)(long long)arg;
    while (1) {
        think(p);
        int first = left(p), second = right(p);
        if (p == rogue) {
            Zem_wait(&state_lock);
            int l = fork_free[left(p)], r = fork_free[right(p)];
            if (rogue_right ? (r || !l) : (!l && r)) { first = right(p); second = left(p); }
            Zem_post(&state_lock);
        }
        grab(first);
        usleep(gap_us[p]);
        grab(second);
        eat(p);
        drop(first);
        drop(second);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) { fprintf(stderr, "usage: %s N algorithm [think_us eat_us gap_us slow_list rogue_right]\n", argv[0]); return 1; }
    N = atoi(argv[1]);
    algo = atoi(argv[2]);
    if (N < 3 || N > MAX_N || algo < 1 || algo > 4) { fprintf(stderr, "bad args\n"); return 1; }

    int think_arg = argc > 3 ? atoi(argv[3]) : 1000;    // think time for everyone
    int eat_arg   = argc > 4 ? atoi(argv[4]) : 1000;    // eat time (staggered by philosopher)
    int gap_arg   = argc > 5 ? atoi(argv[5]) : 0;       // pause between the two forks
    char *slow    = argc > 6 ? argv[6] : "";            // e.g. 0,4 : these philosophers think 1 s
    rogue_right   = argc > 7 ? atoi(argv[7]) : 0;       // Alg 4 tie-break: 1 = rogue prefers right

    for (int i = 0; i < N; i++) {
        think_us[i] = think_arg;
        eat_us[i] = eat_arg + 1000 * i;
        gap_us[i] = gap_arg;
        fork_free[i] = 1;
        Zem_init(&forks[i], 1);
        Zem_init(&self[i], 0);
    }
    for (char *tok = strtok(slow, ","); tok; tok = strtok(NULL, ","))
        think_us[atoi(tok)] = 1000000;
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