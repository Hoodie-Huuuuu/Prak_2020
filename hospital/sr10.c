#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>


// end by Ctrl+C
// no more processes
// max count of patients is undefined
// don't change the protocol of getting to the queue

// doctor can talk with only one patient
// doctor mustn't handle the requests "who is the latest"
// doctor mustn't handle the patients queue

// almost all time patients sleep in queue and eventually:
//      - check if the previous patient is existed
//      - answer to requests "who is the latest"

// please don't use arrays of pipes
// don't use pipes for long-time storing the information
// (but you can use pipe for transferring
//  the complex information between processes)

// don't use active waiting

// don't increase zombies
// freezing by Ctrl+Z and the following "fg" command mustn't cause the deadlock


//============
//GLOBAL
int next_pat[2];
int queue[2];
int previous[2];
int wait_doc_free[2];


enum
{
    NOBODY = 1,
};


int doctor(void);
int patient(int patient_no);


int main(void)
{   
    pipe(queue);
    pipe(next_pat);
    pipe(previous);
    pipe(wait_doc_free);

    char c = 1;
    write(queue[1], &c, sizeof(char));//чтобы мог зайти первый пациент
    write(wait_doc_free[1], &c, sizeof(char));

    
    signal(SIGUSR1, SIG_IGN);//крик: "кто последний?""
    signal(SIGUSR2, SIG_IGN);//крик от последнего мужика
    

    srand(time(0));

    if (fork() == 0) {
        return doctor();
    }

    int patient_no = 0;
    while (1) {
        sleep(rand() * (10.0 / RAND_MAX) + 3);
        if (patient_no == INT_MAX) {
            break;
        }
        ++ patient_no;
        if (fork() == 0) {
            return patient(patient_no);
        }
        int status;
        while (waitpid(-1, &status, WNOHANG) > 1) {
        }
    }
    return 0;
}


#define MESSAGE(str) fprintf(stderr, "Doctor: " str "\n")
int doctor(void)
{
    char c = 0;
    while (1) {

        MESSAGE("1. Please come in");
        // 1. sleep until a patient will come
        read(next_pat[0], &c, sizeof(char));

        //2. talk with the patient
        MESSAGE("2. Hello a new patient, my good patient");
        sleep(rand() * (7.0 / RAND_MAX) + 3);

        MESSAGE("3. Good bye");
        write(wait_doc_free[1], &c, sizeof(char));
    }
    return 0;
}
#undef MESSAGE

//GLOBAL VARIABLE =========
int patient_no = 0;;
int flag_last = 0;
pid_t previous_pid = -1;
pid_t pid;
char c = 0;
//===========================

#define MESSAGE(str) fprintf(stderr, "Patient %02d [%06d]: " str "\n", patient_no, pid)
//================================================
void sig_1st_hndl(int s)
{

    MESSAGE("ok, i am first\n");
    flag_last = 1;
    previous_pid = NOBODY;//its mean that i am first
}


void sig_wholast(int s)
{
    if (flag_last == 1) {
        write(previous[1], &pid, sizeof(pid));
        MESSAGE("2b. I'm the latest");
        kill(0, SIGUSR2);
        flag_last = 0;
    }
}


void sig_last_hndl(int s)
{
    flag_last = 1;//now i am last
}
//=================================================

int patient(int patient_n)
{
    
    patient_no = patient_n;
    pid = getpid();

    read(queue[0], &c, sizeof(char));//

    MESSAGE("1. Hi!");
    // Only one patient may do steps 1 and 2 simultaneously

    // 1. Broadcast the request, who is the latest patient?
    MESSAGE("2. Who is the latest?");
    signal(SIGUSR2, sig_last_hndl);//реакция на последнего

    kill(0, SIGUSR1);//кто последний?
    
    signal(SIGALRM, sig_1st_hndl);//никто
    // 2. wait for the answer for a 5 second (check the answer each second)
    
    alarm(5);
    pause();//if alarm i am first
    signal(SIGALRM, SIG_IGN);//никто

    if (previous_pid != NOBODY) {
        read(previous[0] , &previous_pid, sizeof(previous_pid));
    }
    //!!! THE LATEST PROCESS MUST INVOKE MESSAGE("2b. I'm the latest");
    // Now you have PID of the latest patient process ("previous patient")
    // or info that you are the first patient

    MESSAGE("3. I'm waiting for my turn");
    signal(SIGUSR1, sig_wholast);//меня спросили кто последний
    write(queue[1], &c, sizeof(char)); //теперь может кричать следующий
    // 3. wait until we become the first patient to the doctor
    //  (i.e. check each second if the previous person process is existed) 
    

    if (previous_pid != NOBODY) {
        while(kill(previous_pid, 0) != -1) {
            sleep(1);
        }
    }
    
    MESSAGE("4. I'm the next to the doctor");
    // 4. wait until doctor will become free
    read(wait_doc_free[0], &c, sizeof(char));
    
    write(next_pat[1], &c, 1);
    MESSAGE("5. Hi doctor!");
    // 5. wake up the doctor and exit my process
    

    return 0;
}
#undef MESSAGE