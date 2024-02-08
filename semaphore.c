#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

int counter = 0;
int buffer_size;
int no_of_threads;
int in = 0;
int out = 0;
int no_of_elements = 0;
int t;

sem_t mutex1, mutex2, empty, full;

int* buffer;

void printBuffer()
{
    printf("Buffer: ");
    for(int i = 0; i < buffer_size; i ++)
    {
        printf("%d\t",buffer[i]);
    }
    printf("\nIn = %d\tOut = %d\n",in,out);
}

int randomNo(int x)
{
    if(x == 1)
    {
        return rand() % 5 + 5;
    }
    if(x == 2)
    {
        return rand() % 10 + 1;
    }
    if(x == 3)
    {
        return rand() % 5 + 20;
    }
}

void* mCounter(void* args)
{
    int id = *(int*)args;
    while(1)
    {
	printf("Counter thread %d: recieved a message.\n",id);
        printf("Counter thread %d: waiting to write.\n",id);
        sem_wait(&mutex1);
        counter ++;
        printf("Counter thread %d: now adding to counter, counter value = %d.\n",id,counter);
        sem_post(&mutex1);
        sleep(randomNo(1));
    }
}

void* mMonitor(void* args)
{
    while(1)
    {
        printf("Monitor thread: waiting to read counter.\n");
        while(counter == 0)
        {}
        sem_wait(&mutex1);
        int c = counter;
        counter = 0;
        sem_post(&mutex1);
        if(no_of_elements == buffer_size)
        {
            printf("Monitor thread: Buffer full!!\n");
        }
        sem_wait(&empty);
        sem_wait(&mutex2);
        buffer[in] = c;
        printf("Monitor thread: writing to buffer at position = %d.\n",in);
        in = (in + 1) % buffer_size;
        no_of_elements ++;
        printBuffer();
        sem_post(&mutex2);
        sem_post(&full);
        sleep(randomNo(2));
    }
}

void* mCollector(void* args)
{
    while(1)
    {
        if(no_of_elements == 0)
        {
            printf("Collector thread: nothing is in the buffer!!\n");
        }
        sem_wait(&full);
        sem_wait(&mutex2);
        printf("Collector thread: reading from the buffer at position = %d.\n",out);
        int data = buffer[out];
        out = (out + 1) % buffer_size;
        no_of_elements --;
        sem_post(&mutex2);
        sem_post(&empty);
        sleep(randomNo(3));
    }
}

int main(int argc, char* argv[])
{
    buffer_size = atoi(argv[1]);
    no_of_threads = atoi(argv[2]);
    buffer = (int*)malloc(buffer_size*sizeof(int));
    printf("Buffer Size = %d\tNumber of Threads = %d\n",buffer_size,no_of_threads);
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    sem_init(&empty, 0, buffer_size);
    sem_init(&full, 0, 0);
    pthread_t counters[no_of_threads];
    pthread_t monitor;
    pthread_t collector;
    int thread_no[no_of_threads];
    for(int i = 0; i < no_of_threads; i ++)
    {
    	thread_no[i] = i + 1;
        pthread_create(&counters[i], NULL, mCounter, &thread_no[i]);
    }
    pthread_create(&monitor, NULL, mMonitor, NULL);
    pthread_create(&collector, NULL, mCollector, NULL);
    for(int i = 0; i < no_of_threads; i ++)
    {
        pthread_join(counters[i], NULL);
    }
    pthread_join(monitor, NULL);
    pthread_join(collector, NULL);
    return 0;
}
