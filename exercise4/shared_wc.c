#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define MEN        1
#define WOMEN      0
#define TIME_IN_WC 2

typedef struct shared { 
    pthread_mutex_t monitor;
    pthread_cond_t  men;
    pthread_cond_t  women;
    pthread_cond_t  isDone;
    int people;
    int capacity;
    int menw, womenw;        // men and women waiting in queue
    int men_in, women_in;    // men and women inside WC (CS)
    int type;                // men or women
} shared;

void entry_woman(void* args){
    shared *arguments = (void *) args;

    pthread_mutex_lock(&arguments->monitor);
    while (1) {
	if(arguments->men_in > 0 || arguments->women_in == arguments->capacity){    
       	    arguments->womenw++;
       	    pthread_cond_wait(&arguments->women,&arguments->monitor);
    	}
    	if((arguments->women_in < arguments->capacity) && (arguments->men_in == 0)) {
    	    arguments->women_in++;
            break;
    	}
	else {
	    pthread_cond_wait(&arguments->women,&arguments->monitor);
	}
    }
    pthread_mutex_unlock(&arguments->monitor);
    return;
}

void exit_woman(void* args){
    shared *arguments = (void *) args;

    pthread_mutex_lock(&arguments->monitor);
    arguments->women_in--;
    arguments->people--;
	    
    if (arguments->women_in == 0) {
	if (arguments->womenw > 0) {
	    arguments->womenw--;
	    pthread_cond_signal(&arguments->women);
	}
	if (arguments->menw > 0) {
       	    arguments->menw--;
            pthread_cond_signal(&arguments->men);
    	}
    }
    pthread_cond_signal(&arguments->women);
    
    /* Last one to inform main. */
    if(arguments->men_in == 0 && arguments->menw == 0 && arguments->women_in == 0 && arguments->womenw == 0){
        pthread_cond_signal(&arguments->isDone);
        pthread_mutex_unlock(&arguments->monitor);
        return;    
    }
    pthread_cond_signal(&arguments->isDone);
    pthread_mutex_unlock(&arguments->monitor);
    return;
}

void entry_man(void*args){
    shared* arguments = (void*)args;
    
    pthread_mutex_lock(&arguments->monitor);
    while (1) {
    	if ((arguments->women_in > 0) || (arguments->men_in == arguments->capacity)) {
            arguments->menw++;
            pthread_cond_wait(&arguments->men,&arguments->monitor);
    	}
    	if((arguments->men_in < arguments->capacity) && (arguments->women_in == 0)) {
            arguments->men_in++;
	    break;
    	}
	else {
            pthread_cond_wait(&arguments->men,&arguments->monitor);
	}
    }
    pthread_mutex_unlock(&arguments->monitor);
    return;
}


void exit_man(void* args){
    shared *arguments = (void *) args;

    pthread_mutex_lock(&arguments->monitor);
    arguments->men_in--;
    arguments->people--;

    if (arguments->men_in == 0) {
    	if (arguments->womenw > 0) {
            arguments->womenw--;
            pthread_cond_signal(&arguments->women);
        }
        if (arguments->menw > 0) {
            arguments->menw--;
            pthread_cond_signal(&arguments->men);
       	}
    }
    pthread_cond_signal(&arguments->men);
    
    /* Last one to inform main. */
    if(arguments->men_in == 0 && arguments->menw == 0 && arguments->women_in == 0 && arguments->womenw == 0){
        pthread_cond_signal(&arguments->isDone);
        pthread_mutex_unlock(&arguments->monitor);
        return;    
    }
    pthread_cond_signal(&arguments->isDone); 
    pthread_mutex_unlock(&arguments->monitor);  
    return;
}


void* people_fun(void* args){
    shared *arguments = (void *) args;

    /* Men code. */
    if(arguments->type == MEN){
        entry_man(arguments);
        printf("Man <%ld> heading in WC.\n",pthread_self());
        sleep(TIME_IN_WC);
        printf("Man <%ld> heading out of WC.\n",pthread_self());
        exit_man(arguments);
        return NULL;
    }
    
    /* Woman code. */
    entry_woman(arguments);
    printf("Woman <%ld> heading in WC.\n",pthread_self());
    sleep(TIME_IN_WC);
    printf("Woman <%ld> heading out of WC.\n",pthread_self());
    exit_woman(arguments);
    return NULL;
}


int main(int argc,char* argv[]){
    shared info;  
    int i;
    int delay, num_of_people;

    printf("[*] Starting program.\n");
    sleep(2);

    if(argc != 2){
        fprintf(stderr,"[!] ERROR: usage: ./shared_wc <capacity>\n");
        exit(EXIT_FAILURE);
    }
    printf("[*] Initializing structures and variables.\n");
    sleep(2);
 
    info.people = 0; 
    info.capacity = atoi(argv[1]);
    pthread_mutex_init(&info.monitor,NULL);
    pthread_cond_init(&info.men,NULL);
    pthread_cond_init(&info.women,NULL);
    pthread_cond_init(&info.isDone,NULL);
    info.women_in = 0;
    info.men_in = 0;
    info.menw = 0;
    info.womenw = 0;
    
    
    printf("[*] Reading from stdin and creating threads.\n");
    sleep(2);	
    while (scanf("%d:%d:%d", &num_of_people, &info.type, &delay) != EOF) {
        pthread_t people_threads[num_of_people];
        info.people += num_of_people;
	for (i = 0; i < num_of_people; i++) {
            pthread_create(&people_threads[i], NULL, people_fun, (void *) &info);
        }
        sleep(delay);
    }
   	
    /* Main waiting to get signaled for termination. */ 
    while (info.people > 0)
        pthread_cond_wait(&info.isDone,&info.monitor); 
    
    printf("[*] Destroying condition variables and monitor and exiting.\n");
    sleep(2);
    pthread_mutex_destroy(&info.monitor);
    pthread_cond_destroy(&info.men);
    pthread_cond_destroy(&info.women);
    pthread_cond_destroy(&info.isDone);
    
    printf("[*] Done.\n");
    sleep(2);
    return 0;
}
