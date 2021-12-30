#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define TIME_OF_THE_RIDE   2
#define STARTING_POSITION  0
#define READY_FOR_NEW_RIDE 1
#define STOP_THE_TRAIN     2
#define START_THE_TRAIN    3

typedef struct train {
    int     num_of_passengers;  /* # of passengers in general. */
    int     maxNum;             /* # of max seats in train. */
    int     currNum;            /* # of passenger in current ride. */
    int     num_of_free_seats;  /* # of free seats in the train. */
    int     passengers_waiting;
    int     counter;        
    int     signal;
    int     num_of_rides;
    pthread_mutex_t monitor;
    pthread_cond_t  train;
    pthread_cond_t  passenger;
    pthread_cond_t  wait;
    pthread_cond_t  done; 	
} my_train;

/* Function prototypes. */
void  train_ride(my_train *arg);
void *train_func(void *arg);
void *passenger_func(void *arg);
void  enter_train(my_train *arg);
void  exit_train(my_train *arg);

int main(int argc, char *argv[]) {
    my_train   train_info;
    pthread_t  train_t;

    printf("[*] Starting program.\n");
    sleep(2);

    if(argc != 3){
        fprintf(stderr,"[!] ERROR: usage: ./train <max_num_of_seats> <num_of_passengers_came>\n");
        exit(EXIT_FAILURE);
    }
    printf("[*] Initializing structures and variables.\n");
    sleep(2);
	
    pthread_mutex_init(&train_info.monitor, NULL);
    pthread_cond_init(&train_info.train, NULL);
    pthread_cond_init(&train_info.passenger, NULL);
    pthread_cond_init(&train_info.wait, NULL);
    pthread_cond_init(&train_info.done, NULL);
    train_info.num_of_rides       = 0;
    train_info.maxNum             = atoi(argv[1]);
    train_info.num_of_passengers  = atoi(argv[2]);
    train_info.currNum            = 0;
    train_info.num_of_free_seats  = train_info.maxNum;
    train_info.passengers_waiting = 0;
    train_info.counter            = 0;
    train_info.signal             = STARTING_POSITION;
    
    printf("[*] Creating train thread.\n");
    sleep(2);
    pthread_create(&train_t, NULL, train_func, (void*) &train_info);

    pthread_t pass[train_info.num_of_passengers];
    printf("[*] Creating passengers threads.\n");
    sleep(2);
    for (int i = 0; i < train_info.num_of_passengers; i++) {
        pthread_create(&pass[i], NULL, passenger_func, (void*) &train_info);
    }
	
    /* Waiting for train's thread to terminate. */
    while(train_info.num_of_passengers > 0)
	pthread_cond_wait(&train_info.done, &train_info.monitor);

    printf("[*] Done.\n");
    sleep(1);
    return 0;
}

void train_ride(my_train *arg) {
    pthread_mutex_lock(&arg->monitor);
    if (arg->num_of_free_seats != 0 || arg->signal != STARTING_POSITION) {
	if (arg->signal == STOP_THE_TRAIN) {
	    pthread_mutex_unlock(&arg->monitor);
	    return;
	}
	if (arg->signal != START_THE_TRAIN) {
	    pthread_cond_wait(&arg->train, &arg->monitor);
        }
    }
	
    if (arg->num_of_passengers == 0) {
	printf("Train is out of order for today. Bye.\n");
	sleep(1);
	printf("[*] Sending termination signal.\n");
	sleep(1);
	pthread_cond_signal(&arg->done);
	pthread_mutex_unlock(&arg->monitor);
	return;
    }
	
    arg->num_of_rides++;
    printf("Train starting its ride.\n");
    sleep(TIME_OF_THE_RIDE);
    printf("Ride is finished.\n");
	
    arg->signal = READY_FOR_NEW_RIDE;
    arg->currNum = 0;
	
    for (int i = 0; i < arg->maxNum; i++) 
        pthread_cond_signal(&arg->passenger);
   
   pthread_mutex_unlock(&arg->monitor);
   return;
}

void *train_func(void *args) {
    my_train *arg = (my_train *) args;
    while (arg->num_of_passengers > 0) {
	train_ride(arg);
    }
    return NULL;
}

void enter_train(my_train *arg) {
    pthread_mutex_lock(&arg->monitor);
    while(1) {
	arg->currNum++;	
	arg->num_of_free_seats--;
	if (arg->num_of_free_seats == 0) {  /* This passengers is the last allowed to enter. */
	    printf("Passenger <%ld> enters.\n", pthread_self());
            sleep(1);
	    arg->signal = START_THE_TRAIN;
	    pthread_cond_signal(&arg->train); /* Start the train. */
	    pthread_cond_wait(&arg->passenger, &arg->monitor); /* Wait until ride is finished. */ 
            arg->signal = READY_FOR_NEW_RIDE;
	    break;
        }
	else if (arg->num_of_free_seats > 0) {
	    printf("Passenger <%ld> enters.\n", pthread_self());
	    sleep(1);
	    pthread_cond_wait(&arg->passenger, &arg->monitor); /* wait until ride is finished. */
	    break;
	}
	else {
	    arg->passengers_waiting++;
            pthread_cond_wait(&arg->wait, &arg->monitor);
	}
    }
    pthread_mutex_unlock(&arg->monitor);
    return;
}

void exit_train(my_train *arg) {
    pthread_mutex_lock(&arg->monitor);
    printf("Passenger <%ld> got off the train.\n", pthread_self());
    arg->num_of_passengers--;
    arg->counter++;
    if(arg->counter == arg->maxNum) {
	arg->num_of_free_seats = arg->maxNum;		
	
	/* Wake up the next set of passengers. */
	if (arg->num_of_passengers > 0) {
	    for (int i = 0; i < arg->maxNum; i++) {
		pthread_cond_signal(&arg->wait);
	    }
	}
	arg->counter = 0;
	printf("Passengers waiting: %d\n", arg->num_of_passengers);
	arg->signal = READY_FOR_NEW_RIDE;
    }
    if (arg->num_of_passengers == 0) {
	arg->signal = STOP_THE_TRAIN;
	pthread_cond_signal(&arg->train);
    }
    pthread_mutex_unlock(&arg->monitor);
    return;
}

void *passenger_func(void *arg) {
    my_train *args = (my_train *) arg;
    enter_train(args);
    exit_train(args);
    return NULL;
}
