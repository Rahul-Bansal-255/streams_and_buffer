#include <stdio.h> // printf() fopen() fclose() fprintf()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()
#include <pthread.h> // pthread_create() pthread_t
#include <signal.h> // signal() kill()
#include <unistd.h> // getpid() pid_t fork() getcwd()
#include <time.h> // time_t
#include <sys/ipc.h> // ftok()
#include <sys/shm.h> // shmget()
#include <errno.h> // 

int number_of_keys = 10;//KEYS
int buffer_size = 3;//KEYS

pid_t producer_pid;
pid_t consumer_pid;

pid_t pid = 0;//PROCESS ID

// charset is used in choosing a random char
char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";


struct Item;
void sigusr1_handler(int num);
void random_string(char str[], unsigned int str_size);
void* generator(void* arg);
void buffer_path(char *cwd, int cwd_size);
void create_buffer();
void producer();
void consumer();

int main(int argc, char* argv[])
{
    printf("Start\n");
    pid = getpid();
    
    // Command Line Arguments
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "--number_of_keys"))
        {
            number_of_keys = atoi(argv[++i]);
        }
        else if(!strcmp(argv[i], "--buffer_size"))
        {
            buffer_size = atoi(argv[++i]);
        }
    }
    printf("cmd args\n");

    // Setting Signal Handlers
    signal(SIGUSR1, sigusr1_handler);
    // signal(SIGUSR2, sigusr2_handler_parent);
    printf("signal set\n");

    // Creating And Joining Generator Thread
    pthread_t generator_thread_id;
    pthread_create(&generator_thread_id, 0,  generator, 0);
    pthread_join(generator_thread_id, 0);

    while (1) {}
    
    printf("parent end\n");
    return 0;
    
}

struct Item
{
    char data[11];
    pid_t pid;
    time_t creation_time;
    int write;
};


void sigusr1_handler(int num)
{
    printf("inside sigusr1\n");
    create_buffer();

    producer_pid = fork();
    if(producer_pid == 0)
    {
        printf("producer fork\n");
        producer();
        exit(0);
    }
    printf("producer pid : %d\n", producer_pid);
    consumer_pid = fork();
    if(consumer_pid == 0)
    {
        printf("consumer fork\n");
        consumer();
        exit(0);
    }
    printf("consumer pid : %d\n", consumer_pid);
}

void random_string(char str[], unsigned int str_size)
{
    for(int i = 0; i < str_size; i++)
    {
        str[i] = charset[rand() % 62];
    }
    return;
}

void* generator(void* arg)
{
    printf("inside gen\n");
    FILE* fptr;
    fptr = fopen("original.txt", "w");

    char str[10];
    for(int i = 0; i < number_of_keys; i++)
    {
        random_string(str, 10);
        fprintf(fptr, "%s", str);
    }

    fclose(fptr);

    kill(pid, SIGUSR1);
}

void buffer_path(char *cwd, int cwd_size)
{
    getcwd(cwd, cwd_size);
    char str[] = "/buffer";
    int i = 0;
    while(cwd[i] != '\0') 
    {
        i++;
    }
    for(int j = 0; j < sizeof(str); j++)
    {
        cwd[i] = str[j];
        i++;
    }
    cwd[i] = '\0';
}

void create_buffer()
{
    printf("inside create_buffer\n");
    char cwd[4096];
    buffer_path(cwd, 4096);

    key_t key_a = ftok(cwd, getpid());
    int shm_id_a = shmget(key_a, sizeof(struct Item) * buffer_size, 0666|IPC_CREAT);
    struct Item *buffer = (struct Item*)shmat(shm_id_a, 0, 0);

    printf("key_a : %d\n", key_a);
    printf("shm_id_a : %d\n", shm_id_a);

    printf("errno %d\n", errno);

    for(int i = 1; i < buffer_size; i++) 
    {
        buffer[i].write = 1;
    }
}

void producer()
{
    printf("inside producer\n");
    char cwd[4096];
    buffer_path(cwd, 4096);

    key_t key_a = ftok(cwd, getppid());
    int shm_id_a = shmget(key_a, sizeof(struct Item) * buffer_size, 0);
    struct Item *buffer = (struct Item*)shmat(shm_id_a, 0, 0);
    
    printf("producer key_a : %d\n", key_a);
    printf("producer shm_id_a : %d\n", shm_id_a);

    FILE* fptr;
    fptr = fopen("original.txt", "r");

    int i = 0;

    while(feof(fptr) == 0) 
    {
        printf("producer loop %d\n", i);
        while(buffer[i].write == 0) {}
        fgets(buffer[i].data, 11, fptr);
        printf("key : %s\n", buffer[i].data);
        buffer[i].pid = getpid();
        time(&buffer[i].creation_time);
        buffer[i].write = 0;
        i++;
        i = i % buffer_size;
    }
    
    fclose(fptr);
    printf("producer exit\n");
}

void consumer()
{
    printf("inside consumer\n");
    char cwd[4096];
    buffer_path(cwd, 4096);

    key_t key_a = ftok(cwd, getppid());
    int shm_id_a = shmget(key_a, sizeof(struct Item) * buffer_size, 0);
    struct Item *buffer = (struct Item*)shmat(shm_id_a, 0, 0);

    printf("consumer key_a : %d\n", key_a);
    printf("consumer shm_id_a : %d\n", shm_id_a);

    FILE* fptr;
    fptr = fopen("duplicate.txt", "w");

    int i = 0;
    int count = 0;

    while(count <= number_of_keys) 
    {
        printf("consumer loop %d\n", i);
        while(buffer[i].write == 1) {}
        fprintf(fptr, buffer[i].data);
        buffer[i].write = 1;
        i++;
        count++;
        i = i % buffer_size;
    }

    fclose(fptr);
    printf("consumer exit\n");
}