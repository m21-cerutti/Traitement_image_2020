#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>
#include <math.h>

#define CORE_HSIZE 11
#define NB_CORE ((2 * CORE_HSIZE + 1) * (2 * CORE_HSIZE + 1))
#define R 7

//////////////////////////////////////
// Utilities fonctions

double Gaussian(int sigma, double k)
{
  double g = exp(-k) / (2.0 * (sigma) * (sigma));
  fprintf(stderr,"g:%lf\n",g);
  fprintf(stderr,"k:%f exp(-k):%lf sigma:%d\n",k, exp(-k), sigma);
  return g;
}

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index / cols;
  *j = index % cols;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
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
        continue;
      int index = positionToIndex(i + y, j + x, cols);
      V[cpt] = index;
      cpt++;
    }
  }
  *nbNeighboor = cpt;
}

float euclidian_dist(int p, int q, pnm source, int cols, int rows)
{
  float res = 0;
  int diff = 0;
  int n = 0;
  int pi, pj;
  indexToPosition(p, &pi, &pj, cols);
  int qi, qj;
  indexToPosition(q, &qi, &qj, cols);
  for (int u = -R; u <= R; u++) {
    for (int v = -R; v <= R; v++) {
      //ignore border
      //fprintf(stderr,"cols:%d\n",cols);
      //fprintf(stderr,"rows:%d\n",rows);
      if ((pi + u) < 0 || (pi + u) >= rows)
        continue;
      //fprintf(stderr,"pi:%d\n",pi+u);
      if ((pj + v) < 0 || (pj + v) >= cols)
        continue;
      if ((qi + u) < 0 || (qi + u) >= rows)
        continue;
      //fprintf(stderr,"qi:%d\n",qi+u);
      if ((qj + v) < 0 || (qj + v) >= cols)
        continue;
      diff = pnm_get_component(source, pi + u, pj + v, 0);
      diff -= pnm_get_component(source, qi + u, qj + v, 0);
      res += diff*diff;
      n++;
    }
  }
  return res/n;
}

double weight(int sigma, int p, int q, pnm source, int cols, int rows)
{
  double d = euclidian_dist(p, q, source, cols, rows);
  fprintf(stderr,"res:%f\n",d);
  return Gaussian(sigma, d);
}

unsigned short nlmeans(pnm source, int i, int j, int sigma, int cols, int rows, int *V, int nbNeighbour)
{
  double up = 0;
  double down = 0;

  int p = positionToIndex(i, j, cols);
  for (int i = 0; i < nbNeighbour; i++)
  {
    int q = V[i];
    int iq, jq;
    indexToPosition(q, &iq, &jq, rows);
    unsigned short pixel_q = pnm_get_component(source, iq, jq, 0);

    double common_factor = weight(sigma, p, q, source, cols, rows);
    //fprintf(stderr,"w:%f",common_factor);
    up += common_factor * pixel_q;
    down += common_factor;
  }
  return (unsigned short)(up / down);
}

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
      fprintf(stderr,"p : %d - ",pnm_get_component(input, i, j, 0));
      unsigned short res = nlmeans(input, i, j, sigma, imsCols, imsRows, V, nbNeighboor);
      fprintf(stderr,"p': %d\n",res);
      for (int channel = 0; channel < 3; channel++)
        pnm_set_component(output, i, j, channel, res);
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
