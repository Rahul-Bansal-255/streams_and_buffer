#include <stdio.h> // printf() fopen() fclose() fprintf()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()
#include <pthread.h> // pthread_create() pthread_t
#include <signal.h> // signal() kill()
#include <unistd.h> // getpid() pid_t fork()
#include <time.h>

int number_of_keys = 1000;//KEYS

// key_size includes an end line character '\n'
int key_size = 1000;//CHAR 

int buffer_size = 10;//KEYS

int delay_producer = 0;//NANO SEC

int delay_consumer = 0;//NANO SEC

pid_t pid = 0;//PROCESS ID

// charset is used in choosing a random char
char charset[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

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
        else if(!strcmp(argv[i], "--key_size"))
        {
            key_size = atoi(argv[++i]);
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

    printf("Setting Handlers\n");
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, sigint_handler);

    printf("Creating generator thread\n");

    pthread_t generator_thread_id;
    int generator_thread_arg = 0;

    int generator_pthread_create_return_value = 
        pthread_create(&generator_thread_id, NULL,  generator, &generator_thread_arg);

    printf("generator_pthread_create_return_value %d\n", generator_pthread_create_return_value);

    int generator_pthread_join_return_value = 
        pthread_join(generator_thread_id, NULL);

    printf("generator_pthread_join_return_value %d\n", generator_pthread_join_return_value);

    return 0;
    
}

struct Item
{
    char* data;
    pid_t pid;
    time_t creation_time;
};


void sigusr1_handler(int num)
{
    printf("sigusr1_handler\n");
    int producer_pid = fork();
    if(producer_pid == 0)
    {
        producer();
        return;
    }
    int consumer_pid = fork();
    if(consumer_pid == 0)
    {
        consumer();
        return;
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
    printf("generator\n");

    FILE* fptr;
    fptr = fopen("original.txt", "w");

    char str[key_size + 1];
    for(int i = 0; i < number_of_keys; i++)
    {
        random_string(str, key_size);
        fprintf(fptr, "%s", str);
    }
    fclose(fptr);
    kill(pid, SIGUSR1);
    return NULL;
}

void start_buffering()
{
    // Circular Buffer Memory
    struct Item ** buffer = (struct Item**)malloc(sizeof(struct Item) * buffer_size);

}

void producer()
{
    FILE* fptr;
    fptr = fopen("original.txt", "r");

    fclose(fptr);
}

void consumer()
{
    FILE* fptr;
    fptr = fopen("duplicate.txt", "w");

    fclose(fptr);
}