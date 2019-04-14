#include "type.h"
#include "string.c"

PROC proc[NPROC]; // NPROC PROCs
PROC *freeList;   // freeList of PROCs
PROC *readyQueue; // priority queue of READY procs
PROC *zombieList;
PROC *timerQueue;
PROC *running; // current running proc pointer
PROC *sleepList; // list of SLEEP procs
PROC *ptemp; // list of SLEEP procs
int procsize = sizeof(PROC);

#define printf kprintf
#define gets kgets

#include "kbd.c"
#include "vid.c"
#include "exceptions.c"

#include "queue.c"
#include "wait.c"
#include "pv.c"
#include "pipe.c"
#include "uart.c"
#include "timer.c"

UART *up;

/*******************************************************
  kfork() creates a child process; returns child pid.
  When scheduled to run, child PROC resumes to body();
********************************************************/
int body(), tswitch(), do_sleep(), do_wakeup(), do_exit(), do_switch();
int do_kfork();
int scheduler();

int kprintf(char *fmt, ...);

void copy_vectors(void)
{
  extern u32 vectors_start;
  extern u32 vectors_end;
  u32 *vectors_src = &vectors_start;
  u32 *vectors_dst = (u32 *)0;
  while (vectors_src < &vectors_end)
    *vectors_dst++ = *vectors_src++;
}

int timer_handler();
TIMER *tp[4]; // 4 TIMER structure pointers

void IRQ_handler()
{
  int vicstatus, sicstatus;
  int ustatus, kstatus;

  // read VIC SIV status registers to find out which interrupt
  vicstatus = VIC_STATUS;
  sicstatus = SIC_STATUS;
  if (vicstatus & 0x80000000)
  { // SIC interrupts=bit_31=>KBD at bit 3
    if (sicstatus & 0x08)
    {
      kbd_handler();
    }
  }
  int flag;
  flag = timer_handler(0);

  // ADD ENQUEUE TO ADD_TIMER
  if (timerQueue != 0 && total_timers <= 1) {
    int sr = int_off();
    PROC *p = dequeue(&timerQueue);
    timer_start(total_timers, p->pid, p->timer_sec);
    int_on(sr);
    //dequeue(&timerQueue);
  }
  for (int i = 1; i < total_timers; i++)
  {
    flag = timer_handler(i);
    if (flag != 0)
    {
      printf("---- AWAKEN THE CHILD %d\n", flag);
      kwakeup(&proc[flag]);
      printList("sleepList", sleepList);
      // Add next item on list to timer
      if (ptemp != 0) {
        PROC *p = dequeue(ptemp);
        enqueue(&timerQueue, p);
        ksleep(p);
      }
    }
  }
}

// initialize the MT system; create P0 as initial running process
int init()
{
  int i;
  PROC *p;

  for (i = 0; i < NPROC; i++)
  { // initialize PROCs
    p = &proc[i];
    p->pid = i; // PID = 0 to NPROC-1
    p->status = FREE;
    p->timer_display = FALSE;
    p->timer_sec = 0;
    p->priority = 0;
    p->next = p + 1;
  }
  p[0].timer_display = TRUE;
  proc[NPROC - 1].next = 0;
  freeList = &proc[0]; // all PROCs in freeList
  readyQueue = 0;      // readyQueue = empty
  sleepList = 0;       // sleepList = empty
  timerQueue = 0;

  // create P0 as the initial running process
  p = running = dequeue(&freeList); // use proc[0]
  p->status = READY;
  p->priority = 0;
  p->ppid = 0; // P0 is its own parent
  p->parent = p;

  printList("freeList", freeList);
  printf("init complete: P0 running\n");
}

int INIT()
{
  int pid, status;
  PIPE *p = &pipe;
  printf("P1 running: create pipe and writer reader processes\n");
  kpipe();
  kfork((int)pipe_writer());
  kfork((int)pipe_reader());
  printf("P1 waits for ZOMBIE child\n");
  while(1){
    pid = kwait(&status);
    if (pid < 0){
      printf("no more child, P1 loops\n");
      while(1);
    }
    printf("P1 buried a ZOMBIE child %d\n", pid);
  }
}
  

int menu()
{
  printf("***************************************\n");
  printf(" ps fork switch exit wait sleep wakeup \n");
  printf("***************************************\n");
}

char *status[] = {"FREE", "READY", "SLEEP", "ZOMBIE", "BLOCK"};

int do_ps()
{
  int i;
  PROC *p;
  printf("PID  PPID  status\n");
  printf("---  ----  ------\n");
  for (i = 0; i < NPROC; i++)
  {
    p = &proc[i];
    printf(" %d    %d    ", p->pid, p->ppid);
    if (p == running)
      printf("RUNNING\n");
    else
      printf("%s\n", status[p->status]);
  }
}

int body() // process body function
{
  int c;
  char cmd[64];
  printf("proc %d starts from body()\n", running->pid);
  while (1)
  {
    printf("***************************************\n");
    printf("proc %d running: parent=%d\n", running->pid, running->ppid);
    printList("readyQueue", readyQueue);
    printSleepList(sleepList);
    printList("freeList", freeList);
    printList("zombieList", zombieList);
    printList("running", running);

    if (running->child != 0)
    { // Displays child
      printf("-child list: [%d %s ]", running->child->pid, status[running->child->status]);
      PROC *temp = running->child; // Gets current running child
      while (temp->sibling != 0)
      { // Displays sibling of child
        printf("->[%d %s ]", temp->sibling->pid, status[running->child->status]);
        temp = temp->sibling;
      }
      printf("\n");
    }

    menu();

    //show_pipe();

    printf("enter a command:\n>> ");
    gets(cmd);
    printf("\n");

    if (strcmp(cmd, "ps") == 0)
      do_ps();
    else if (strcmp(cmd, "fork") == 0)
      do_kfork();
    else if (strcmp(cmd, "switch") == 0)
      do_switch();
    else if (strcmp(cmd, "wait") == 0)
      do_wait();
    else if (strcmp(cmd, "exit") == 0)
      do_exit();
    else if (strcmp(cmd, "sleep") == 0)
      do_sleep();
    else if (strcmp(cmd, "wakeup") == 0)
      do_wakeup();
    else if (strcmp(cmd, "t") == 0)
      do_timer();
    else if (strcmp(cmd, "w") == 0)
      do_write();
    else if (strcmp(cmd, "r") == 0)
      do_read();
    else if (strcmp(cmd, "p") == 0)
      do_pipe();
    else
      printf("[!] - INVALID INPUT\n");
  }
}

int kfork()
{
  int i;
  PIPE *pp = &pipe;
  PROC *p = dequeue(&freeList);
  if (p == 0)
  {
    kprintf("kfork failed\n");
    return -1;
  }
  p->ppid = running->pid;
  p->parent = running;
  p->status = READY;
  p->priority = 1;

  p->parent = running; // Saves content of parent proc
  if (p->parent->child == 0)
  {                       /// child is empty
    p->parent->child = p; // Stores p in binary tree
  }
  else
  { /// child is not empty
    PROC *temp = p->parent->child;
    while (temp->sibling != 0)
    {
      temp = temp->sibling; // traverse until free node
    }
    temp->sibling = p; //Stores p in binary tree @node
  }

  // set kstack to resume to body
  // stack = r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r14
  //         1  2  3  4  5  6  7  8  9  10 11  12  13  14
  for (i = 1; i < 15; i++)
    p->kstack[SSIZE - i] = 0;
  p->kstack[SSIZE - 1] = (int)body; // in dec reg=address ORDER !!!
  p->ksp = &(p->kstack[SSIZE - 14]);

  enqueue(&readyQueue, p);

  return p->pid;
}

int do_kfork()
{
  int child = kfork();
  if (child < 0)
    printf("kfork failed\n");
  else
  {
    printf("proc %d kforked a child = %d\n", running->pid, child);
    printList("readyQueue", readyQueue);
  }

  return child;
}

int do_switch()
{
  tswitch();
}

int do_wait()
{
  int pid, status;
  pid = kwait(&status);
  printf("pid = %d, exitVal = %d\n", pid, status);
}

int do_exit()
{
  kexit(running->pid); // exit with own PID value
}

int do_sleep()
{

  int event;
  printf("enter an event value to sleep on : ");
  event = geti();
  ksleep(event);
}

int do_wakeup()
{
  int event;
  printf("enter an event value to wakeup with : ");
  event = geti();
  kwakeup(event);
}

int add_queue(int sec) {
  running->timer_sec = sec;
  if (timerQueue != 0 && total_timers > 1) {
    enqueue(&ptemp, running);
  } else {
    enqueue(&timerQueue, running);
    ksleep(running);
    printf("\n+ ADDED PROC %d TO ENQUEUE (%d SEC)\n", running->pid, running->timer_sec);
    printList("sleepList", sleepList);
  }
}

int do_timer()
{
  int sec;
  printf("enter a time value to wakeup with : ");
  sec = geti();
  printf("\n+ New TIMER (%d sec) for P%d", sec, running->pid);
  add_queue(sec);
  
}

int do_write()
{
  pipe_writer();
}

int do_read()
{
  pipe_reader();
}

int do_pipe()
{
  INIT();
}

int main()
{
  int i;
  char line[128];
  u8 kbdstatus, key, scode;
  KBD *kp = &kbd;
  color = YELLOW;
  row = col = 0;

  fbuf_init();
  kprintf("Welcome to Wanix in ARM\n");
  kbd_init();
  uart_init();

  VIC_INTENABLE |= (1 << 4); // timer0,1 at bit4
  VIC_INTENABLE |= (1 << 5); // timer2,3 at bit5

  /* enable SIC interrupts */
  VIC_INTENABLE |= (1 << 31); // SIC to VIC's IRQ31
  /* enable KBD IRQ */
  SIC_INTENABLE = (1 << 3); // KBD int=bit3 on SIC
  SIC_ENSET = (1 << 3);     // KBD int=3 on SIC
  *(kp->base + KCNTL) = 0x12;

  timer_init();
  timer_start(0, 0, 1);

  init();

  printQ(readyQueue);
  kfork(); // kfork P1 into readyQueue
  //kfork(INIT());

  unlock();
  while (1)
  {
    if (readyQueue)
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
