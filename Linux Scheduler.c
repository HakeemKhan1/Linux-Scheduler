#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NUM_THREADS 3
#define MAX_SIZE 512
#define BASE_QUANTUM 20

enum sched_policy
{
    RR,
    FIFO,
    NORMAL,
};

struct ProcessInfo
{
    int PID;
    char name[10];
    int static_prio;
    int dynamic_prio;
    int remain_time;
    int time_slice;
    int accu_time_slice;
    int last_cpu;
    enum sched_policy sched;
};

struct ProcessInfo RQ0[MAX_SIZE];
struct ProcessInfo RQ1[MAX_SIZE];
struct ProcessInfo RQ2[MAX_SIZE];

int RQ0_count = 0;
int RQ1_count = 0;
int RQ2_count = 0;

pthread_mutex_t mutex_RQ0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_RQ1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_RQ2 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;

int is_queue_empty(struct ProcessInfo queue[], int count)
{
    return count == 0;
}

void *producer_function(void *arg);
void *consumer_function(void *arg);
int max(int a, int b);
int min(int a, int b);
int random_range(int min, int max);

void insert_into_queue(struct ProcessInfo process, struct ProcessInfo queue[], int *count, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    if (*count < MAX_SIZE)
    {
        queue[*count] = process;
        (*count)++;
    }
    pthread_mutex_unlock(mutex);
}

struct ProcessInfo remove_highest_priority(struct ProcessInfo queue[], int *count, pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
    struct ProcessInfo highest_priority_process = queue[0];
    for (int i = 1; i < *count; i++)
    {
        queue[i - 1] = queue[i];
    }
    (*count)--;
    pthread_mutex_unlock(mutex);
    return highest_priority_process;
}

int main()
{
    srand(time(NULL)); // Initialize random number generator

    pthread_t producer_thread;
    pthread_t consumer_threads[NUM_THREADS];

    int res;

    res = pthread_create(&producer_thread, NULL, producer_function, NULL);
    if (res != 0)
    {
        perror("Producer thread creation failed");
        exit(EXIT_FAILURE);
    }

    sleep(3);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        int *thread_id = malloc(sizeof(int));
        if (thread_id == NULL)
        {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        *thread_id = i;
        res = pthread_create(&consumer_threads[i], NULL, consumer_function, (void *)thread_id);
        if (res != 0)
        {
            perror("Consumer thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    pthread_join(producer_thread, NULL);

    // Continue until all queues are empty
    while (!(is_queue_empty(RQ0, RQ0_count) && is_queue_empty(RQ1, RQ1_count) && is_queue_empty(RQ2, RQ2_count)))
    {
        // Sleep for a while to allow consumers to finish
        usleep(500000);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(consumer_threads[i], NULL);
    }

    return 0;
}

void *producer_function(void *arg)
{
    FILE *file = fopen("process_data.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        struct ProcessInfo new_process;
        sscanf(line, "%d,%[^,],%d,%d,%d", &new_process.PID, new_process.name, &new_process.static_prio,
               &new_process.dynamic_prio, &new_process.remain_time);

        // Additional modification to read scheduling policy
        if (strcmp(new_process.name, "RR") == 0)
        {
            new_process.sched = RR;
        }
        else if (strcmp(new_process.name, "FIFO") == 0)
        {
            new_process.sched = FIFO;
        }
        else
        {
            new_process.sched = NORMAL;
        }

        pthread_mutex_lock(&producer_mutex);

        usleep(1000000);

        pthread_mutex_unlock(&producer_mutex);

        if (new_process.static_prio < 100)
        {
            insert_into_queue(new_process, RQ0, &RQ0_count, &mutex_RQ0);
        }
        else if (new_process.static_prio < 130)
        {
            insert_into_queue(new_process, RQ1, &RQ1_count, &mutex_RQ1);
        }
        else
        {
            insert_into_queue(new_process, RQ2, &RQ2_count, &mutex_RQ2);
        }
    }

    fclose(file);

    printf("Producer thread finished. Exiting.\n");

    pthread_exit(NULL);
}

void *consumer_function(void *arg)
{
    int thread_id = *(int *)arg;
    free(arg);

    while (1)
    {
        struct ProcessInfo current_process;

        if (RQ0_count > 0)
        {
            current_process = remove_highest_priority(RQ0, &RQ0_count, &mutex_RQ0);
        }
        else if (RQ1_count > 0)
        {
            current_process = remove_highest_priority(RQ1, &RQ1_count, &mutex_RQ1);
        }
        else if (RQ2_count > 0)
        {
            current_process = remove_highest_priority(RQ2, &RQ2_count, &mutex_RQ2);
        }
        else
        {
            usleep(500000);
            continue;
        }

        // Calculate the quantum size based on static priority
        int quantum;
        if (current_process.static_prio < 120) {
            quantum = (140 - current_process.static_prio) * BASE_QUANTUM;
        } else {
            quantum = (140 - current_process.static_prio) * 5;
        }

        // Simulate the execution time for this iteration
        int simulated_execution_time = random_range(1, 10) * 10;

        // Check if the simulated execution time is less than the quantum
        if (simulated_execution_time < quantum) {
            // Apply a bonus for dynamic priority calculation
            current_process.dynamic_prio += 10;
        } else {
            current_process.dynamic_prio += 5;
        }

        // Update time_slice with the calculated quantum
        current_process.time_slice = quantum;

        // Sleep for the time_slice to simulate process execution
        usleep(current_process.time_slice * 1000); // usleep takes microseconds

        // Check if the process did not use the entire time slice
        if (simulated_execution_time < quantum) {
            // Simulate the process going to sleep for 1000ms
