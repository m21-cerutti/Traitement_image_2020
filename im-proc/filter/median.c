 #include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

int
median(int* V, int size)
{
  qsort(V, size, sizeof(int), cmpfunc);
  return V[size/2];
}

void getNeighboor(int ip, int jp, int halfsize, pnm source, int rows, int cols, int *V, int *nbNeighboor)
{
  int cpt = 0;

  for (int i = ip - halfsize; i <= (ip + halfsize); i++)
  {
    for (int j = jp - halfsize; j <= (jp + halfsize); j++)
    {
      //Ignore border
      if(i < 0  ||  i > rows || j <0 || j > cols)
        continue;

      V[cpt] = pnm_get_component(source, i, j, 0);
      cpt++;
    }
  }
  *nbNeighboor = cpt;
}

void
process(int halfsize, char *ims, char *imd)
{
  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);
  int size = (2*halfsize+1);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  int nbNeighboor = 0;
  int res;
  int V[size*size];

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      getNeighboor(i, j , halfsize, input, imsRows, imsCols, V, &nbNeighboor);
      res = median(V, nbNeighboor);
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
  fprintf(stderr, "Usage: %s <halfsize> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  process(0, NULL, NULL);
  return EXIT_SUCCESS;
}
