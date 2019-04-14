/*********** t.c file of A Multitasking System *********/
#include <stdio.h>
#include "type.h"
#include "string.h"
#include "queue.c"     // include queue.c file

PROC proc[NPROC];      // NPROC PROCs
PROC *freeList;        // freeList of PROCs
PROC *readyQueue;      // priority queue of READY procs
PROC *running;         // current running proc pointer
PROC *zombieList;      //
PROC *sleepList;        // sleepList of PROCs

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body();
int tswitch();

int ksleep(int event)
{ 
  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
}

int kwakeup(int event) {
  int i;
	PROC *p = sleepList;
  PROC *temp;
  temp = 0;
	
	if (p == NULL) // No Child in SleepList
	{
		printf("[!] - NO CHILD IN SLEEPLIST TO WAKEUP.\n");
		return -1;
	}
	else
  {
    while(p = dequeue(&sleepList))
    {
      if (p->event == event)
      {
        printf("WAKE UP THE CHILD #%d\n", p->pid);
        p->status = READY;
        enqueue(&readyQueue, p);
      }
      else
      {
        enqueue(&temp, p);
      }
    }
    sleepList = temp;
    printList("sleepList", sleepList);

  }
	return 0;
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
  p->priority = 1;       // ALL PROCs priority=1,except P0
  p->ppid = running->pid;

  p->parent = running;  // Saves content of parent proc
  if(p->parent->child == 0)
  { /// child is empty
    p->parent->child = p; // Stores p in binary tree
  }
  else
  { /// child is not empty
    PROC *temp = p->parent->child;
    while(temp->sibling != 0)
    {
      temp = temp->sibling; // traverse until free node
    }
    temp->sibling = p;  //Stores p in binary tree @node
  }
  
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

int kexit(int exitValue)
{

  int i;
  PROC *p;

  for (i = 2; i < NPROC; i++)
  { // Skip root proc[0] and proc[1]
    p = &proc[i];
    /// Child sent to parent (orphanage)
    if(p->status != FREE && p->ppid == running->pid)
    {
      p->ppid = 1;
      p->parent = &proc[1];
      if (p->status == ZOMBIE) {
        p->status = FREE;
        enqueue(&freeList, p);  //Release all ZOMBIE childs
      }
      else
      {
        printf("Send child #&d to proc[1] (parent)\n", p->pid);
        p->ppid = 1;
        p->parent = &proc[1];
      }
    }
  }

  running->exitCode = exitValue;  // Record exitValue in proc for parent
  running->status = ZOMBIE;       // Will not free PROC
  enqueue(&zombieList, p);

  kwakeup(running->parent);
  kwakeup(&proc[1]);

  tswitch();  //Switch process to give up CPU
}

int kwait(int *status)
{ /// proc waits for (and disposes of) a ZOMBIE child.
  int i, hasChild = 0;
  PROC *p;

  while (1) {
    for (i=1; i<NPROC; i++) {
      p = &proc[i];

      /// Search for (any) zombie child
      if (p->status != FREE && p->ppid == running->pid)
      {
        hasChild = 1;
        if (p->status == ZOMBIE)
        {
          dequeue(&zombieList);
          *status = p->exitCode;  // Get exit code
          p->status = FREE;       // Set status to Free
          enqueue(&freeList, p);  // release child PROC to freelist
          return (p->pid);        // Return pid
        }

      }
    }
    if (!hasChild) {
      printf("[!] - NO CHILD FOUND / P1 NO DIE\n");
      return -1;
    }
    ksleep(running);
  }
}



int do_kfork()
{
   int child = kfork();
   if (child < 0)
      printf("kfork failed\n");
   else {
      printf("proc %d kforked a child = %d\n", running->pid, child); 
      printList("readyQueue", readyQueue);
      //printList("sleepList", sleepList);
   }
   return child;
}

int do_sleep()
{
  int event;
  printf("enter an event value to sleep on: ");
  //event = geti();
  ksleep(event);
}

int do_wakeup() {
  int event;
  printf("Enter an event value to wakeup with: ");
  //event = geti();
  kwakeup(event);
}

int do_wait()
{
  int pid, status;
	pid = kwait(&status);
	printf("pid = %d, exitVal = %d\n", pid, status);
}

int do_switch()
{
   tswitch();
}

int do_exit()
{
  kexit(running->pid); 
}

int do_jesus()
{
  int i;
  PROC *p;
  printf("Jesus performs miracles\n");
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

int menu()
{
  printf("********************************\n");
  printf(" ps  fork  switch  exit  jesus  \n");
  printf("********************************\n");
}

char *status[ ] = {"FREE", "READY", "SLEEP", "ZOMBIE"};
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
    
int body()   // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while(1){
    printf("***************************************\n");
    printf(":: proc %d running: parent=%d\n", running->pid,running->ppid);
    printList("readyQueue", readyQueue);
    printList("sleepList", sleepList);
    printList("freeList", freeList);
    printList("zombieList", zombieList);
    printList("running", running);

    if (running->child != 0) {    // Displays child
      printf("\n- child list: %d", running->child->pid);
      PROC *temp = running->child;  // Gets current running child
      while(temp->sibling != 0) { // Displays sibling of child
        printf(", %d", temp->sibling->pid);
        temp = temp->sibling;
      }
      printf("\n\n");
    }

    menu();
    printf("\nenter a command:\n>> ");
    fgets(cmd, 64, stdin);
    printf("\n");
    cmd[strlen(cmd)-1] = 0;

    if (strcmp(cmd, "ps")==0)
      do_ps();
    else if (strcmp(cmd, "fork")==0)
      do_kfork();
    else if (strcmp(cmd, "switch")==0)
      do_switch();
    else if (strcmp(cmd, "exit")==0)
      do_exit();
    else if (strcmp(cmd, "jesus")==0)
      do_jesus();
    else if (strcmp(cmd, "wait")==0)
      do_wait();
    else
      printf("[!] - INVALID INPUT\n");
  }
}
// initialize the MT system; create P0 as initial running process
int init() 
{
  int i;
  PROC *p;
  for (i=0; i<NPROC; i++){ // initialize PROCs to freeList
    p = &proc[i];
    p->pid = i;            // PID = 0 to NPROC-1  
    p->status = FREE;
    p->priority = 0;      
    p->next = p+1;
  }
  proc[NPROC-1].next = 0;  
  freeList = &proc[0];     // all PROCs in freeList     
  readyQueue = 0;          // readyQueue = empty
  sleepList = 0;

  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0] 
  p->status = READY;
  p->priority = 0;         // P0 has the lowest priority 0
  p->ppid = 0;             // P0 is its own parent
  p->parent = p;
  printList("freeList", freeList);
  printf("init complete: P0 running\n"); 
}

/*************** main() function ***************/
int main()
{
   printf("Welcome to the MT Multitasking System\n");
   init();    // initialize system; create and run P0
   kfork();   // kfork P1 into readyQueue  
   while(1){
     printf("P0: switch process\n");
     while (readyQueue == 0); // loop if readyQueue empty
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
