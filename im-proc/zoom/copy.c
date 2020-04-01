#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <fft.h>

#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define FOLDER_TEST ""

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index % cols;
  *j = index / cols;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

void getSizePnm(pnm source, int *rows, int *cols)
{
  *cols = pnm_get_width(source);
  *rows = pnm_get_height(source);
}

void save_image(pnm img, char *prefix, char *name)
{
  int output_size = strlen(prefix) +strlen(FOLDER_TEST) + strlen(name);
  char output[output_size];

  char *bname, *dname;
  dname = dir_name(name);
  bname = base_name(name);
  sprintf(output, "%s/%s%s%s", dname, FOLDER_TEST, prefix, bname);
  pnm_save(img, PnmRawPpm, output);
}

//////////////////////////////////////
// Copy

void copy(int factor, pnm source, pnm dest, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      for (int k = 0; k < 3; k++)
      {
        const unsigned short val = pnm_get_component(source, i, j, k);
        for (int fi = 0; fi < factor; fi++)
        {
          for (int fj = 0; fj < factor; fj++)
          {
            pnm_set_component(dest, factor*i+fi, factor*j+fj, k, val);
          }
        }
      }
    }
  }
}


void usage(const char *s)
{
  fprintf(stderr, "Usage: %s <factor> <ims> <imd> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);

  int factor = atoi(argv[1]);
  pnm source = pnm_load(argv[2]);

  int rows, cols;
  getSizePnm(source, &rows, &cols);

  char* destName = argv[3];
  pnm dest = pnm_new(factor * cols, factor * rows, PnmRawPpm);
  copy(factor, source, dest, rows, cols);
  save_image(dest, "", destName);

  pnm_free(source);
  pnm_free(dest);
  return EXIT_SUCCESS;
}
