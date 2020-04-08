#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <bcl.h>

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

int
euclidian_dist(int p, int q)
{
  (void)p;
  (void)q;
  return 0;
}

int weight(int p, int q, int sigma)
{
  return exp(-euclidian_dist(p,q))/(2*sigma*sigma);
}

int
nlmeans(int sigma, int p, int* V, int nbNeighboor)
{
  int up = 0;
  int down = 0;
  int common_factor = 0;
  int q = 0;
  for (int i = 0; i < nbNeighboor; i++) {
    q = V[i];
    common_factor = weight(p,q,sigma);
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

  int nbNeighboor = 0;
  int res;
  int halfsize = 1;
  int size = (halfsize+1)*(halfsize+1);
  int V[size*size];
  int p = 0;

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      p = pnm_get_component(input, i, j, 0);
      getNeighboor(i, j , halfsize, input, imsRows, imsCols, V, &nbNeighboor);
      res = nlmeans(sigma, p,  V, nbNeighboor);
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
