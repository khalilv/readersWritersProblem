#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

static sem_t rw_mutex;
static sem_t mutex;

static int glob = 0;       //SHARED VARIABLE FOR READERS AND WRITERS
static int read_count = 0; //NUM OF READERS READING THE VARIABLE AT THE SAME TIME

static double writers_time_max[10]; //ARRAYS TO HOLD THE MAX,MIN AND AVERAGE VALUES OF EACH READER AND WRITER THREAD
static double writers_time_min[10];
static double writers_time_avg[10];
static double readers_time_max[500];
static double readers_time_min[500];
static double readers_time_avg[500];

static int readersTotal = 0; //THE TOTAL # OF TIMES THE GLOBAL VARIABLE IS ACCESSED BY THE READERS
static int writersTotal = 0; //THE TOTAL # OF TIMES THE GLOBAL VARIABLE IS ACCESSED BY THE WRITERS

static void *reader_func(void *args)
{
    int *tmp = ((int *)args);
    int readerloops = *tmp;
    double min = TMP_MAX;
    double max = 0;
    double avg = 0;
    int index = *(tmp + 1);

    clock_t t;
    do
    {
        t = clock(); //START TIMER
        sem_wait(&mutex);

        //CRITICAL SEC 1 STARTS
        read_count++;
        readersTotal++;

        if (read_count == 1)
        {
            //IF YOU ARE THE ONLY READER TRY TO ACCESS GLOB VARIABLE
            sem_wait(&rw_mutex);
            //CRITICAL SEC 2 STARTS
        }

        sem_post(&mutex); //CRITICAL SEC 1 ENDS
        t = clock() - t;  //END TIMER

        printf("The value of the global variable is %d\n", glob); //READ VALUE

        //CRITICAL SEC 2 ENDS
        sem_wait(&mutex);
        read_count--;
        if (read_count == 0)
        {
            sem_post(&rw_mutex); //YOU ARE THE LAST READER. RELEASE SEMAPHORE SO THAT WRITERS CAN ACCESS
        }
        sem_post(&mutex);

        //CALCULATE AVERAGE,MAX AND MIN WAIT TIMES
        avg += t;
        if (t < min)
        {
            min = t;
        }
        if (t > max)
        {
            max = t;
        }

        readerloops--;

        srand(time(NULL));
        int sleeptime = (rand() % 100 + 1) * 1000;
        usleep(sleeptime);

    } while (readerloops > 0);
    //STORE AVERAGE,MIN.MAX WAIT TIMES IN READERS ARRAYS. TIMES ARE IN MICROSECONDS
    readers_time_max[index] = max * 1000000 / CLOCKS_PER_SEC;
    readers_time_min[index] = min * 1000000 / CLOCKS_PER_SEC;
    readers_time_avg[index] = avg * 1000000 / CLOCKS_PER_SEC;

    return NULL;
}

static void *writer_func(void *args)
{
    int *tmp = ((int *)args);
    int writerloops = *tmp;
    double min = TMP_MAX;
    double max = 0;
    double avg = 0;
    int index = *(tmp + 1);
    clock_t t;

    do
    {
        t = clock(); //START CLOCK

        sem_wait(&rw_mutex);
        //CRITICAL SECTION STARTS

        glob = glob + 10; //INCREMENT GLOBAL VARIABLE
        writersTotal++;

        sem_post(&rw_mutex); //CRITICAL SECTION ENDS

        t = clock() - t; //STOP CLOCK

        writerloops--;

        //CALCULATE AVERAGE, MIN, MAX WAIT TIMES
        avg += t;
        if (t < min)
        {
            min = t;
        }
        if (t > max)
        {
            max = t;
        }

        srand(time(NULL));
        int sleeptime = (rand() % 100 + 1) * 1000;
        usleep(sleeptime);

    } while (writerloops > 0);

    //STORE AVERAGE,MIN.MAX WAIT TIMES IN WRITERS ARRAYS. TIMES ARE IN MICROSECONDS
    writers_time_max[index] = max * 1000000 / CLOCKS_PER_SEC;
    writers_time_min[index] = min * 1000000 / CLOCKS_PER_SEC;
    writers_time_avg[index] = avg * 1000000 / CLOCKS_PER_SEC;
    return NULL;
}

//HELPER FUNCTION TO CALCULATE READERS MAX,MIN,AVG WAIT TIMES
int calcReadersTimes()
{
    double max = 0;
    double min = TMP_MAX;
    double sum = 0;
    for (int i = 0; i < 500; i++)
    {
        sum += readers_time_avg[i];
        if (readers_time_max[i] > max)
        {
            max = readers_time_max[i];
        }
        if (readers_time_min[i] < min)
        {
            min = readers_time_min[i];
        }
    }

    double average = sum / readersTotal;

    printf("\nMAXIMUM READER TIME WAS %f\n", max);
    printf("MINIMUM READER TIME WAS %f\n", min);
    printf("AVERAGE READER TIME WAS %f\n\n", average);
}

//HELPER FUNCTION TO CALCULATE WRITERS MAX,MIN,AVG WAIT TIMES
int calcWritersTimes()
{
    double max = 0;
    double min = TMP_MAX;
    double sum = 0;
    for (int i = 0; i < 10; i++)
    {
        sum += writers_time_avg[i];
        if (writers_time_max[i] > max)
        {
            max = writers_time_max[i];
        }
        if (writers_time_min[i] < min)
        {
            min = writers_time_min[i];
        }
    }

    double average = sum / writersTotal;

    printf("MAXIMUM WRITER TIME WAS %f\n", max);
    printf("MINIMUM WRITER TIME WAS %f\n", min);
    printf("AVERAGE WRITER TIME WAS %f\n\n", average);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Incorrect number of inputs. Program Terminating\n");
        exit(1);
    }

    pthread_t readers[500];
    pthread_t writers[10];
    int reader_repeat_count = atoi(argv[2]);
    int writer_repeat_count = atoi(argv[1]);
    int writeParameters[10][2];
    int readParameters[500][2];

    //INITIALIZE SEMAPHORES
    if (sem_init(&rw_mutex, 0, 1) == -1)
    {
        printf("Error initializing rw_mutex semaphore\n");
        exit(1);
    }

    if (sem_init(&mutex, 0, 1) == -1)
    {
        printf("Error initializing mutex semaphore\n");
        exit(1);
    }

    //CALL ON WRITER FUNCTION WITH ALL WRITER THREADS
    for (int i = 0; i < 10; i++)
    {
        writeParameters[i][0] = writer_repeat_count;
        writeParameters[i][1] = i;

        if (pthread_create(&writers[i], NULL, writer_func, &writeParameters[i]) != 0)
        {
            printf("Error creating writer thread\n");
            exit(1);
        }
    }

    //CALL ON READER FUNCTION WITH ALL READER THREADS
    for (int j = 0; j < 500; j++)
    {
        readParameters[j][0] = reader_repeat_count;
        readParameters[j][1] = j;
        if (pthread_create(&readers[j], NULL, reader_func, &readParameters[j]) != 0)
        {
            printf("Error creating reader thread\n");
            exit(1);
        }
    }

    //JOIN READER THREADS
    for (int i = 0; i < 500; i++)
    {
        if (pthread_join(readers[i], NULL) != 0)
        {
            printf("Error joining reader threads\n");
            exit(1);
        }
    }
    //JOIN WRITER THREADS
    for (int i = 0; i < 10; i++)
    {
        if (pthread_join(writers[i], NULL) != 0)
        {
            printf("Error joining writer threads\n");
            exit(1);
        }
    }
    //CALCULATE READERS AND WRITERS WAIT TIMES RESPECTIVELY
    calcReadersTimes();
    calcWritersTimes();
    return 0;
}
