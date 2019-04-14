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
/**************************
 need to catch Control-C, Contorl-D keys
 so need to recognize LCTRL key pressed and then C or D key
 Ref: 5.6.2.1
******************************/

#include "keymap"
#include "keymap2"

#define LSHIFT 0x12
#define RSHIFT 0x59
#define ENTER 0x5A
#define LCTRL 0x14
#define RCTRL 0xE014

#define ESC 0x76
#define F1 0x05
#define F2 0x06
#define F3 0x04
#define F4 0x0C

#define KCNTL 0x00
#define KSTAT 0x04
#define KDATA 0x08
#define KCLK 0x0C
#define KISTA 0x10

typedef volatile struct kbd
{
  char *base;
  char buf[128];
  int head, tail;
  int data, room;
} KBD;

volatile KBD kbd;
int shifted = 0;
int release = 0;
int control = 0;

volatile int keyset;
int kputc(char);
extern int color;

int kbd_init()
{
  char scode;
  keyset = 1; // default to scan code set-1

  KBD *kp = &kbd;
  kp->base = (char *)0x10006000;
  *(kp->base + KCNTL) = 0x14; // bit4=Enable bit0=INT on  //!!! 0x10 -> 0x14
  *(kp->base + KCLK) = 8;
  kp->head = kp->tail = 0;

  kp->data = 0;
  kp->room = 128;

  shifted = 0;
  release = 0;
  control = 0;

  printf("Detect KBD scan code: press the ENTER key : ");
  while ((*(kp->base + KSTAT) & 0x10) == 0)
    ;
  scode = *(kp->base + KDATA);
  printf("scode=%x ", scode);
  if (scode == 0x5A)
    keyset = 2;
  printf("keyset=%d\n", keyset);
}

// kbd_handler1() for scan code set 1
void kbd_handler1()
{
  u8 scode, c;
  KBD *kp = &kbd;

  scode = *(kp->base + KDATA);

  //printf("scan code = %x ", scode);

  if (scode & 0x80)
    return;
  c = unsh[scode];

  if (c == '\r')
    kputc('\n');
  kputc(c);

  kp->buf[kp->head++] = c;
  kp->head %= 128;

  kp->data++;
  kp->room--;
  kwakeup(&kp->data);
}

// kbd_handelr2() for scan code set 2
void kbd_handler2()
{
  // YOUR kbd handler for SET 2 scan code
}

/*void kbd_handler()
{
  if (keyset == 1)
    kbd_handler1();
  else
    kbd_handler2();
}*/

void kbd_handler()
{
  u8 scode, c;

  KBD *kp = &kbd;
  color = YELLOW;              // int color in vid.c file
  scode = *(kp->base + KDATA); // get scan code from KDATA reg => clear IRQ

  //printf("scanCode = %x\n", scode);

  if (scode == 0xF0)
  {              // key release
    release = 1; // set flag
    return;
  }

  if (scode == 0x12)
  { //L Shift is being pressed
    shifted = 1;
  }

  if (scode == 0x14)
  { //L Ctrl is being pressed
    control = 1;
  }

  if (scode == 0x12 && release == 1)
  { //L Shift is released
    shifted = 0;
  }

  if (scode == 0x14 && release == 1)
  { //L Ctrl is released
    control = 0;
  }

  if (release && scode)
  {              // next scan code following key release
    release = 0; // clear flag
    return;
  }

  if (shifted && scode)
    c = utab[scode]; //Lowercase
  else               // ONLY IF YOU can catch LEFT or RIGHT shift key
    c = ltab[scode]; //Uppercase
  if (control && scode)
  {
    if (scode == 0x21)
      printf("PROGRAM IS TERMINATED\n");
    if (scode == 0x23)
    {
      c = 0x04;
      printf("FILE STREAMING IS TERMINATED\n");
    }
  }

  if (c == '\r')
    kputc('\n');
  kputc(c);

  kp->buf[kp->head++] = c; // Enter key into CIRCULAR buf[]
  kp->head %= 128;
  kp->data++;
  kp->room--;
  //kwakeup(&kp->data);
}

int kgetc()
{
  char c;
  KBD *kp = &kbd;

  while (kp->data == 0)
    ; // BUSY wait for data

  // !!!
  /*while(kp->data == 0) {
    ksleep(&kp->data);
  }*/

  lock();
  c = kp->buf[kp->tail++];
  kp->tail %= 128;
  kp->data--;
  kp->room++;
  unlock();

  return c;
}

/*int kgetc()
{
  char c;
  KBD *kp = &kbd;

  while(1) {
    lock();
    if (kp->data==0) {
      unlock();
      ksleep(&kp->data);
    }
    else {
      break;
    }
  }

   c = kp->buf[kp->tail++];
   kp->tail %= 128;
   kp->data--; kp->room++;
  unlock();
  return c;
}*/

int kgets(char s[])
{
  char c;
  KBD *kp = &kbd;

  while ((c = kgetc()) != '\r')
  {
    if (c == '\b')
    {
      s--;
      continue;
    }
    *s++ = c;
  }
  *s = 0;
  return strlen(s);
}
