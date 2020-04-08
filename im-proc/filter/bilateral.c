#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>
#include <math.h>

#define CORE_HSIZE 10
#define NB_CORE ((2 * CORE_HSIZE + 1) * (2 * CORE_HSIZE + 1))

//////////////////////////////////////
// Utilities fonctions

double Gaussian(int sigma, double k)
{
  return (exp(-((k) * (k)) / (2.0 * (sigma) * (sigma))));
}

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

unsigned short bilateral(pnm source, int i, int j, int cols, int sigma_s, int sigma_g, int *V, int cpt)
{
  double up = 0;
  double down = 0;

  int p = positionToIndex(i, j, cols);
  unsigned short pixel_p = pnm_get_component(source, i, j, 0);

  for (int i = 0; i < cpt; i++)
  {
    int q = V[i];
    int iq, jq;
    indexToPosition(q, &iq, &jq, cols);
    unsigned short pixel_q = pnm_get_component(source, iq, jq, 0);

    double common_factor = Gaussian(sigma_s, abs(p - q)) * Gaussian(sigma_g, abs(pixel_p - pixel_q));
    up += common_factor * pixel_q;
    down += common_factor;
  }
  return (unsigned short)(up / down);
}

void process(int sigma_s, int sigma_g, char *ims, char *imd)
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
      unsigned short res = bilateral(input, i, j, imsCols, sigma_s, sigma_g, V, nbNeighboor);

      for (int channel = 0; channel < 3; channel++)
        pnm_set_component(output, i, j, channel, res);
    }
  }

  pnm_save(output, PnmRawPpm, imd);
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s <sigma_s> <sigma_g> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 4
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);
  process(atoi(argv[1]), atoi(argv[2]), argv[3], argv[4]);
  return EXIT_SUCCESS;
}
