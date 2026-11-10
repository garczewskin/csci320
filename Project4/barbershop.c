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
Zem_t mutex;            // protects shared state
Zem_t customers;        // barbers sleep here; customers post once seated

// Part 2: one barber, shared rendezvous semaphores
int waiting = 0;
Zem_t barber_ready, cut_done;

void *barber_simple(void *arg) {
    while (1) {
        // TODO
    }
}

void *customer_simple(void *arg) {
    int id = (int)(long long)arg;
    while (1) {
        // TODO
    }
}

// Part 3: per-customer semaphores and a queue of customer ids
int queue[MAX_CUSTOMERS], q_head = 0, q_tail = 0, q_count = 0;
Zem_t ready[MAX_CUSTOMERS], done[MAX_CUSTOMERS];

int queue_push(int id) { /* TODO: 1 on success, 0 if full */ return 0; }
int queue_pop(void)    { /* TODO */ return -1; }

void *barber_multi(void *arg) {
    int b = (int)(long long)arg;
    while (1) {
        // TODO
    }
}

void *customer_multi(void *arg) {
    int id = (int)(long long)arg;
    while (1) {
        // TODO
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
    exit(0);
}
