# Project 4 - Concurrency
**Files:** `dining.c` (Part 1), `barbershop.c` (Parts 2 and 3)

## Part 1: Dining Philosophers

Run: `./dining N algo think_us eat_us gap_us slow_list rogue_right`

- `think_us`, `eat_us`: think and eat time in microseconds (philosopher `i` eats for `eat_us + 1000*i`)
- `gap_us`: pause between picking up the two forks
- `slow_list`: philosophers that think for 1 second, e.g. `0,4` (`""` for none)
- `rogue_right`: 1 means the Algorithm 4 rogue prefers its right fork

Each run lasts 20 seconds and prints how many times each philosopher ate (0 to 4). N = 5 throughout. `DEADLOCK SUSPECTED` prints if nobody eats for 5 seconds.

### Algorithm 1: left fork, then right fork

Wait on the left fork, pause, wait on the right fork.

| Command | Result |
|---|---|
| `./dining 5 1 0 1000 100000` | Deadlock, 0 eats for all |
| `./dining 5 1` | Deadlock, 0 eats for all |

Deadlock occurred. Every philosopher holds its left fork and waits for the right.

### Algorithm 2: both forks or neither

A philosopher takes both forks only if both are free, otherwise it waits to be woken by a neighbor.

| Command | Eats |
|---|---|
| `./dining 5 2` | 3377 / 4288 / 2900 / 1480 / 2430 |
| `./dining 5 2 0 50000 0 2` | 134 / 252 / 18 / 236 / 129 |
| `./dining 5 2 0 50000 0 0,4` | 18 / 365 / 19 / 347 / 18 |

No deadlock. In the baseline everyone eats, and the counts differ because eat times differ. A long thinker alone does not starve (philosopher 2 ate 18 times, about its maximum). With 0 and 4 slow, philosopher 2 starved: 19 eats against 365 and 347 for neighbors 1 and 3, who take the forks right back.

### Algorithm 3: at most N-1 seated

A `table` semaphore starts at N-1. Take a seat, then the right fork, then the left.

| Command | Eats |
|---|---|
| `./dining 5 3 0 1000 100000` | 152 / 149 / 148 / 147 / 149 |

No deadlock with the settings that deadlocked Algorithm 1. Counts are even.

### Algorithm 4: rogue philosopher

Philosopher 0 takes whichever of its two forks looks free first, then waits for the other. The rest behave as in Algorithm 1.

| Command | Result |
|---|---|
| `./dining 5 4 0 1000 100000 "" 0` (pause, prefers left) | Deadlock, 0 / 0 / 0 / 0 / 0 |
| `./dining 5 4 0 1000 100000 "" 1` (pause, prefers right) | Deadlock, 2 / 1 / 3 / 3 / 3 |
| `./dining 5 4 1000 1000 0 "" 0` (no pause, prefers left) | Deadlock, 812 / 811 / 811 / 811 / 810 |
| `./dining 5 4 1000 1000 0 "" 1` (no pause, prefers right) | Deadlock, 1032 / 1031 / 1031 / 1031 / 1030 |

The rogue does not prevent deadlock. It still happens when the rogue holds fork 0 and the others hold their left forks. The tie-break only changed how soon: immediately with the pause, after about 800 to 1000 eats without it.

## Part 2: Barbershop, one barber

Run: `./barbershop simple <chairs> 1 <customers>`

The barber sleeps on `customers`, which each seated customer posts. `barber_ready` calls the customer to the chair and `cut_done` releases them after the cut. `customer_left` keeps the barber from starting the next cut before the last customer leaves. A customer who finds every chair taken leaves and comes back later.

| Command | Haircuts | Balks | Early leaves |
|---|---|---|---|
| `./barbershop simple 3 1 8` | 147 | 81 | 0 |

8 customers, 3 chairs, 30 second run, 200 ms cuts (maximum 150 haircuts).

## Part 3: Multiple barbers

Run: `./barbershop multi <chairs> <barbers> <customers>`

Each customer has its own `ready[id]` and `done[id]` semaphores, and waiting customers sit in a queue of ids. A barber takes the next id, signals that customer, cuts, then signals their `done`. The barber count is a command-line argument.

| Command | Haircuts | Balks | Early leaves | Max concurrent cuts |
|---|---|---|---|---|
| `./barbershop multi 3 1 8` | 147 | 87 | 0 | 1 |
| `./barbershop multi 3 2 8` | 268 | 4 | 0 | 2 |
| `./barbershop multi 3 2 20` | 296 | 456 | 0 | 2 |

Two barbers cut at the same time, so haircuts nearly double (maximum 300). With 8 customers almost no one is turned away. With 20 customers the chairs fill up and many are turned away.