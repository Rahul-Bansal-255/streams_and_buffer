#include <stdio.h> // printf() fopen() fclose()
#include <stdlib.h> // atoi() rand()
#include <string.h> // strcmp()

int NUMBER_OF_KEYS = 1000; // 1000 KEYS
int KEY_SIZE = 1000; // 1000 CHAR 
int BUFFER_SIZE = 10; // 10 KEYS
int DELAY_PRODUCER = 0; // 0 NANO SEC
int DELAY_CONSUMER = 0; // 0 NANO SEC

char CHARSET[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

void random_string(char str[], unsigned int str_size);
void generator();

int main(int argc, char* argv[])
{
    // Command Line Arguments
    for(int i = 1; i < argc; i++) 
    {
        if(strcmp(argv[i], "--number_of_keys"))
        {
            NUMBER_OF_KEYS = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--key_size"))
        {
            KEY_SIZE = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--buffer_size"))
        {
            BUFFER_SIZE = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--delay_producer"))
        {
            DELAY_PRODUCER = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "--delay_consumer"))
        {
            DELAY_CONSUMER = atoi(argv[++i]);
        }
    }

    generator();

    return 0;
    
}

void random_string(char str[], unsigned int str_size)
{
    int i;
    for(i = 0; i < str_size; i++)
    {
        str[i] = CHARSET[rand() % 62];
    }
    str[str_size - 2] = '\n';
    str[str_size - 1] = '\0';
    return;
}

void generator()
{
    FILE* fptr;
    fptr = fopen("original.txt", "w");

    char str[KEY_SIZE];
    for(int i = 0; i < NUMBER_OF_KEYS; i++)
    {
        random_string(str, KEY_SIZE);
        fprintf(fptr, "%s", str);
    }
    fclose(fptr);
}