/**
 * @file test-fft.c
 * @brief test the behaviors of functions in fft module
 *
 */

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

#define VISU 0.2
#define FOLDER_TEST "res/"

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int rows)
{
  *i = index % rows;
  *j = index / rows;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

unsigned short *pnmToGray(pnm source, int *rows, int *cols)
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

void getMinMax(float *source, int rows, int cols, float* min, float* max)
{
  float _max = FLT_MIN;
  float _min = FLT_MAX;

  for (int index = 0; index < (rows * cols); index++)
  {
    if(source[index] > _max)
    {
      _max = source[index] ;
    }
    if(source[index] < _min)
    {
      _min = source[index] ;
    }
  }
  *max = _max;
  *min = _min;
}

void normalizeFloat(float *source, float* dest, int rows, int cols, float newMin, float newMax)
{
  float tmp[rows*cols];

  float max, min;
  getMinMax(source, rows, cols, &min, &max);

  for (int index = 0; index < (rows * cols); index++)
  {
    float norm = (newMax - newMin)/(max - min) * source[index] + (newMin * max - newMax * min) / (max-min);
    tmp[index] = norm;
  }

  for (int index = 0; index < (rows * cols); index++)
  {
    dest[index] = tmp[index];
  }
}

void floatToPnm(float *source, pnm dest, int rows, int cols)
{
  normalizeFloat(source, source, rows, cols, 0.f, 1.f);

  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, rows);
    unsigned short gray = source[index] *254;
    
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, i, j, k, gray);
    }
  }
}

void centerImageFloat(float *source, float* dest, int rows, int cols)
{
  int middleRow = (rows*1.0)/2;
  int middleCol = (cols*1.0)/2;
 
  float tmp[rows*cols];
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, rows);
    int newIndex = positionToIndex((i + middleRow)%rows, (j + middleCol) %cols, rows);
    tmp[newIndex] = source[index];
  }

  for (int index = 0; index < (rows * cols); index++)
  {
    dest[index] = tmp[index];
  }
}

void powerAFloat(float *source, float* dest, int rows, int cols, float k)
{
  float max, min;
  getMinMax(source, rows, cols, &min, &max);

  float tmp[rows*cols];
  for (int index = 0; index < (rows * cols); index++)
  {
    float gray = pow((source[index] / max), k);
    tmp[index] = gray;
  }

  for (int index = 0; index < (rows * cols); index++)
  {
    dest[index] = tmp[index];
  }
}

void save_image(pnm img, char *prefix, char *name)
{
  //printf("%s\n", name);
  int output_size = strlen(prefix) +strlen(FOLDER_TEST) + strlen(name);
  char output[output_size];

  char *bname, *dname;
  dname = dir_name(name);
  bname = base_name(name);
  sprintf(output, "%s/%s%s%s", dname, FOLDER_TEST, prefix, bname);
  pnm_save(img, PnmRawPpm, output);
  //printf("%s saved.\n",output);
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
  unsigned short *gray = pnmToGray(source, &rows, &cols);

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
  unsigned short *gray = pnmToGray(source, &rows, &cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  //Test
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);
  spectra2freq(rows, cols, as, ps, freq_repr);
  
  unsigned short *newgray = backward(rows, cols, freq_repr);

  pnm imgReconstruct = pnm_new(cols, rows, PnmRawPpm);
  grayToPnm(newgray, imgReconstruct, rows, cols);

  save_image(imgReconstruct, "FB-ASPS-", name);

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
  unsigned short *gray = pnmToGray(source, &rows, &cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  //Test
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);

  pnm imgAs = pnm_new(cols, rows, PnmRawPpm);
  centerImageFloat(as, as, rows, cols);
  powerAFloat(as, as, rows, cols, VISU);
  floatToPnm(as, imgAs, rows, cols);
  save_image(imgAs, "AS-", name);

  pnm imgPs = pnm_new(cols, rows, PnmRawPpm);
  centerImageFloat(ps, ps, rows, cols);
  powerAFloat(ps, ps, rows, cols, VISU);
  floatToPnm(ps, imgPs, rows, cols);
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

   pnm source = pnm_load(name);
  int rows, cols;
  unsigned short *gray = pnmToGray(source, &rows, &cols);

  fftw_complex *freq_repr = forward(rows, cols, gray);

  //Test
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);

  // Modif
  float min, max;
  getMinMax(as, rows, cols, &min, &max);
  centerImageFloat(as, as, rows, cols);
  for (int i = -8; i< 9; i+=16)
  {
    int index =positionToIndex(i + rows/2, cols/2, rows);
    as[index] =  0.25 * max;
    
  }
  for (int j = -8; j< 9; j+=16)
  {
    int index =positionToIndex(rows/2, j + cols/2, rows);
    as[index] =  0.25 * max;
  }
  centerImageFloat(as, as, rows, cols);

  //AS save
  float asVisu[rows * cols];
  centerImageFloat(as, asVisu, rows, cols);
  powerAFloat(asVisu, asVisu, rows, cols, VISU);

  pnm imgAs = pnm_new(cols, rows, PnmRawPpm);
  floatToPnm(asVisu, imgAs, rows, cols);
  save_image(imgAs, "FAS-", name);
  //Free pnm As
  free(imgAs);

  //Reconstruct
  spectra2freq(rows, cols, as, ps, freq_repr);
  unsigned short *newgray = backward(rows, cols, freq_repr);
  pnm imgReconstruct = pnm_new(cols, rows, PnmRawPpm);
  grayToPnm(newgray, imgReconstruct, rows, cols);
  save_image(imgReconstruct, "FREQ-", name);

  //free
  pnm_free(imgReconstruct);
  free(newgray);
  free(freq_repr);
  free(gray);
  pnm_free(source);

  fprintf(stderr, "OK\n");
}

void run(char *name)
{
  test_forward_backward(name);
  test_reconstruction(name);
  test_display(name);
  test_add_frequencies(name);
fftw_cleanup();
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
