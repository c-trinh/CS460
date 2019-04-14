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

#define DR 0x00
#define FR 0x18

#define RXFE 0x10
#define TXFF 0x20

//char *tab = "0123456789ABCDEF";

typedef struct uart
{
  char *base;
  int n;
} UART;

UART uart[4];

int uart_init()
{
  int i;
  UART *up;

  for (i = 0; i < 4; i++)
  {
    up = &uart[i];
    up->base = (char *)(0x101F1000 + i * 0x1000);
    up->n = i;
  }
  uart[3].base = (char *)(0x10009000); // uart3 at 0x10009000
}

int ugetc(UART *up)
{
  while (*(up->base + FR) & RXFE)
    ;
  return *(up->base + DR);
}

int uputc(UART *up, char c)
{
  while (*(up->base + FR) & TXFF)
    ;
  *(up->base + DR) = c;
}

int ugets(UART *up, char *s)
{
  while ((*s = (char)ugetc(up)) != '\r')
  {
    uputc(up, *s);
    s++;
  }
  *s = 0;
}

int uprints(UART *up, char *s)
{
  while (*s)
    uputc(up, *s++);
}

//Source(s): Book + vid.c (Lab 2 Part 2)

int urpx(UART *up, int x)
{
  char c;
  if (x)
  {
    c = tab[x % 16];
    urpx(up, x / 16);
  }
  uputc(up, c);
}

int urpu(UART *up, int x)
{
  char c;
  if (x)
  {
    c = tab[x % 10];
    urpu(up, x / 10);
  }
  uputc(up, c);
}

int uprintx(UART *up, int x)
{
  uprints(up, "0x");
  if (x == 0)
    uputc(up, '0');
  else
    urpx(up, x);
  uputc(up, ' ');
}

int uprintu(UART *up, int x)
{
  if (x == 0)
    uputc(up, '0');
  else
    urpu(up, x);
  uputc(up, ' ');
}

int uprinti(UART *up, int x)
{
  if (x < 0)
  {
    uputc(up, '-');
    x = -x;
  }
  uprintu(up, x);
}

/** WRITE YOUR uprintf(UART *up, char *fmt, . . .) for formatted print **/
int uprintf(char *fmt, ...)
{
  int *ip;
  char *cp;
  cp = fmt;
  ip = (int *)&fmt + 1;

  UART *up = &uart[0];

  while (*cp)
  {
    if (*cp != '%')
    {
      uputc(up, *cp);
      if (*cp == '\n')
        uputc(up, '\r');
      cp++;
      continue;
    }
    cp++;
    switch (*cp)
    {
    case 'c':
      uputc(up, (char)*ip);
      break;
    case 's':
      uprints(up, (char *)*ip);
      break;
    case 'd':
      uprinti(up, *ip);
      break;
    case 'u':
      uprintu(up, *ip);
      break;
    case 'x':
      uprintx(up, *ip);
      break;
    }
    cp++;
    ip++;
  }
}