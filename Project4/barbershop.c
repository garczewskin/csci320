// Build: gcc -pthread -o barbershop barbershop.c
// Run:   ./barbershop <simple|multi> <chairs> <barbers> <customers>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "common.h"
#include "common_threads.h"
#include "zemaphore.h"

#define MAX_CUSTOMERS 64
#define MAX_BARBERS 8

int n_chairs, num_barbers, num_customers;
int cut_us = 200000, cuts = 0, balks = 0;
Zem_t mutex;            // protects shared state
Zem_t customers;        // barbers sleep here; customers post once seated

// Part 2: one barber, shared rendezvous semaphores
int waiting = 0;
Zem_t barber_ready, cut_done, customer_left;

void *barber_simple(void *arg) {
    while (1) {
        Zem_wait(&customers);
        Zem_wait(&mutex); waiting--; Zem_post(&mutex);
        Zem_post(&barber_ready);
        printf("barber: cutting\n");
        usleep(cut_us);
        printf("barber: finished\n");
        Zem_post(&cut_done);
        Zem_wait(&customer_left);
    }
}

void *customer_simple(void *arg) {
    int id = (int)(long long)arg;
    unsigned seed = id;
    while (1) {
        usleep(200000 + rand_r(&seed) % 800000);
        Zem_wait(&mutex);
        if (waiting == n_chairs) {
            balks++;
            Zem_post(&mutex);
            printf("customer %d: shop full, leaves\n", id);
            continue;
        }
        waiting++;
        printf("customer %d: sits (%d waiting)\n", id, waiting);
        Zem_post(&mutex);
        Zem_post(&customers);
        Zem_wait(&barber_ready);
        printf("customer %d: getting haircut\n", id);
        Zem_wait(&cut_done);
        printf("customer %d: leaves with haircut\n", id);
        Zem_wait(&mutex); cuts++; Zem_post(&mutex);
        Zem_post(&customer_left);
    }
}

// Part 3: per-customer semaphores and a queue of customer ids
int queue[MAX_CUSTOMERS], q_head = 0, q_tail = 0, q_count = 0;
Zem_t ready[MAX_CUSTOMERS], done[MAX_CUSTOMERS];

int queue_push(int id) {
    if (q_count == n_chairs) return 0;
    queue[q_tail] = id;
    q_tail = (q_tail + 1) % MAX_CUSTOMERS;
    q_count++;
    return 1;
}
int queue_pop(void) {
    int id = queue[q_head];
    q_head = (q_head + 1) % MAX_CUSTOMERS;
    q_count--;
    return id;
}

void *barber_multi(void *arg) {
    int b = (int)(long long)arg;
    while (1) {
        Zem_wait(&customers);
        Zem_wait(&mutex); int id = queue_pop(); Zem_post(&mutex);
        Zem_post(&ready[id]);
        printf("barber %d: cutting customer %d\n", b, id);
        usleep(cut_us);
        printf("barber %d: finished customer %d\n", b, id);
        Zem_post(&done[id]);
    }
}

void *customer_multi(void *arg) {
    int id = (int)(long long)arg;
    unsigned seed = id;
    while (1) {
        usleep(200000 + rand_r(&seed) % 800000);
        Zem_wait(&mutex);
        if (!queue_push(id)) {
            balks++;
            Zem_post(&mutex);
            printf("customer %d: shop full, leaves\n", id);
            continue;
        }
        printf("customer %d: sits\n", id);
        Zem_post(&mutex);
        Zem_post(&customers);
        Zem_wait(&ready[id]);
        printf("customer %d: getting haircut\n", id);
        Zem_wait(&done[id]);
        printf("customer %d: leaves with haircut\n", id);
        Zem_wait(&mutex); cuts++; Zem_post(&mutex);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) { fprintf(stderr, "usage: %s simple|multi chairs barbers customers\n", argv[0]); return 1; }
    int multi = strcmp(argv[1], "multi") == 0;
    n_chairs = atoi(argv[2]);
    num_barbers = atoi(argv[3]);
    num_customers = atoi(argv[4]);
    if (n_chairs < 1 || num_barbers < 1 || num_barbers > MAX_BARBERS ||
        num_customers < 1 || num_customers > MAX_CUSTOMERS || (!multi && num_barbers != 1)) {
        fprintf(stderr, "bad args\n");
        return 1;
    }

    Zem_init(&mutex, 1);
    Zem_init(&customers, 0);
    Zem_init(&barber_ready, 0);
    Zem_init(&cut_done, 0);
    Zem_init(&customer_left, 0);
    for (int i = 0; i < num_customers; i++) {
        Zem_init(&ready[i], 0);
        Zem_init(&done[i], 0);
    }

    pthread_t bt[MAX_BARBERS], ct[MAX_CUSTOMERS];
    for (int i = 0; i < num_barbers; i++) {
        long long id = i;
        Pthread_create(&bt[i], NULL, multi ? barber_multi : barber_simple, (void *)id);
    }
    for (int i = 0; i < num_customers; i++) {
        long long id = i;
        Pthread_create(&ct[i], NULL, multi ? customer_multi : customer_simple, (void *)id);
    }

    sleep(30);
    printf("haircuts: %d, balks: %d\n", cuts, balks);
    exit(0);
}