int tswitch();

int ksleep(int event)
{
  int sr = int_off();
  printf("proc %d going to sleep on event=%d\n", running->pid, event);

  running->event = event;
  running->status = SLEEP;
  enqueue(&sleepList, running);
  printList("sleepList", sleepList);
  tswitch();
  int_on(sr);
}

int kwakeup(int event)
{
  PROC *temp, *p;
  temp = 0;
  int sr = int_off();
  
  //printList("sleepList", sleepList);

  while (p = dequeue(&sleepList)){
    if (p->event == event){
	    printf("wakeup %d\n", p->pid);
	    p->status = READY;
	    enqueue(&readyQueue, p);
    }
    else{
	    enqueue(&temp, p);
    }
  }
  sleepList = temp;
  //printList("sleepList", sleepList);
  int_on(sr);
}

/*int kexit(int exitValue)
{
  printf("proc %d in kexit(), value=%d\n", running->pid, exitValue);
  running->exitCode = exitValue;
  running->status = ZOMBIE;
  tswitch();
}*/

int kexit(int exitValue)
{

  int i;
  PROC *p;

  if (running->pid==1){
    printf("P1 NO dies\n");
    return -1;
  }

  for (i = 2; i < NPROC; i++)
  { // Skip root proc[0] and proc[1]
    p = &proc[i];
    
    /// Child sent to parent (orphanage)
    if(p->status != FREE && p->ppid == running->pid)
    {
      if (p->status == ZOMBIE) {
        p->status = FREE;
        enqueue(&freeList, p);  //Release all ZOMBIE childs
      }
      else
      {
        printf("Send child &d to proc[1] (parent)\n", p->pid);
        p->ppid = 1;
        p->parent = &proc[1];
      }
    }
  }

  running->exitCode = exitValue;  // Record exitValue in proc for parent
  running->status = ZOMBIE;       // Will not free PROC
  enqueue(&zombieList, p);

  kwakeup(&proc[1]);
  kwakeup(running->parent);

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
