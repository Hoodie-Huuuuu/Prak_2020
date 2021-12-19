#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

// end by Ctrl+C
// no more processes
// max count of persons is undefined
// exactly CAR_CAPACITY passengers in the car 

int car(void);
int person(int person_no);

enum { CAR_CAPACITY = 5 };

//my GLOBAL variables
int seats[2];
int can_exit[2];
int in_car[2];
int last[2];
//===================

void remove_zombie(int signo)
{
    wait(0);
}

//char mas_seats[CAR_CAPACITY] = {0};

int main(void)
{
    //mas_seats[CAR_CAPACITY - 1] = 1;
    pipe(seats);
    pipe(can_exit);
    pipe(in_car);
    pipe(last);

    srand(time(0));
    signal(SIGCHLD, remove_zombie);

    if (fork() == 0) {
        return car();
    }

    int person_no = 0;
    while (1) {
        sleep(rand() * (5.0 / RAND_MAX));
        if (person_no == INT_MAX) {
            break;
        }
        ++ person_no;
        if (fork() == 0) {
            return person(person_no);
        }
    }
    return 0;
}



int car(void)
{
    #define MESSAGE(str) fprintf(stderr, "Car: " str "\n")
    
    MESSAGE("1. Hi!");

    
    while (1) {
        MESSAGE("2. Please load the car");
    //1. allow passengers to seat and wait while the last passenger will wake up me
        char c = 0;
        int i;
        for (i = 0; i < CAR_CAPACITY - 1; ++i) {//allow passengers to seat
            write(seats[1], &c, sizeof(c));
        }
        c = 1; //last passenger
        write(seats[1], &c, sizeof(c));
        read(last[0], &c, sizeof(c));  //wait while the last passenger will wake up me

        MESSAGE("3. Go!");

        for (i = 0; i < CAR_CAPACITY; ++i) {
            write(in_car[1], &c, sizeof(c)); //all passengers in trip
        }
        sleep(5);

        MESSAGE("4. Trip ends");
    //2. allow passengers to go out and wait while the last passenger will wake up me
        c = 0; 
        for (i = 0; i < CAR_CAPACITY - 1; ++i) { //allow passengers to go out
            write(can_exit[1], &c, sizeof(c));
        }
        c = 1;
        write(can_exit[1], &c, sizeof(c)); //allow LAST passenher to go out
        read(last[0], &c, sizeof(c)); //wait the last passenger to go out
    }
    return 0;

    #undef MESSAGE
}

int person(int person_no)
{
    #define MESSAGE(str) fprintf(stderr, "Person %02d: " str "\n", person_no)

    MESSAGE("1. Hi!"); 
    //1. wait while there is possibility to seat into the car
    char c;
    read(seats[0], &c, sizeof(c));

    //2. occupy the seat in the car (+ wake up the car if I occupy the last seat)
    MESSAGE("2. I'm sitting into the car");
    if (c == 1) { //last seat
        write(last[1], &c, sizeof(c));
    }
    sleep(1);
    read(in_car[0], &c, sizeof(c)); //wait a full car

    MESSAGE("3. I'm in trip!");
    //3. wait while there is possibility to leave the car (+ wake up the car if I leave the last seat)
    read(can_exit[0], &c, sizeof(c)); //

    
    MESSAGE("4. Thanks for the trip. Bye!");
    if (c == 1) {
        write(last[1], &c, sizeof(c)); //last passenger left the car
    }

    return 0;   

    #undef MESSAGE
}