#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

#define CONTINUE  0
#define TERMINATE 1
#define PROCESS   2

/* Functions' Prototypes. */
void *work(void *args);
int   prime(int number);

/* Structs. */
typedef struct thread_info {
    int             number;
	int             signal;
    pthread_mutex_t monitor;
   	pthread_cond_t  available;
   	pthread_cond_t  process;
	pthread_cond_t  ready;
	pthread_cond_t  finish;
	pthread_cond_t  start;
    pthread_cond_t  worker_term;
	pthread_cond_t  wakeup;
} THREAD_INFO;

int main(int argc, char **argv) {
    int num_of_workers, i;
    THREAD_INFO threads_info;
    
    printf("[*] Starting program.\n");
    sleep(2);
    if (argc != 2) {
        fprintf(stderr, "[!] ERROR: usage: ./exercise2 <num_of_workers>\n");
        exit(EXIT_FAILURE);
    }
    
    printf("[*] Initializing monitor and condition variables.\n");
    sleep(2);
    num_of_workers = atoi(argv[1]);
	threads_info.signal = CONTINUE;
    pthread_mutex_init(&threads_info.monitor, NULL);
	pthread_cond_init(&threads_info.available, NULL);
	pthread_cond_init(&threads_info.process, NULL);
	pthread_cond_init(&threads_info.wakeup, NULL);
	pthread_cond_init(&threads_info.worker_term, NULL);
	pthread_cond_init(&threads_info.finish, NULL);
	pthread_cond_init(&threads_info.start, NULL);
	pthread_cond_init(&threads_info.ready, NULL);

	pthread_t thread_IDs[num_of_workers];
    for(i = 0; i < num_of_workers; i++) {
        pthread_create(&thread_IDs[i], NULL, &work, (void *) &threads_info);
    }
	threads_info.signal = PROCESS;
    printf("[*] Ready to read from stdin.\n\n");
    while (1) {
        if (scanf("%d", &threads_info.number) == EOF) break;
    	pthread_cond_signal(&threads_info.start);
		pthread_cond_wait(&threads_info.available, &threads_info.monitor);
		pthread_cond_signal(&threads_info.wakeup);
		pthread_cond_wait(&threads_info.ready, &threads_info.monitor);
		pthread_cond_signal(&threads_info.process);
	}
	
	threads_info.signal = TERMINATE;

	/* Wait for all workers to become available. */
	for (i = 0; i < num_of_workers; i++) {
		pthread_cond_signal(&threads_info.start);
		pthread_cond_wait(&threads_info.available, &threads_info.monitor);   
		pthread_cond_signal(&threads_info.wakeup);
	}
	
    /* Notify workers to terminate. */
    for (i = 0; i < num_of_workers; i++)
		pthread_cond_signal(&threads_info.wakeup);   
 
    /* Wait for all workers to terminate. */
    for (i = 0; i < num_of_workers; i++) {
		pthread_cond_signal(&threads_info.finish); 
		pthread_cond_wait(&threads_info.worker_term, &threads_info.monitor);	
	}
	
	sleep(2);
    printf("\n[*] Done.\n");
    return 0;
}

void *work(void *args) {
    THREAD_INFO *argument = (THREAD_INFO *) args;
    pthread_mutex_lock(&argument->monitor);
	while (1) {
		pthread_cond_wait(&argument->start, &argument->monitor);
		pthread_cond_signal(&argument->available);
        int temp_number = argument->number;
		
		while(argument->signal) {
			pthread_cond_wait(&argument->wakeup, &argument->monitor);
			pthread_cond_signal(&argument->ready);
			if (argument->signal == PROCESS) {
				pthread_cond_wait(&argument->process, &argument->monitor);
				break;
			}
			else if (argument->signal == TERMINATE) {
				pthread_cond_wait(&argument->finish, &argument->monitor); 
				pthread_cond_signal(&argument->worker_term);
				pthread_mutex_unlock(&argument->monitor);
				return NULL;
			}
   		}

		/* Processing assigned value. */    
		if (prime(temp_number)) 
            printf("%d: prime number\n", temp_number);
        else
            printf("%d: not prime number\n", temp_number);
    }
    //pthread_cond_signal(&argument->worker_term); /* Send a termination signal. */
	//pthread_mutex_unlock(&argument->monitor);
    //return NULL;
}

int prime(int number) {
	int i;
    for (i = 2; i <= sqrt(number); i++) {
        if (number % i == 0) return 0;
    }
    if (number <= 1) return 0;
    return 1;
}
