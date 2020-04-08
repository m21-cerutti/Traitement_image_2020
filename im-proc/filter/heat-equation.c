#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//////////////////////

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index % cols;
  *j = index / cols;
}

int positionToIndex(int i, int j, const int cols)
{
  return i * cols + j;
}

//////////////////////

#define CORE_HSIZE 1
#define NB_CORE ((2 * CORE_HSIZE + 1) * (2 * CORE_HSIZE + 1))

double CoreHeat[NB_CORE] = {
    0, 0.25, 0,
    0.25, -1, 0.25,
    0, 0.25, 0};

double convulutionPartialGray(pnm source, int i, int j, double core[], int halfsize)
{
  int imsRows = pnm_get_height(source);
  int imsCols = pnm_get_width(source);

  double result = 0;
  for (int x = -halfsize; x <= halfsize; x++)
  {
    for (int y = -halfsize; y <= halfsize; y++)
    {
      int i_f = i + y;
      int j_f = j + x;

      //Border mirrored
      if ((i + y) >= imsRows || (i + y) < 0)
      {
        i_f = i - y;
      }
      if ((j + x) >= imsCols || (j + x) < 0)
      {
        j_f = j - x;
      }

      int coreIndex = positionToIndex(halfsize + y, halfsize + x, 2 * halfsize + 1);
      double contribution = core[coreIndex];
      unsigned short val = pnm_get_component(source, i_f, j_f, 0);
      result += contribution * val;
    }
  }
  return result;
}

//////////////////////

void heat_equation(pnm source_n, int i, int j, pnm dest_np1)
{
  unsigned short result = pnm_get_component(source_n, i, j, 0);
  result += (unsigned short)(convulutionPartialGray(source_n, i, j, CoreHeat, CORE_HSIZE));
  result = MAX(MIN(result,255),0);
  for (int channel = 0; channel < 3; channel++)
  {
    pnm_set_component(dest_np1, i, j, channel, result);
  }
}

void process(int n, char *ims, char *imd)
{
  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm s_n = pnm_dup(input);
  pnm s_np1 = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int k = 0; k < n; k++)
  {
    for (int i = 0; i < imsRows; i++)
    {
      for (int j = 0; j < imsCols; j++)
      {
        heat_equation(s_n, i, j, s_np1);
      }
    }
    //Swap buffers
    pnm tmp = s_n;
    s_n = s_np1;
    s_np1 = tmp;
  }

  pnm_save(s_n, PnmRawPpm, imd);
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s <n> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);
  int n = atoi(argv[1]);
  process(n, argv[2], argv[3]);
  return EXIT_SUCCESS;
}
