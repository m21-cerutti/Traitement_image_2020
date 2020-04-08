#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <bcl.h>
#include <fft.h>

typedef float (*filter_func)(int, int, int, int, int, int, int);

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

float dist(int u, int v) 
{
  return sqrtf((u * u) + (v * v));
}

//////////////////////////////////////

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
    indexToPosition(index, &i, &j, *cols);
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

//////////////////////////////////////

void save_image(pnm img, char *prefix, char *name)
{
  //printf("%s\n", name);
  int output_size = strlen(prefix) + strlen(name);
  char output[output_size];

  char *bname, *dname;
  dname = dir_name(name);
  bname = base_name(name);
  sprintf(output, "%s/%s%s", dname, prefix, bname);
  pnm_save(img, PnmRawPpm, output);

  free(dname);
  free(bname);
  //printf("%s saved.\n",output);
}

//////////////////////////////////////

void copyFloatArray(float *source, float *dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    dest[index] = source[index];
  }
}

void centerImageFloat(float *source, float *dest, int rows, int cols)
{
  int middleRow = rows / 2;
  int middleCol = cols / 2;

  float tmp[rows * cols];
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, cols);
    int newIndex = positionToIndex((i + middleRow) % rows, (j + middleCol) % cols, rows);
    tmp[newIndex] = source[index];
  }
  copyFloatArray(tmp, dest, rows, cols);
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
  copyFloatArray(tmp, dest, rows, cols);
}

void floatToPnm(float *source, pnm dest, int rows, int cols)
{
  normalizeFloat(source, source, rows, cols, 0.f, 1.f);

  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, cols);
    unsigned short gray = source[index] *255;
    
    for (int k = 0; k < 3; k++)
    {
      pnm_set_component(dest, i, j, k, gray);
    }
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
  copyFloatArray(tmp, dest, rows, cols);
}

//////////////////////////////////////

float lowpass(int u, int v, int d0, int n, int w, int u0, int v0)
{
  (void)u0;
  (void)v0;
  (void)w;
  double d = dist(u, v);
  return 1. / (1 + pow( (d / d0), 2 * n));
}

float highpass(int u, int v, int d0, int n, int w, int u0, int v0)
{
  (void)u0;
  (void)v0;
  (void)w;
  double d = dist(u, v);
  return 1. / (1 +  pow((d0 / d), 2 * n));
}

float bandreject(int u, int v, int d0, int n, int w, int u0, int v0)
{
  (void)u0;
  (void)v0;
  double d = dist(u, v);
  double tmp = (d * w) / (d * d - d0 * d0);
  return 1. / (1 + pow(tmp, 2 * n));
}

float bandpass(int u, int v, int d0, int n, int w, int u0, int v0)
{
  return 1. - bandreject(u, v, d0, n, w, u0, v0);
}

float notch(int u, int v, int d0, int n, int w, int u0, int v0)
{
  (void)w;
  double d1 = dist((u - u0), (v - v0));
  double d2 = dist((u + u0), (v + v0));
  double tmp = (d0 * d0) / (d1 * d2);
  return 1. / (1 + pow(tmp, 2 * n));
}

filter_func filterSelect(char *filter)
{
  if (strcmp(filter, "lp") == 0)
    return &lowpass;
  else if (strcmp(filter, "hp")== 0)
    return &highpass;
  else if (strcmp(filter, "br")== 0)
    return &bandreject;
  else if (strcmp(filter, "bp")== 0)
    return &bandpass;
  else if (strcmp(filter, "no")== 0)
    return &notch;

  fprintf(stderr, "Wrong filter name\n");
  exit(EXIT_FAILURE);
}
//////////////////////////////////////

void process(char *in, char *out,
             float (*apply)(int, int, int, int, int, int, int),
             int d0, int nx2, int ww, int u0, int v0)
{
  pnm source = pnm_load(in);

  int rows, cols;
  unsigned short *gray = pnmToGray(source, &rows, &cols);
  fftw_complex *freq_repr = forward(rows, cols, gray);
  float as[rows * cols];
  float ps[rows * cols];
  freq2spectra(rows, cols, freq_repr, as, ps);

  //Modify
  centerImageFloat(as, as, rows, cols);
  int midleRow = rows / 2;
  int midleCols = cols / 2;
  for (int u = -midleRow; u < midleRow; u++)
  {
    for (int v = -midleCols; v < midleCols; v++)
    {
      int index = positionToIndex(midleRow + u, midleRow + v, rows);
      //Strange but inversed
      float filter = apply(v, u, d0, nx2, ww, u0, v0);
      float res = as[index] * filter;
      as[index] = res;
    }
  }
  centerImageFloat(as, as, rows, cols);

  //AS save DEBUG
  float asVisu[rows * cols];
  centerImageFloat(as, asVisu, rows, cols);
  powerAFloat(asVisu, asVisu, rows, cols, 0.2);
  pnm imgAs = pnm_new(cols, rows, PnmRawPpm);
  floatToPnm(asVisu, imgAs, rows, cols);
  save_image(imgAs, "FAS-", out);
  pnm_free(imgAs);

  spectra2freq(rows, cols, as, ps, freq_repr);
  unsigned short *newgray = backward(rows, cols, freq_repr);
  pnm imgReconstruct = pnm_new(cols, rows, PnmRawPpm);
  grayToPnm(newgray, imgReconstruct, rows, cols);

  save_image(imgReconstruct, "", out);

  //free
  pnm_free(imgReconstruct);
  free(newgray);
  free(freq_repr);
  free(gray);
  pnm_free(source);
}

void usage(char *s)
{
  fprintf(stderr, "Usage: %s <ims> <imd> <filter> ", s);
  fprintf(stderr, "<d0> <n> <w> <u0> <v0>\n");
  exit(EXIT_FAILURE);
}

#define PARAM 8
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);

  filter_func filter = filterSelect(argv[3]);
  int d0 = atoi(argv[4]);
  int n = atoi(argv[5]);
  int w = atoi(argv[6]);
  int u0 = atoi(argv[7]);
  int v0 = atoi(argv[8]);

  process(argv[1], argv[2], filter, d0, n, w, u0, v0);
  return EXIT_SUCCESS;
}
