#define TLOAD 0x0
#define TVALUE 0x1
#define TCNTL 0x2
#define TINTCLR 0x3
#define TRIS 0x4
#define TMIS 0x5
#define TBGLOAD 0x6

typedef volatile struct timer{
    u32 *base; // timer's base address; as u32 pointer
    int tick, ss, proc_id; // per timer data area
    char clock[16];
}TIMER;

int total_timers = 0;

volatile TIMER timer[4]; //4 timers; 2 per unit; at 0x00 and 0x20
void timer_init()
{
    int i; TIMER *tp;
    printf("timer_init()\n");
    for (i=0; i<4; i++){
        tp = &timer[i];
        if (i==0) tp->base = (u32 *)0x101E2000;
        if (i==1) tp->base = (u32 *)0x101E2020;
        if (i==2) tp->base = (u32 *)0x101E3000;
        if (i==3) tp->base = (u32 *)0x101E3020;
        *(tp->base+TLOAD) = 0x0; // reset
        *(tp->base+TVALUE)= 0xFFFFFFFF;
        *(tp->base+TRIS) = 0x0;
        *(tp->base+TMIS) = 0x0;
        *(tp->base+TLOAD) = 0x100;
        // CntlReg=011-0010=|En|Pe|IntE|-|scal=01|32bit|0=wrap|=0x66
        *(tp->base+TCNTL) = 0x62;
        *(tp->base+TBGLOAD) = 0x1C00; // timer counter value
        tp->proc_id = 0;
        tp->ss = 0; // initialize wall clock
        strcpy((char *)tp->clock, "");
    }
}

int timer_handler(int n) {
    int i;
    TIMER *t = &timer[n];
    char label[4];
        t->tick++; // Assume 120 ticks per second
    if (t->ss != 0) {

        if (t->tick==100*total_timers){
            t->tick = 0;
            if (n == 0)
                t->ss++;
            else
                t->ss--;
            t->clock[7]='0'+(t->ss%10);
            t->clock[6]='0'+(t->ss/10);
        }

            

        if (n == 0) {
            color = GREEN; // display in different color
            label[n] = 'C';
        } else {
            color = RED; // display in different color
            label[n] = 'P';
        }
        label[1] = (t->proc_id + '0');
        label[2] = ':';
        label[3] = ' ';

        for (i=0; i<8; i++){
            kpchar(label[i], n, 65+i); // to line n of LCD
            kpchar(t->clock[i], n, 70+i); // to line n of LCD
        }
        color = YELLOW;             // Revert to original colo
        timer_clearInterrupt(n); // clear timer interrupt
    } else {
        timer_clearInterrupt(n); // clear timer interrupt
        /*int last_timer_index = total_timers-1;
        if (n > 0 && n != last_timer_index) { //if timer is > 2, swap to keep array consistent. Move deleting to end of list
            printf("---- SWAPPING:      %d > 0, timer[%d]\n", n, total_timers-1);
            //TIMER *last_timer = &timer[last_timer_index];
            //TIMER *timer_n = &timer[n];
            //timer_n->proc_id = last_timer->proc_id;
            //timer_n->ss = last_timer->ss;
            //timer_n->tick = last_timer->tick;
            //timer[n] = timer[last_timer_index];

            //printf("timer[%d] id = %d, timer[%d] id = %d\n", n, timer_n->proc_id, total_timers-1, last_timer->proc_id);
        }
        return timer_stop(last_timer_index);    //remove last timer in array
        */
        return timer_stop(n);
    }
    return 0;
}

void timer_start(int n, int proc_id, int sec) // timer_start(0), 1, etc.
{
    TIMER *tp = &timer[n];
    tp->proc_id = proc_id;
    tp->ss = sec;
    kprintf("timer_start %d base=%x\n", n, tp->base);
    *(tp->base+TCNTL) |= 0x80; // set enable bit 7
    total_timers++;
}

int timer_clearInterrupt(int n) // timer_start(0), 1, etc.
{
    TIMER *tp = &timer[n];
    *(tp->base+TINTCLR) = 0xFFFFFFFF;
}

int timer_stop(int n) // stop a timer
{
    TIMER *tp = &timer[n];
    *(tp->base+TCNTL) &= 0x7F; // clear enable bit 7
    total_timers--;
    return tp->proc_id;
}