#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int rows)
{
  *i = index % rows;
  *j = index / rows;
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
nmleans(int* V, int cpt, int p)
{
  int up = 0;
  int down = 0;
  int common_factor = 0;
  for (int i = 0; i < cpt; i++) {
    q = V[i];
    common_factor = Gaussian(sigma_s, p-q)*Gaussian(sigma_g, p-q);
    up += common_factor * q;
    down += common_factor;
  }
  return up/down;
}

void
process(int sigma, char *ims, char *imd)
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

      getNeighboor(V, &cpt, j, i, imsCols, imsRows, halfsize);
      //printf("i:%d j:%d \n", i, j);


      for (int i = 0; i < cpt; i++) {
        //printf("index : %d\n", V[i]);
        indexToPosition(V[i],&x,&y,imsCols);
        V_value[i] = pnm_get_component(input, x, y, 0);
      }
      //printf("coucou\n");

      res = median(V_value, cpt);
      int print = pnm_get_component(input, i, j, 0);
      printf("Pixel : %d\n",print);

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
  fprintf(stderr, "Usage: %s <sigma> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  process(atoi(argv[1]), argv[2], argv[3]);
  return EXIT_SUCCESS;
}
