/*********** t.c file of A Multitasking System *********/
#include <stdio.h>
#include "string.h"
#include "type.h"

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs 
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer

PROC *sleepList;       // list of SLEEP procs

struct semaphore full, empty, mutex;

#include "queue.c"     // include queue.c file
#include "wait.c"      // include wait.c file

#define PRSIZE 8
char buf[PRSIZE];
int head, tail;

//SEMAPHORE rwsem = 1, wsem = 1, rsem = 1;
//int nreader = 0; // Active Readers

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body(), tswitch(), do_sleep(), do_wakeup(), do_exit(), do_switch();
int do_kfork();

int P(struct semaphore *s)
{ //Locking - print the BLOCKed process pid and semaphore queue;
  int itr = 0;
  PROC *p;

  //int_off turns off interrupts, makes sure were alone
  //itr = int_off();
  //dec the value
  s->value--;
  //check the value
  if(s-> value < 0)
  {
    running->status = BLOCK;
    enqueue(&(s->queue), running);  //Puts Proc in Semaphore Queue
    tswitch();  //in the if? or out of the if?
  }
  //turn on interrupts again
  //int_on(itr);
}

int V(struct semaphore *s)
{ //Unlocking - print the unBLOCKed process pid, semaphore queue and readyQueue;
  int itr = 0;
  PROC *p;

  //int_off turns off interrupts, makes sure were alone
  //itr = int_off();
  //dec the value
  s->value++;
  //check the value
  if(s->value <= 0)
  {
    p = dequeue(&(s->queue));
    p->status = READY;
    enqueue(&readyQueue, p);
  }
  //turn on interrupts again
  //int_on(itr);
}

/*void Reader() {
  while(1) {
    P(rwsem);
    P(rsem);
    nreader++;
    if (nreader == 1)
      P(wsem);
    V(rsem);
    V(rwsem);
    P(rsem);
    nreader--;
    if (nreader == 0)
      V(wsem);
    V(rsem);
  }
}

void Write() {
  while(1) {
    P(rwsem);
    P(wsem);
    V(wsem);
    V(rwsem);
  }
}
*/

int producer() {
  char c, *cp;
  char line[PRSIZE];
  while (1) {
    //ugets(up, line);

    printf("input something: ");
    /// 1.) input a string
    fgets(line,PRSIZE,stdin);
    /// 2.) write chars of string to buff
    strncpy(buf, line, PRSIZE);
    cp = line;
    while (*cp) {
      printf("Producer %d P(empty=%d)\n", running->pid, empty.value);
      P(&empty);
      P(&mutex);
      head %= PRSIZE;
      V(&mutex);
      printf("Producer %d V(full=%d)\n", running->pid, full.value);
      V(&full);
    }
  }
}

int consumer() {
  char c, line[PRSIZE];
  int i;
  /// 1.) get 10 chars from buf[]
  strncpy(line, buf, 10);
  /// 2.) show the chars
  printf("buf[10]: %s\n", buf);
  while(1) {
    printf("Consumer %d P(full=%d)\n", running->pid, full.value);
    P(&full);
    P(&mutex);
    c = buf[tail++];
    tail %= PRSIZE;
    V(&mutex);
    printf("Consumer %d V(empty=%d) ", running->pid, empty.value);
    V(&empty);
  }
}


// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty

  sleepList = 0;           // sleepList = empty
  
  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;
  p->ppid = 0;             // P0 is its own parent
  
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

int menu()
{
  printf("****************************************\n");
  printf(" ps fork switch exit jesus sleep wakeup \n");
  printf("****************************************\n");
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE", "BLOCK"};

int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i=0; i<NPROC; i++){
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}

int do_jesus()
{
  int i;
  PROC *p;
  printf("Jesus perfroms miracles here\n");
  for (i=1; i<NPROC; i++){
    p = &proc[i];
    if (p->status == ZOMBIE){
      p->status = READY;
      enqueue(&readyQueue, p);
      printf("raised a ZOMBIE %d to live again\n", p->pid);
    }
  }
  printList("readyQueue", readyQueue);
}
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid,running->ppid);
    printList("readyQueue", readyQueue);
    printSleep("sleepList ", sleepList);
    
    menu();
    printf("enter a command : ");
    fgets(cmd, 64, stdin);
    cmd[strlen(cmd)-1] = 0;

    if (strcmp(cmd, "ps")==0)
      do_ps();
    if (strcmp(cmd, "fork")==0)
      do_kfork();
    if (strcmp(cmd, "switch")==0)
      do_switch();
    if (strcmp(cmd, "exit")==0)
      do_exit();
    if (strcmp(cmd, "jesus")==0)
      do_jesus();
   if (strcmp(cmd, "sleep")==0)
      do_sleep();
   if (strcmp(cmd, "wakeup")==0)
      do_wakeup();
    if (strcmp(cmd, "p")==0)
      producer();
    if (strcmp(cmd, "c")==0)
      consumer();
  }
}

int kfork()
{
  int  i;
  PROC *p = dequeue(&freeList);
  if (!p){
     printf("no more proc\n");
     return(-1);
  }
  /* initialize the new proc and its stack */
  p->status = READY;
  p->priority = 1;       // ALL PROCs priority=1, except P0
  p->ppid = running->pid;
  
  /************ new task initial stack contents ************
   kstack contains: |retPC|eax|ebx|ecx|edx|ebp|esi|edi|eflag|
                      -1   -2  -3  -4  -5  -6  -7  -8   -9
  **********************************************************/
  for (i=1; i<10; i++)               // zero out kstack cells
      p->kstack[SSIZE - i] = 0;
  p->kstack[SSIZE-1] = (int)body;    // retPC -> body()
  p->ksp = &(p->kstack[SSIZE - 9]);  // PROC.ksp -> saved eflag 
  enqueue(&readyQueue, p);           // enter p into readyQueue
  return p->pid;
}

int do_kfork()
{
   int child = kfork();
   if (child < 0)
      printf("kfork failed\n");
   else{
      printf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
   }
   return child;
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  kexit(running->pid);  // exit with own PID value 
}

int do_sleep()
{
  int event;
  printf("enter an event value to sleep on : ");
  scanf("%d", &event); getchar();
  sleep(event);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wakeup with : ");
  scanf("%d", &event); getchar(); 
  wakeup(event);
}


/*************** main() function ***************/
int main()
{
   printf("Welcome to the MT Multitasking System\n");
   init();    // initialize system; create and run P0
   kfork();   // kfork P1 into readyQueue  

  /* Product-Consumer */
  head = tail = 0;
  full.value = 0;
  //full.queue = 0;
  empty.value = PRSIZE;
  //empty.queue = 0;
  mutex.value = 1;
  //mutex.queue = 0;
  printf("P0 kfork tasks\n");
  kfork((int)producer, 0);  //157
  kfork((int)consumer, 0);
  kfork((int)producer, 1);
  kfork((int)consumer, 1);
  printf("PRODUCER + CONSUMER: \n");
  printList("readyQueue", readyQueue);

   while(1){
     printf("P0: switch process\n");
     while (readyQueue == 0);
         tswitch();
   }
}

/*********** scheduler *************/
int scheduler()
{ 
  printf("proc %d in scheduler()\n", running->pid);
  if (running->status == READY)
     enqueue(&readyQueue, running);
  printList("readyQueue", readyQueue);
  running = dequeue(&readyQueue);
  printf("next running = %d\n", running->pid);  
}
