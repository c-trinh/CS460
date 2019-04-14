/********************************************************************
Copyright 2010-2017 K.C. Wang, <kwang@eecs.wsu.edu>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

int kgetpid()
{
  return running->pid;
}

int kgetppid()
{
  return running->ppid;
}

char *pstatus[]={"FREE   ","READY  ","SLEEP  ","BLOCK  ","ZOMBIE", " RUN  "};
int kps()
{
  int i; PROC *p; 
  for (i=0; i<NPROC; i++){
     p = &proc[i];
     kprintf("proc[%d]: pid=%d ppid=%d", i, p->pid, p->ppid);
     if (p==running)
       printf("%s ", pstatus[5]);
     else
       printf("%s", pstatus[p->status]);
     printf("name=%s\n", p->name);
  }
}

int kchname(char *s)
{ 
  kprintf("kchname: name=%s\n", s);
  strcpy(running->name, s);
  return 123;
}

int ktswitch()
{
  tswitch();
}

int kgetPA()
{
  return running->pgdir[2048] & 0xFFFF0000;
}

int kkfork()
{ /// Only loads U1
  printf("[ ENTER KFORK HELPER FUNCTION ]\n");
  PROC *p = kfork("u1");
  
  if (p)
    return p->pid;
  return -1;
}

int kkwait() {
  printf("[ ENTER KWAIT HELPER FUNCTION ]\n");
  kwait(running->status);
}

int kkexit() {
  printf("[ ENTER KEXIT HELPER FUNCTION ]\n");
  kexit(running->pid);
}

int kksleep() {
  printf("[ ENTER KSLEEP HELPER FUNCTION ]\n");
  int e;
  printf("Input an event to sleep on: ");
  e = geti();
  printf("\nEvent: %d\n", e);
  ksleep(e);
}

int kkwakeup() {
  printf("[ ENTER KWAKEUP HELPER FUNCTION ]\n");
  int e;
  printf("Input an event to sleep on: ");
  e = geti();
  printf("\nEvent: %d\n", e);
  kwakeup(e);
}

// called from svc_entry in ts.s
int svc_handler(int a, int b, int c, int d)
{
  int r;

  switch(a){
    case 0: r = kgetpid();          break;
    case 1: r = kgetppid();         break;
    case 2: r = kps();              break;
    case 3: r = kchname((char *)b); break;
    case 4: r = ktswitch();         break;

    // Insert Syscalls Here: kwait, ksleep, kwakeup
    case 5: r = kkfork();           break;
    case 6: r = kkwait();           break;
    case 7: r = kkexit();           break;
    case 8: r = kksleep();          break;
    case 9: r = fork();             break;
    case 10: r = exec((char *) b);           break;
    case 11: r = kkwakeup();        break;

    case 90: r = kgetc() &0x7F;     break;
    case 91: r = kputc(b);          break;
    case 92: r = kgetPA();          break;

    default: printf("invalid syscall %d\n", a);
  }
  
  return r;  // return to goUmode in ts.s; replace r0 in Kstack with r
}
