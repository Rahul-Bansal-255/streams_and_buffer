#include <stdio.h> // printf() fopen() fclose()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()
#include <pthread.h> // pthread_create() pthread_t
#include <signal.h> // signal() kill()
#include <unistd.h> // getpid()

int number_of_keys = 1000; // 1000 KEYS
int key_size = 1000; // 1000 CHAR 
int buffer_size = 10; // 10 KEYS
int delay_producer = 0; // 0 NANO SEC
int delay_consumer = 0; // 0 NANO SEC
int pid = 0;

char CHARSET[] = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

struct BufferNode;

void sigusr1_handler(int num);
void sigint_handler(int num);
void random_string(char str[], unsigned int str_size);
void* generator(void* arg);

int main(int argc, char* argv[])
{
    pid = getpid();
    
    for(int i = 1; i < argc; i++) // Command Line Arguments
    {
        if(strcmp(argv[i], "--number_of_keys"))
        {
            number_of_keys = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--key_size"))
        {
            key_size = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--buffer_size"))
        {
            buffer_size = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--delay_producer"))
        {
            delay_producer = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--delay_consumer"))
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

    printf("%d\n", generator_pthread_create_return_value);

    int generator_pthread_join_return_value = 
        pthread_join(generator_thread_id, NULL);

    printf("%d\n", generator_pthread_join_return_value);

    return 0;
    
}

struct BufferNode
{
    char* data;
    struct BufferNode* next;
};


void sigusr1_handler(int num)
{
    printf("sigusr1_handler\n");
}

void sigint_handler(int num)
{
    printf("sigint_handler\n");
}

void random_string(char str[], unsigned int str_size)
{
    for(int i = 0; i <= str_size - 2; i++)
    {
        str[i] = CHARSET[rand() % 62];
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
    struct BufferNode* node = (struct BufferNode*)malloc(sizeof(struct BufferNode));
    (*node).data = NULL;
    struct BufferNode* iter = node;
    for(int i = 1; i < buffer_size; i++) {
        (*iter).next = (struct BufferNode*)malloc(sizeof(struct BufferNode));
        iter = (*iter).next;
        (*iter).data = NULL;
    }
    (*iter).next = node;

}

void* producer(void* arg)
{
    FILE* fptr;
    fptr = fopen("original.txt", "r");

    fclose(fptr);
}

void* consumer(void* arg)
{
    FILE* fptr;
    fptr = fopen("duplicate.txt", "w");

    fclose(fptr);
}