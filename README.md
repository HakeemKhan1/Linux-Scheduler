# Multilevel Process Scheduler Simulator

This project is a C-based simulation of a **Multilevel Feedback Queue (MLFQ) scheduler** using POSIX threads. It demonstrates how operating systems schedule processes based on static priority, dynamic priority, and different scheduling policies.

##  Key Features

- Simulates 3 scheduling policies:
  - **RR** (Round Robin)
  - **FIFO** (First In First Out)
  - **NORMAL** (priority-based, Linux-like behavior)
- Implements 3 Ready Queues (RQ0, RQ1, RQ2) for different priority ranges
- Uses `pthread` for multithreading:
  - 1 **producer thread**: Reads process data from a file
  - 3 **consumer threads**: Execute processes based on scheduling logic
- Dynamic priority updates based on simulated execution behavior
- Thread-safe queue operations via mutex locks

---

##  Input Format (`process_data.txt`)

Each line in `process_data.txt` must follow this format:

```
PID,NAME,STATIC_PRIO,DYNAMIC_PRIO,REMAIN_TIME
```

- `PID`: Integer process ID
- `NAME`: Scheduling policy (`RR`, `FIFO`, or `NORMAL`)
- `STATIC_PRIO`: Static priority (integer)
- `DYNAMIC_PRIO`: Dynamic priority (integer)
- `REMAIN_TIME`: Remaining execution time (integer, ms)

**Example:**
```
1,RR,90,100,200
2,FIFO,110,120,150
3,NORMAL,135,140,100
```

---

## How to Build

Compile with gcc and pthread support:
```sh
gcc -o scheduler Assingment3.c -lpthread
```

---

## How to Run

1. Make sure `process_data.txt` is in the same directory as the executable.
2. Run the program:
   ```sh
   ./scheduler
   ```

---

##  How It Works

- The **producer thread** reads each process from `process_data.txt` and places it in the correct ready queue based on its static priority.
- The **consumer threads** simulate process execution, updating priorities and remaining time, and re-queueing or finishing processes as needed.
- The simulation ends when all queues are empty.

---

##  Author

Hakeem Khan
