#include <stdio.h> // printf() fopen() fclose() fprintf()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()
#include <pthread.h> // pthread_create() pthread_t
#include <signal.h> // signal() kill()
#include <unistd.h> // getpid() pid_t fork() getcwd()
#include <time.h> // time_t
#include <sys/ipc.h> // ftok()
#include <sys/shm.h> // shmget()

int number_of_keys = 1000;//KEYS
int buffer_size = 10;//KEYS
int delay_producer = 0;//NANO SEC
int delay_consumer = 0;//NANO SEC

pid_t pid = 0;//PROCESS ID

// charset is used in choosing a random char
char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

struct BufferNode;
void sigusr1_handler(int num);
void sigint_handler(int num);
void random_string(char str[], unsigned int str_size);
void* generator(void* arg);

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
    signal(SIGINT, sigint_handler);

    // Creating And Joining Generator Thread
    pthread_t generator_thread_id;
    pthread_create(&generator_thread_id, NULL,  generator, NULL);
    pthread_join(generator_thread_id, NULL);

    return 0;
    
}

struct Item
{
    char data[1000];
    pid_t pid;
    time_t creation_time;
};


void sigusr1_handler(int num)
{
    create_buffer();

    pid_t producer_pid = fork();
    if(producer_pid == 0)
    {
        producer();
        exit(0);
    }
    pid_t consumer_pid = fork();
    if(consumer_pid == 0)
    {
        consumer();
        exit(0);
    }
}

void sigint_handler(int num)
{
    printf("sigint_handler\n");
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
        random_string(str, 1000);
        fprintf(fptr, "%s", str);
    }

    fclose(fptr);

    kill(pid, SIGUSR1);
}

void buffer_path(char *cwd, int cwd_size) {
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
    int shm_id_a = shmget(sysvipc_key_a, sizeof(struct Item) * buffer_size, NULL);
    struct Item buffer[] = (struct Item*)shmat(shm_id_a, NULL, NULL);
    key_t sysvipc_key_b = ftok(cwd, 'b');
    int shm_id_b = shmget(sysvipc_key_b, sizeof(int) * buffer_size, NULL);
    int mark[] = (int*)shmat(shm_id_b, NULL, NULL);

    printf("Inside Producer\n");
    FILE* fptr;
    fptr = fopen("original.txt", "r");

    fclose(fptr);
}

void consumer()
{
    char cwd[4096];
    buffer_path(cwd, 4096);
    key_t sysvipc_key_a = ftok(cwd, 'a');
    int shm_id_a = shmget(sysvipc_key_a, sizeof(struct Item) * buffer_size, NULL);
    struct Item buffer[] = (struct Item*)shmat(shm_id_a, NULL, NULL);
    key_t sysvipc_key_b = ftok(cwd, 'b');
    int shm_id_b = shmget(sysvipc_key_b, sizeof(int) * buffer_size, NULL);
    int mark[] = (int*)shmat(shm_id_b, NULL, NULL);
    
    printf("Inside Consumer\n");
    FILE* fptr;
    fptr = fopen("duplicate.txt", "w");

    fclose(fptr);
}