#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include <bcl.h>
#include <se.h>

enum {SQUARE, DIAMOND, DISK, LINE_V, DIAG_R, LINE_H, DIAG_L, CROSS, PLUS};

void drawLineH(pnm shape, int hs) {
  int size = 2*hs+1;
  for (int y = 0; y < size; y++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, hs+1, y, channel, 255);
    }
  }
}

void drawLineV(pnm shape, int hs) {
  int size = 2*hs+1;
  for (int x = 0; x < size; x++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, x, hs+1, channel, 255);
    }
  }
}

void drawDiagL(pnm shape, int hs) {
  int size = 2*hs+1;
  for (int i = 0; i < size; i++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, i, i, channel, 255);
    }
  }
}

void drawDiagR(pnm shape, int hs) {
  int size = 2*hs+1;
  for (int i = 0 ; i < size; i++) {
    for (int channel = 0; channel < 3; channel++) {
      pnm_set_component(shape, ((size-1) - i), i, channel, 255);
    }
  }
}

void
Square(pnm shape, int hs)
{
  int size = 2*hs+1;

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
 int size = 2*hs+1;

  for (int i = 0; i < size/2; i++) {
    int act_half_size = i;
    for (int j = -act_half_size; j < act_half_size; j++) {
      for (int channel = 0; channel < 3; channel++) {
        pnm_set_component(shape, i, hs + j, channel, 255);
      }
    }
  }

  for (int i = 0; i < size; i++) {

    int act_half_size = hs - i;

    for (int j = -act_half_size; j < act_half_size; j++) {
      for (int channel = 0; channel < 3; channel++) {
        pnm_set_component(shape, hs + i, hs + j, channel, 255);
      }
    }
  }
}

void
Disk(pnm shape, int hs)
{
  int size = 2*hs+1;
  int radius = hs*hs;

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {

      int xMinusR = i-hs;
      int yMinusR = j-hs;

      int dist = xMinusR * xMinusR + yMinusR * yMinusR;
      if (dist <= radius) {
        for (int channel = 0; channel < 3; channel++) {
          pnm_set_component(shape, i, j, channel, 255);
        }
      }
    }
  }
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

  int size = 2*hs+1;
  pnm shape = pnm_new(size, size, PnmRawPpm);

  switch(s) {
    case SQUARE:
      Square(shape, hs);
      break;
    case DIAMOND:
      Diamond(shape, hs);
      break;
    case DISK:
      Disk(shape, hs);
      break;
    case LINE_V:
      LineV(shape, hs);
      break;
    case DIAG_R:
      DiagR(shape, hs);
      break;
    case LINE_H:
      LineH(shape, hs);
      break;
    case DIAG_L:
      DiagL(shape, hs);
      break;
    case CROSS:
      Cross(shape, hs);
      break;
    case PLUS:
      Plus(shape, hs);
      break;
    default:
      puts("Shape doesn't exist\n");
      exit(1);
  }

  return shape;
}
