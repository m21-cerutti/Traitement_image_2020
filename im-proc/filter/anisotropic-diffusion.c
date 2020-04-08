#include <stdlib.h>
#include <stdio.h>

#include <math.h>
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

      //Border duplication
      if ((i + y) >= imsRows || (i + y) < 0)
      {
        i_f = i;
      }
      if ((j + x) >= imsCols || (j + x) < 0)
      {
        j_f = j;
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

typedef double (*func_diffuse)(double, int);

double detectBorders0(double s, int lambda)
{
  (void)s;
  (void)lambda;
  return 1;
}

double detectBorders1(double s, int lambda)
{
  return 1/(1 + (s/lambda) * (s/lambda));
}

double detectBorders2(double s, int lambda)
{
  return exp(-(s/lambda) * (s/lambda));
}

//////////////////////
#define TEMP_DISC 0.25 // <= 0.25

double CoreHeat[NB_CORE] = {
    0, 1, 0,
    1, -4, 1,
    0, 1, 0};

void anisotropic_equation(pnm source_n, int i, int j, pnm dest_np1, int lambda, func_diffuse diff)
{
  unsigned short result = pnm_get_component(source_n, i, j, 0);
  //Heat
  double conv = TEMP_DISC * convulutionPartialGray(source_n, i, j, CoreHeat, CORE_HSIZE);

  //Anisotropic
  result += (unsigned short)(diff(abs(conv), lambda) * conv);

  //Trunc
  result = MAX(MIN(result, 255), 0);
  for (int channel = 0; channel < 3; channel++)
  {
    pnm_set_component(dest_np1, i, j, channel, result);
  }
}

//////////////////////

void process(int n, int lambda, int function, char *ims, char *imd)
{
  func_diffuse diffuse_funtion;
  switch (function)
  {
  case 0:
    diffuse_funtion = detectBorders0;
    break;
  case 1:
    diffuse_funtion = detectBorders1;
    break;
  case 2:
    diffuse_funtion = detectBorders2;
    break;

  default:
    fprintf(stderr, "Funtion for anistropic %d  not in [0,1,2].\n", lambda);
    exit(EXIT_FAILURE);
    break;
  }

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
        anisotropic_equation(s_n, i, j, s_np1, lambda, diffuse_funtion);
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
  fprintf(stderr, "Usage: %s <n> <lambda> <function> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 5
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);
  int n = atoi(argv[1]);
  int lambda = atoi(argv[2]);
  int func = atoi(argv[3]);
  process(n, lambda, func, argv[4], argv[5]);
  return EXIT_SUCCESS;
}
