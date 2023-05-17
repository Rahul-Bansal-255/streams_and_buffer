#include <stdio.h> // printf() fopen() fclose() fprintf()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()
#include <pthread.h> // pthread_create() pthread_t
#include <signal.h> // signal() kill()
#include <unistd.h> // getpid() pid_t fork() getcwd()
#include <time.h> // time_t
#include <sys/ipc.h> // ftok()
#include <sys/shm.h> // shmget()

int number_of_keys = 10;//KEYS
int buffer_size = 3;//KEYS
int delay_producer = 0;//NANO SEC
int delay_consumer = 0;//NANO SEC

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
        else if(!strcmp(argv[i], "--delay_producer"))
        {
            delay_producer = atoi(argv[++i]);
        }
        else if(!strcmp(argv[i], "--delay_consumer"))
        {
            delay_consumer = atoi(argv[++i]);
        }
    }

    // Setting Signal Handlers
    signal(SIGUSR1, sigusr1_handler);
    // signal(SIGUSR2, sigusr2_handler_parent);

    // Creating And Joining Generator Thread
    pthread_t generator_thread_id;
    pthread_create(&generator_thread_id, 0,  generator, 0);
    pthread_join(generator_thread_id, 0);

    return 0;
    
}

struct Item
{
    char data[10];
    pid_t pid;
    time_t creation_time;
};


void sigusr1_handler(int num)
{
    create_buffer();

    producer_pid = fork();
    if(producer_pid == 0)
    {
        producer();
        exit(0);
    }
    consumer_pid = fork();
    if(consumer_pid == 0)
    {
        sleep(1);
        consumer();
        exit(0);
    }
}

void random_string(char str[], unsigned int str_size)
{
    for(int i = 0; i <= str_size - 2; i++)
    {
        str[i] = charset[rand() % 62];
    }
    str[str_size - 1] = '\n';
    str[str_size] = '\0';
    return;
}

void* generator(void* arg)
{
    FILE* fptr;
    fptr = fopen("original.txt", "w");

    char str[1001];
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
    char cwd[4096];
    buffer_path(cwd, 4096);
    key_t sysvipc_key_a = ftok(cwd, 'a');
    int shm_id_a = shmget(sysvipc_key_a, sizeof(struct Item) * buffer_size, IPC_CREAT);
    key_t sysvipc_key_b = ftok(cwd, 'b');
    int shm_id_b = shmget(sysvipc_key_b, sizeof(int) * buffer_size, IPC_CREAT);
}

void producer()
{
    char cwd[4096];
    buffer_path(cwd, 4096);

    key_t sysvipc_key_a = ftok(cwd, 'a');
    int shm_id_a = shmget(sysvipc_key_a, sizeof(struct Item) * buffer_size, 0);
    struct Item *buffer = (struct Item*)shmat(shm_id_a, 0, 0);
    
    key_t sysvipc_key_b = ftok(cwd, 'b');
    int shm_id_b = shmget(sysvipc_key_b, sizeof(int) * buffer_size, 0);
    int *mark = (int*)shmat(shm_id_b, 0, 0);

    FILE* fptr;
    fptr = fopen("original.txt", "r");

    int i = 0;

    while(feof(fptr) != 0) 
    {
        while(mark[i] == 1) {}
        mark[i] = 1;
        fgets(buffer[i].data, 10, fptr);
        buffer[i].pid = getpid();
        time(&buffer[i].creation_time);
        mark[i] = 0;
        i++;
        i = i % buffer_size;
    }
    
    fclose(fptr);
}

void consumer()
{
    char cwd[4096];
    buffer_path(cwd, 4096);
    key_t sysvipc_key_a = ftok(cwd, 'a');
    int shm_id_a = shmget(sysvipc_key_a, sizeof(struct Item) * buffer_size, 0);
    struct Item *buffer = (struct Item*)shmat(shm_id_a, 0, 0);
    key_t sysvipc_key_b = ftok(cwd, 'b');
    int shm_id_b = shmget(sysvipc_key_b, sizeof(int) * buffer_size, 0);
    int *mark = (int*)shmat(shm_id_b, 0, 0);
    
    FILE* fptr;
    fptr = fopen("duplicate.txt", "w");

    int i = 0;
    int count = 0;

    while(count <= number_of_keys) 
    {
        while(mark[i] == 1) {}
        mark[i] = 1;
        fprintf(fptr, buffer[i].data);
        mark[i] = 0;
        i++;
        count++;
        i = i % buffer_size;
    }

    fclose(fptr);
}