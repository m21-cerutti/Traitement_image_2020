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

unsigned short *pnmToGray(pnm source, unsigned short *dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    unsigned short gray = 0;
    int i, j;
    indexToPosition(index, &i, &j, cols);
    for (int k = 0; k < 3; k++)
    {
      unsigned short val = pnm_get_component(source, i, j, k);
      gray += val;
    }
    gray /= 3;
    dest[index] = gray;
  }
  return dest;
}

void grayToPnm(unsigned short *source, pnm dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, cols);
    unsigned short gray = source[index];
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, i, j, k, gray);
    }
  }
}

void copyComplex(fftw_complex *source, fftw_complex* dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    dest[index] = source[index];
  }
}

void centerComplex(fftw_complex *source, fftw_complex* dest, int rows, int cols)
{
  int middleRow = rows/2;
  int middleCol = cols/2;
 
  fftw_complex tmp[rows*cols];
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, cols);
    int newIndex = positionToIndex((i + middleRow)%rows, (j + middleCol) %cols, rows);
    tmp[newIndex] = source[index];
    //source[index] = pow(-1, i+j) * source[index]; // Marche qu'avec des réels
  }
  copyComplex(tmp, dest, rows, cols);
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
// Padding

void padding(int factor, pnm source, pnm dest, int rows, int cols)
{
  unsigned short gray[rows*cols];
  pnmToGray(source, gray, rows, cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  centerComplex(freq_repr, freq_repr, rows, cols);

  int newSize = (rows*factor)*(cols*factor);
  fftw_complex* paddedTab = calloc(newSize, sizeof(fftw_complex));

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int index = positionToIndex(i, j, rows);
      int newI = i +((rows * factor)-(rows))/2;
      int newJ = j +((cols * factor)-(cols))/2;
      int newindex = positionToIndex(newI, newJ, rows*factor);
      paddedTab[newindex] = freq_repr[index];
    }
  }

  centerComplex(paddedTab, paddedTab, rows*factor, cols*factor);
  
  unsigned short *newgray = backward((rows*factor), (cols*factor), paddedTab, factor);

  grayToPnm(newgray, dest, rows*factor, cols*factor);

  //free
  free(newgray);
  free(freq_repr);
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
  padding(factor, source, dest, rows, cols);
  save_image(dest, "", destName);

  pnm_free(source);
  pnm_free(dest);
  return EXIT_SUCCESS;
}
