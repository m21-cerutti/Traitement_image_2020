#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>
#include <math.h>

//////////////////////////////////////
// Utilities fonctions
#define Gaussian(sigma, k) exp(-((k)*(k))/(2.0*(sigma)*(sigma)))

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index % cols;
  *j = index / cols;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

void swap (int* a, int* b)
{
  int t = *a;
  *a = *b;
  *b = t;
}

int split (int *V, int start, int end)
{
  int pivot =  V[end];
  int i = start - 1;

  for (int j = start; j <= end-1; j++) {
    if (V[j] < pivot) {
      i++;
      swap(&V[i], &V[j]);
    }
  }
  return i+1;
}

void
quickSort(int *V, int start, int end)
{
  if (start < end)
  {
    int s = split (V, start, end);
    quickSort(V, start, s-1);
    quickSort(V, s+1, end);
  }
}

int
median(int *V, int size)
{
  quickSort(V, 0, size);
  /*
  for (int i = 0; i < size; i++) {
    printf("%d\n", V[i]);
  }
  */
  printf("mediane :%d -",V[size/2]);
  return V[size/2];
  /*
  double median;
  if (size%2 == 0) {
    median = (V[size/2] + V[size/2 +1])/2;
    return (int)median;
    printf("median :%f -",median);
  }
  printf("mediane :%d -",V[size/2 +1]);
  return V[size/2 +1];
  */
}

void getNeighboor(int *N, int *nbNeighboor, int i, int j, int imsCols, int imsRows, int halfsize)
{
  int cpt = 0;
  for (int x = -halfsize; x < halfsize+1; x++)
  {
    for (int y = -halfsize; y < halfsize+1; y++)
    {
      if ((i+y) >= imsRows || (i+y) < 0)
        continue;
      if ((j+x) >= imsCols || (j+x) < 0)
        continue;
      //printf("x:%d y:%d \n", x, y);

      N[cpt] = positionToIndex(i+y,j+x,imsRows);
      cpt++;

      //printf("i+x:%d j+y:%d \n", i+x, j+y);
      //printf("coucou 2\n");

    }
  }

  *nbNeighboor = cpt;
}

int
bilateral(int sigma_s,int sigma_g, int* V, int cpt, int p)
{
  int up = 0;
  int down = 0;
  int common_factor = 0;
  int q = 0;
  for (int i = 0; i < cpt; i++) {
    q = V[i];
    common_factor = Gaussian(sigma_s, p-q)*Gaussian(sigma_g, p-q);
    up += common_factor * q;
    down += common_factor;
  }
  return up/down;
}

void
process(int sigma_s, int sigma_g, char *ims, char *imd)
{

  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      int V[(2*halfsize+1)*(2*halfsize+1)];
      int cpt = 0;
      int res, x, y;
      int V_value[cpt];

      getNeighboor(V, &cpt, j, i, imsCols, imsRows, halfsize, pixel);
      //printf("i:%d j:%d \n", i, j);

      res = bilateral(sigma_s, sigma_g, V, cpt);
      for (int channel = 0; channel < 3; channel++)
      {
        pnm_set_component(output, i, j, channel, res);
      }
    }
  }

  pnm_save(output, PnmRawPpm, imd);
}

void
usage (char *s){
  fprintf(stderr, "Usage: %s <sigma_s> <sigma_g> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 4
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  process(atoi(argv[1]), atoi(argv[2]), argv[3], argv[4]);
  return EXIT_SUCCESS;
}
