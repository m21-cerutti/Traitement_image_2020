/**
 * @file test-fft.c
 * @brief test the behaviors of functions in fft module
 *
 */

#define _GNU_SOURCE  
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <fft.h>

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int rows)
{
  *i = index % rows;
  *j = index / rows;
}

unsigned short *pnmTOGray(pnm source, int *rows, int *cols)
{
  *cols = pnm_get_width(source);
  *rows = pnm_get_height(source);

  int size = sizeof(unsigned short) * (*cols) * (*rows);
  unsigned short *dest = (unsigned short *)malloc(size);

  for (int index = 0; index < ((*rows) * (*cols)); index++)
  {
    unsigned short gray = 0;
    int i, j;
    indexToPosition(index, &i, &j, *rows);
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
    indexToPosition(index, &i, &j, rows);
    unsigned short gray = source[index];
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, i, j, k, gray);
    }
  }
}

void floatToPnm(float *source, pnm dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, rows);
    unsigned short gray = (source[index] / (1.0*rows*cols))*255;
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, i, j, k, gray);
    }
  }
}

void centerImageFloatAndPnm(float *source, pnm dest, int rows, int cols)
{
  int middleRow = (rows*1.0)/2;
  int middleCol = (cols*1.0)/2;
  float max = -10e30;
  float min = 10e30;

  for (int index = 0; index < (rows * cols); index++)
  {
    if(source[index] > max)
    {
      max = source[index] ;
    }
    if(source[index] < min)
    {
      min = source[index] ;
    }
  }

  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, rows);
    //unsigned short gray = ((source[index] - min) / (max-min))*255;
    //unsigned short gray = pow((source[index] - min) / (max-min), 0.1f)*255;
    unsigned short gray = pow((source[index] / max), 0.1)*255;
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, (i + middleRow)%rows, (j + middleCol) %cols, k, gray);
    }
  }
}

void save_image(pnm img, char *prefix, char *name)
{
  int output_size = strlen(prefix) + strlen(name);
  char output[output_size];

  char *dirc, *basec, *bname, *dname;
  dirc = strdup(name);
  basec = strdup(name);
  dname = dir_name(dirc);
  bname = base_name(basec);
  sprintf(output, "%s/%s%s", dname, prefix, bname);
  pnm_save(img, PnmRawPpm, output);
  printf("%s saved.\n",output);
}

//////////////////////////////////////
//Test structure

/**
 * @brief test the forward and backward functions
 * @param char* name, the input image file name
 */
void test_forward_backward(char *name)
{
  fprintf(stderr, "test_forward_backward: ");

  pnm source = pnm_load(name);

  int rows, cols;
  unsigned short *gray = pnmTOGray(source, &rows, &cols);

  //Test
  fftw_complex *freq_repr = forward(rows, cols, gray);
  unsigned short *newgray = backward(rows, cols, freq_repr);

  pnm imgReconstruct = pnm_new(cols, rows, PnmRawPpm);
  grayToPnm(newgray, imgReconstruct, rows, cols);

  save_image(imgReconstruct, "FB-", name);

  //free
  pnm_free(imgReconstruct);
  free(newgray);
  free(freq_repr);
  free(gray);
  pnm_free(source);

  fprintf(stderr, "OK\n");
}

/**
 * @brief test image reconstruction from of magnitude and phase spectrum
 * @param char *name: the input image file name
 */
void test_reconstruction(char *name)
{
  fprintf(stderr, "test_reconstruction: ");

  pnm source = pnm_load(name);
  int rows, cols;
  unsigned short *gray = pnmTOGray(source, &rows, &cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  //Test
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);
  spectra2freq(rows, cols, as, ps, freq_repr);
  
  unsigned short *newgray = backward(rows, cols, freq_repr);

  pnm imgReconstruct = pnm_new(cols, rows, PnmRawPpm);
  grayToPnm(newgray, imgReconstruct, rows, cols);

  save_image(imgReconstruct, "FB-", name);

  //free
  pnm_free(imgReconstruct);
  free(newgray);
  free(freq_repr);
  free(gray);
  pnm_free(source);

  fprintf(stderr, "OK\n");
}

/**
 * @brief test construction of magnitude and phase images in ppm files
 * @param char* name, the input image file name
 */
void test_display(char *name)
{
  fprintf(stderr, "test_display: ");
  
  pnm source = pnm_load(name);
  
  int rows, cols;
  unsigned short *gray = pnmTOGray(source, &rows, &cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  //Test
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);

  pnm imgAs = pnm_new(cols, rows, PnmRawPpm);
  pnm imgPs = pnm_new(cols, rows, PnmRawPpm);

  centerImageFloatAndPnm(as, imgAs, rows, cols);
  centerImageFloatAndPnm(ps, imgPs, rows, cols);
  save_image(imgAs, "AS-", name);
  save_image(imgPs, "PS-", name);
  
  //free
  pnm_free(imgPs);
  pnm_free(imgAs);
  free(freq_repr);
  free(gray);
  pnm_free(source);

  fprintf(stderr, "OK\n");
}

/**
 * @brief test the modification of magnitude by adding a periodic functions
          on both vertical and horizontal axis, and 
 *        construct output images
 * @param char* name, the input image file name
 */
void test_add_frequencies(char *name)
{
  fprintf(stderr, "test_add_frequencies: ");
  (void)name;
  fprintf(stderr, "OK\n");
}

void run(char *name)
{
  //test_forward_backward(name);
  //test_reconstruction(name);
  test_display(name);
  test_add_frequencies(name);
}

void usage(const char *s)
{
  fprintf(stderr, "Usage: %s <ims> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 1
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);
  run(argv[1]);
  return EXIT_SUCCESS;
}
