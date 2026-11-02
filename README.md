# csci320
This repository contains selected work for CSCI 320: Operating Systems

## Contents
**Project 0A: Familiarization with C, part 1**
- Reads a text file as input using `stdin` and outputs the number of times each letter of the alphabet appears in the file

**Project 0B: Implementing Circular Queues**
- Implements `insert` and `remove` methods for a circular queue in C. Builds on skeleton files provided by the instructor.

**Project 1: Intro to xv6**
- Added syscall to xv6 operating system called `getfilenum` which returns the number of files that the process identified by pid currently has open.
- See [this repository](https://github.com/garczewskin/csci320-xv6) for this work.

**Project 2: Scheduling**
- Added stride scheduler implementation to xv6 operating system
- See [this repository](https://github.com/garczewskin/csci320-xv6) for this work.

**Project 3: Virtual Memory**
- Made three virtual memory modifications to xv6 operating system
    - Add `vmprint()` function
    - Add `pgaccess()` syscall
    - Optimize syscalls using read-only memory
- See [this repository](https://github.com/garczewskin/csci320-xv6) for this work.

**Project 4: Concurrency**
- In-progress

