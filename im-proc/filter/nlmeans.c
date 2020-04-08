#include <stdlib.h>
#include <stdio.h>

#include <math.h>
#include <bcl.h>

#define CORE_HSIZE 5
#define NB_CORE ((2 * CORE_HSIZE + 1) * (2 * CORE_HSIZE + 1))

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index / cols;
  *j = index % cols;
}

int positionToIndex(int i, int j, const int cols)
{
  return i * cols + j;
}

//////////////////////////////////////

void getNeighboor(int i, int j, int halfsize, int rows, int cols, int *V, int *nbNeighboor)
{
  int cpt = 0;

  for (int x = -halfsize; x <= halfsize; x++)
  {
    for (int y = -halfsize; y <= halfsize; y++)
    {
      //Border ignored
      if ((i + y) >= rows || (i + y) < 0 || (j + x) >= cols || (j + x) < 0)
      {
        continue;
      }
      int index = positionToIndex(i + y, j + x, cols);
      V[cpt] = index;
      cpt++;
    }
  }
  *nbNeighboor = cpt;
}

int euclidian_dist(int p, int q, pnm source)
{
  int cols = pnm_get_height(source);
  int rows = pnm_get_width(source);

  int ip, jp, iq, jq;
  indexToPosition(p, &ip, &jp, cols);
  indexToPosition(q, &iq, &jq, cols);

  double sum = 0;
  int cpt = 0;
  for (int x = -CORE_HSIZE; x <= CORE_HSIZE; x++)
  {
    for (int y = -CORE_HSIZE; y <= CORE_HSIZE; y++)
    {
      //Border ignored
      if ((ip + y) >= rows || (ip + y) < 0 || (jp + x) >= cols || (jp + x) < 0)
      {
        continue; //p
      }
      if ((iq + y) >= rows || (iq + y) < 0 || (jq + x) >= cols || (jq + x) < 0)
      {
        continue; //q
      }
      unsigned short pixel_p = pnm_get_component(source, ip + y, jq + x, 0);
      unsigned short pixel_q = pnm_get_component(source, iq + y, jq + x, 0);
      sum += pixel_p - pixel_q;
      cpt++;
    }
  }
  return sum / cpt;
}

double weight(int p, int q, int sigma, pnm source)
{
  return exp(-euclidian_dist(p, q, source)) / (2 * sigma * sigma);
}

//////////////////////////////////////

unsigned short nlmeans(pnm source, int i, int j, int cols, int sigma, int *V, int nbNeighbour)
{
  double up = 0;
  double down = 0;

  int p = positionToIndex(i, j, cols);

  for (int i = 0; i < nbNeighbour; i++)
  {
    int q = V[i];
    int iq, jq;
    indexToPosition(q, &iq, &jq, cols);
    unsigned short pixel_q = pnm_get_component(source, iq, jq, 0);

    double common_factor = weight(p, q, sigma, source);
    up += common_factor * pixel_q;
    down += common_factor;
  }
  return (unsigned short)(up / down);
}

//////////////////////////////////////

void process(int sigma, char *ims, char *imd)
{
  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      int V[NB_CORE];
      int nbNeighboor = 0;
      getNeighboor(i, j, CORE_HSIZE, imsRows, imsCols, V, &nbNeighboor);
      unsigned short res = nlmeans(input, i, j, imsCols, sigma, V, nbNeighboor);
      for (int channel = 0; channel < 3; channel++)
      {
        pnm_set_component(output, i, j, channel, res);
      }
    }
  }

  pnm_save(output, PnmRawPpm, imd);
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s <sigma> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);
  process(atoi(argv[1]), argv[2], argv[3]);
  return EXIT_SUCCESS;
}
