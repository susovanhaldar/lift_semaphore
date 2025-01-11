#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#define P(s) semop(s, &pop, 1)
#define V(s) semop(s, &vop, 1)
#define NFLOORS 10
#define NLIFT 3 // increasing and decreasing lift needs to change the animate function
#define  NPEOPLES 20
int pid[NPEOPLES];  //pid for all people processes
int lid[NLIFT];  //pid for lift processes
//here We can set the number of people
//for each floor we are storing this info
typedef struct Floor_info {
    int waitingtogoup;    //number of people willing to go up from this floor
    int waitingtogodown;  //number of people willing to go down from this floor
    int current_lift;     //this variable stores which lift actually present in this floor right now
    int up_arrow;         //this is a semaphore to queue the people willing to go up
    int down_arrow;       //similarly a semaphore to go down
    int mutex;            //this semaphore only allow one process to change shared variables
} Floor_info;
//structure to each lift
typedef struct lift_info {
    int no;                //id of each lift
    int position;          //indicate which floor the lift is right now
    int direction;         //indicate which direction the lift is going right now
    int peopleinlift;      //indicate how many people in the lift right now
    int capacity;          //indicate the  capacity of the lift
    int mutex[NFLOORS];    //a semaphore to allow one process to change the lift information for a particular floor
    int stops[NFLOORS];    //this will count the number of people willing to leave the lift
    int stopsem[NFLOORS];  //here each process wait for a floor using this semaphore
} lift_info;
Floor_info *parent_floor_shm;  //parent reference of the floor shared memory
lift_info *parent_lift_shm;   //parent reference of the lift shared memory
void errExit(char *str){
    fprintf(stderr,"error happen with desctiption: %s\n",str);
    exit(0);
}
//signal handler function for killing all child process
void sigHandler(int sig){
    printf("\nterminating all child processes\n");
    int i;
    for(i=0;i<NPEOPLES;i++){
        //killing all peoples
        if(kill(pid[i],SIGTSTP)==-1)
        errExit("kill person");
    }
    for(i=0;i<NLIFT;i++){
        //killing all lift 
        if(kill(lid[i],SIGTSTP)==-1)
        errExit("kill lift");
    }
    
    //detaching shared memory here.
    if(shmdt(parent_floor_shm)==-1) errExit("detaching shared memory");
    if(shmdt(parent_lift_shm)==-1) errExit("detaching shared memory");
    exit(0);
}
//this function will animate the output
//This function will only work with 3 lifts. If more lift is present then a slight modification is needed.
void animate(Floor_info *parent_floor_shm, lift_info *parent_lift_shm) {
    int i, j, k;
    int lift1 = parent_lift_shm[0].position, lift2 = parent_lift_shm[1].position, lift3 = parent_lift_shm[2].position;
    int c1 = parent_lift_shm[0].peopleinlift, c2 = parent_lift_shm[1].peopleinlift, c3 = parent_lift_shm[2].peopleinlift;
    system("clear");
    printf("    building       l1(%2d) l2(%2d) l3(%2d)\n",parent_lift_shm[0].capacity,parent_lift_shm[1].capacity,parent_lift_shm[2].capacity);
    printf("_____________________________________\n");
    for (j = NFLOORS - 1; j >= 0; j--) {
        if (lift1 == j && lift2 == j && lift3 == j) {
            printf("|  %3du    %3dd   |  _|_   _|_   _|_\n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |  |  |  |  |  |  |\n");
            printf("|       /|\\       |  |%2d|  |%2d|  |%2d|\n", c1, c2, c3);
            printf("|_______/\\________|  |__|  |__|  |__|\n");
        } else if (lift1 == j && lift2 == j) {
            printf("|  %3du    %3dd   |  _|_   _|_    |  \n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |  |  |  |  |   |  \n");
            printf("|       /|\\       |  |%2d|  |%2d|   |  \n", c1, c2);
            printf("|_______/\\________|  |__|  |__|   |  \n");
        } else if (lift1 == j && lift3 == j) {
            printf("|  %3du    %3dd   |  _|_    |    _|_\n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |  |  |   |    |  |\n");
            printf("|       /|\\       |  |%2d|   |    |%2d|\n", c1, c3);
            printf("|_______/\\________|  |__|   |    |__|\n");
        } else if (lift2 == j && lift3 == j) {
            printf("|  %3du    %3dd   |   |    _|_   _|_\n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |   |    |  |  |  |\n");
            printf("|       /|\\       |   |    |%2d|  |%2d|\n", c2, c3);
            printf("|_______/\\________|   |    |__|  |__|\n");
        } else if (lift1 == j) {
            printf("|  %3du    %3dd   |  _|_    |     | \n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |  |  |   |     | \n");
            printf("|       /|\\       |  |%2d|   |     | \n", c1);
            printf("|_______/\\________|  |__|   |     | \n");
        } else if (lift2 == j) {
            printf("|  %3du    %3dd   |   |    _|_    |  \n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |   |    |  |   |  \n");
            printf("|       /|\\       |   |    |%2d|   |  \n", c2);
            printf("|_______/\\________|   |    |__|   |  \n");
        } else if (lift3 == j) {
            printf("|  %3du    %3dd   |   |     |    _|_\n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |   |     |    |  |\n");
            printf("|       /|\\       |   |     |    |%2d|\n", c3);
            printf("|_______/\\________|   |     |    |__|\n");
        } else {
            printf("|  %3du    %3dd   |   |     |     | \n", parent_floor_shm[j].waitingtogoup, parent_floor_shm[j].waitingtogodown);
            printf("|        *        |   |     |     | \n");
            printf("|       /|\\       |   |     |     | \n");
            printf("|_______/\\________|   |     |     | \n");
        }
    }
    printf("here nu and md in each floor means n people willing to go up and m people willing to go down\n");
    printf("press ctrl+z to terminate the program\n");
}
//each person will execute this function independently.
void create_persons(int i, int shmid_floors, int shmid_lift, struct sembuf pop, struct sembuf vop) {
    int current = 0;
    int destination;
    Floor_info *flr;
    lift_info *lft;
    if((flr = (Floor_info *)shmat(shmid_floors, 0, 0))==(void*)-1) errExit("shmat");
    if((lft = (lift_info *)shmat(shmid_lift, 0, 0))==(void*)-1) errExit("shmat");
    
    //by default I am setting that each person can travel 20 times by the lift.
    while (1) {
        //people
        srand(i);
        //choosing a destination different from the current floor.
        while ((destination = rand() % NFLOORS) == current)
            ;
        //if person wants to go down    
        if (destination < current) {
            // people lock the mutex and increment waiting to go down as the person is willign to go down
            P(flr[current].mutex);
            flr[current].waitingtogodown++;
            // signal the mutex to allow other process to access the floor variables
            V(flr[current].mutex);
            // Wait for a lift to signal that it is ready to take the person from current floor then the person can go inside the lift
            P(flr[current].down_arrow);
            // before boarding lift, decrement the waiting to go down as the person is going to board the lift
            flr[current].waitingtogodown--;
            int lift = flr[current].current_lift;
            //code for boarding lift 
            // increment people in lift
            lft[lift].peopleinlift++;
            // increment the number of people willing to go to the destination floor in the lift
            lft[lift].stops[destination]++;
            // signal the mutex to allow lift process to access the floor variables
            V(flr[current].mutex);
            // wait for the lift to signal that it has reached the destination floor
            P(lft[lift].stopsem[destination]);
            // decrement the people in lift as the person is going to leave the lift
            lft[lift].peopleinlift--;
            // decrement the number of people willing to go to the destination floor in the lift
            lft[lift].stops[destination]--;
            // signal the mutex to allow the lift to deboard other peoples
            V(lft[lift].mutex[destination]);
            // update the current floor as the destination floor 
            current = destination;
        //if person willing to go up    
        } else {
            P(flr[current].mutex);
            flr[current].waitingtogoup++;
            V(flr[current].mutex);
            P(flr[current].up_arrow);
            flr[current].waitingtogoup--;
            int lift = flr[current].current_lift;
            //code for boarding lift 
            lft[lift].peopleinlift++;
            lft[lift].stops[destination]++;
            V(flr[current].mutex);
            P(lft[lift].stopsem[destination]);
            lft[lift].peopleinlift--;
            lft[lift].stops[destination]--;
            V(lft[lift].mutex[destination]);
            current = destination;
        }
        //here each person sleep for a random amount of time before boarding to the lift again
        int w = 3 + rand() % 4;
        sleep(w);
    }
}
//each lift will execute the code independently
void create_lift(int i, int shmid_floors, int shmid_lift, struct sembuf pop, struct sembuf vop) {
    int id = i;   
    lift_info *lft ;
    Floor_info *flr;    
    // actual reference of the shared memories for lifts and floors     
    if((lft = (lift_info *)shmat(shmid_lift, 0, 0))==(void*)-1) errExit("shmat");
    if((flr = (Floor_info *)shmat(shmid_floors, 0, 0))==(void*)-1) errExit("shmat");
    lft[id].no = id;
    lft[id].position = 0;
    lft[id].direction = 1;
    lft[id].peopleinlift = 0;
    lft[id].capacity = 3 * i + 2;   //by default we make the capacity as this  equation. But we can change this
    while (1) {
        sleep(1);
        // This loop deboard the people from the lift if the lift has reached the destination floor
        while (1) {
            // lock the mutex to allow only one process to change the lift variables
            P(lft[id].mutex[lft[id].position]);
            if (lft[id].stops[lft[id].position]) {
                // deboard people that are waiting to go to the current floor by signaling the semaphore once for each person
                V(lft[id].stopsem[lft[id].position]);
                sleep(1);
            } else {
                // if no peoples are waiting signal the mutex itself
                V(lft[id].mutex[lft[id].position]);
                break;
            }
        }
        // this loop board peoples inside lift to go up
        if (lft[id].direction == 1) {
            while (1) {
                // wait for mutex of its current floor
                P(flr[lft[id].position].mutex);
                // check if any peoples are waiting to go up from current floor and the lift has space to board
                if (flr[lft[id].position].waitingtogoup && lft[id].peopleinlift < lft[id].capacity) {
                    flr[lft[id].position].current_lift = id;
                    // signal this semaphore to allow one people to go inside, and people will signal the mutex for the floor
                    V(flr[lft[id].position].up_arrow);
                    sleep(2);
                } else {
                    // signal the mutex itself
                    V(flr[lft[id].position].mutex);
                    break;
                }
            }
        }
        // this loop board peoples inside lift to go down 
        else if (lft[id].direction == 0) {
            while (1) {
                P(flr[lft[id].position].mutex);
                if (flr[lft[id].position].waitingtogodown && lft[id].peopleinlift < lft[id].capacity) {
                    flr[lft[id].position].current_lift = id;
                    V(flr[lft[id].position].down_arrow);
                    sleep(2);
                } else {
                    V(flr[lft[id].position].mutex);
                    break;
                }
            }
        }
        // change lift positions and directions
        if (lft[id].direction == 1 && lft[id].position == NFLOORS - 1) {
            lft[id].direction = 0;
        } else if (lft[id].direction == 1) {
            lft[id].position += 1;
        } else if (lft[id].direction == 0 && lft[id].position == 0) {
            lft[id].direction = 1;
        } else if (lft[id].direction == 0) {
            lft[id].position -= 1;
        }
    }
}
int main() {
    //this the parent process of all process.
    int i, j, k;
    int shmid_floors;        //shared memory id for all floor information
    int shmid_lift;          //shared memory id for all lift information
    
    struct sembuf pop, vop;  //this variable needed in the wait and signal operation
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = SEM_UNDO;
    pop.sem_op = -1;
    vop.sem_op = 1;
    //creating shared memory for floors
    if((shmid_floors = shmget(IPC_PRIVATE, NFLOORS * sizeof(Floor_info), 0777 | IPC_CREAT))==-1) errExit("shmat");
    if((parent_floor_shm = (Floor_info *)shmat(shmid_floors, 0, 0))==(void*)-1) errExit("shmat");
    //we create 3 semaphore for each floor, one for going up, one for going down, one for mutex operation in each floor
    for (i = 0; i < NFLOORS; i++) {
        if((parent_floor_shm[i].up_arrow = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT))==-1) errExit("semget");
        if((parent_floor_shm[i].down_arrow = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT))==-1) errExit("semget");
        if((parent_floor_shm[i].mutex = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT))==-1) errExit("semget");
        if((semctl(parent_floor_shm[i].up_arrow, 0, SETVAL, 0))==-1) errExit("semctl");    //each process should wait for the lift's signal, so we set the semaphore value as 0
        if((semctl(parent_floor_shm[i].down_arrow, 0, SETVAL, 0))==-1) errExit("semctl");  //each process should wait for the lift's signal, so we set the semaphore value as 0
        if((semctl(parent_floor_shm[i].mutex, 0, SETVAL, 1))==-1) errExit("semctl");       //we allow only one process to access floor variables, so we set the semaphore mutex as 1
    }
    if((shmid_lift = shmget(IPC_PRIVATE, sizeof(lift_info) * NLIFT, 0777 | IPC_CREAT))==-1) errExit("semget");
    if((parent_lift_shm = (lift_info *)shmat(shmid_lift, 0, 0))==(void*)-1) errExit("shmat");
    for (j = 0; j < NFLOORS; j++) {
        for (k = 0; k < NLIFT; k++) {
            parent_lift_shm[k].stops[j] = 0;                                           //by default we assumes that no one in the lift
            if((parent_lift_shm[k].stopsem[j] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT))==-1) errExit("semget");  //persons will wait on this semaphores to get down in a floor
            if((parent_lift_shm[k].mutex[j] = semget(IPC_PRIVATE, 1, 0777 | IPC_CREAT))==-1) errExit("semget");    //this semaphore is for concurrent updation of the lift variables
            if((semctl(parent_lift_shm[k].stopsem[j], 0, SETVAL, 0))==-1) errExit("semctl");                       //we set the value of the semaphore as 0 to wait.
            if((semctl(parent_lift_shm[k].mutex[j], 0, SETVAL, 1))==-1) errExit("semctl");                         //we set the mutex as 1 as we allow only 1 process
        }
    }
    //creating process for each people
    
    for (i = 0; i < NPEOPLES; i++) {
        if ((pid[i] = fork()) == 0) {
            create_persons(i, shmid_floors, shmid_lift, pop, vop);
            exit(0);
        }
    }
    //creating lift
    
    for (i = 0; i < NLIFT; i++) {
        if ((lid[i] = fork()) == 0) {
            create_lift(i, shmid_floors, shmid_lift, pop, vop);
            exit(0);
        }
    }
    if(signal(SIGTSTP,sigHandler)==(void*)-1) errExit("signal");
    //code for parent to check the current state of the system.
    while (1) {
        
        animate(parent_floor_shm, parent_lift_shm);
        sleep(1);
    }
}

