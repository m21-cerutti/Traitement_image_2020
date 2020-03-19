#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include <bcl.h>
#include <se.h>

enum {SQUARE, DIAMOND, DISK, LINE_V, DIAG_R, LINE_H, DIAG_L, CROSS, PLUS};

void drawLineH(pnm shape, int hs) {
  int size = 2*hs + 1;
  for (int y = 0; y < size; y++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, hs+1, y, channel, 255);
    }
  }
}

void drawLineV(pnm shape, int hs) {
  int size = 2*hs + 1;
  for (int x = 0; x < size; x++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, x, hs+1, channel, 255);
    }
  }
}

void drawDiagL(pnm shape, int hs) {
  int size = 2*hs + 1;
  for (int i = 0; i < size; i++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, i, i, channel, 255);
    }
  }
}

void drawDiagR(pnm shape, int hs) {
  int size = 2*hs + 1;
  for (int i = 0 ; i < size; i++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, ((size-1) - i), i, channel, 255);
    }
  }
}

void
Square(pnm shape, int hs)
{
  int size = 2*hs + 1;

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      for (int channel = 0; channel < 3; channel++) {
        pnm_set_component(shape, i, j, channel, 255);
      }
    }
  }
}

void
Diamond(pnm shape, int hs)
{
  (void)shape;
  (void)hs;
}

void
Disk(pnm shape, int hs)
{
  (void)shape;
  (void)hs;
}

void
LineV(pnm shape, int hs)
{
  drawLineV(shape, hs);
}

void
DiagR(pnm shape, int hs)
{
  drawDiagR(shape, hs);
}

void
LineH(pnm shape, int hs)
{
  drawLineH(shape, hs);
}

void
DiagL(pnm shape, int hs)
{
  drawDiagL(shape, hs);
}

void
Cross(pnm shape, int hs)
{
  drawDiagR(shape, hs);
  drawDiagL(shape, hs);
}

void
Plus(pnm shape, int hs)
{
  drawLineH(shape, hs);
  drawLineV(shape, hs);
}

pnm
se(int s, int hs){

  int size = 2*hs + 1;
  pnm shape = pnm_new(size, size, PnmRawPpm);

  switch(s) {
    case 0:
      Square(shape, hs);
      break;
    case 1:
      puts("diamond");
      //Diamond(shape, hs);
      break;
    case 2:
      //Disk(shape, hs);
      puts("disk");
      break;
    case 3:
      LineV(shape, hs);
      break;
    case 4:
      DiagR(shape, hs);
      break;
    case 5:
      LineH(shape, hs);
      break;
    case 6:
      DiagL(shape, hs);
      break;
    case 7:
      Cross(shape, hs);
      break;
    case 8:
      Plus(shape, hs);
      break;
    default:
      puts("Shape doesn't exist\n");
      exit(1);
  }

  return shape;
}
