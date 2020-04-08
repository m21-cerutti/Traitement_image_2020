#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <bcl.h>
#include <fft.h>

typedef float (*filter_func)(int, int, int, int, int, int, int);

#define dist(u,v) sqrt((u*u) + (v*v))

//////////////////////////////////////
// Utilities fonctions

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

float
lowpass(int u, int v, int d0, int n, int w, int u0, int v0){
  (void)u0;
  (void)v0;
  (void)w;
  double d = dist(u,v);
  return 1/pow((d/d0),2*n);
}

float
highpass(int u, int v, int d0, int n, int w, int u0, int v0){
  (void)u0;
  (void)v0;
  (void)w;
  double d = dist(u,v);
  return 1/pow((d0/d),2*n);
}

float
bandreject(int u, int v, int d0, int n, int w, int u0, int v0){
  (void)u0;
  (void)v0;
  double d = dist(u,v);
  double tmp = (d*w)/(d*d-d0*d0);
  return 1/(1+pow(tmp,2*n));
}

float
bandpass(int u, int v, int d0, int n, int w, int u0, int v0){
  return 1 - bandreject(u, v, d0, n, w, u0, v0);
}

float
notch(int u, int v, int d0, int n, int w, int u0, int v0){
  (void)w;
  float d1 = dist((u-u0),(v-v0));
  float d2 = dist((u+u0),(v+v0));
  float tmp = (d0*d0)/(d1*d2);
  return 1/(sqrt(pow(tmp,2*n)));
}

filter_func filterSelect(char* filter)
{
    if(strcmp(filter,"lp"))
      return &lowpass;
    if(strcmp(filter,"hp"))
      return &highpass;
    if(strcmp(filter,"br"))
      return &bandreject;
    if(strcmp(filter,"bp"))
      return &bandpass;
    if(strcmp(filter,"no"))
      return &notch;
    fprintf(stderr, "Wrong filter name\n");
    return NULL;
}

void
process(char* in, char* out,
	int d0, int nx2, int ww, int u0, int v0,
	float (*apply)(int, int, int, int, int, int, int))
{
  (void)d0;
  (void)nx2;
  (void)ww;
  (void)u0;
  (void)v0;
  (void)apply;
  pnm input = pnm_load(in);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  unsigned short *complexData = pnm_get_channel(input, NULL, 0);
  fftw_complex * freq_repr = forward(imsRows, imsCols, complexData);
  for (int u = 0; u < imsRows; u++) {
    for (int v = 0; v < imsCols; v++) {
      int index = positionToIndex(u,v,imsRows);
      (void)index;
      //freq_repr[index] = apply(u, v, d0, nx2, ww, u0, v0);
    }
  }
  unsigned short *TcomplexData = backward(imsRows, imsCols, freq_repr);

  for (int channel = 0; channel < 3; channel++) {
    pnm_set_channel(output, TcomplexData, channel);
  }

  pnm_save(output, PnmRawPpm, out);
}

void usage (char *s){
  fprintf(stderr, "Usage: %s <ims> <imd> <filter> ", s);
  fprintf(stderr, "<d0> <n> <w> <u0> <v0>\n");
  exit(EXIT_FAILURE);
}

#define PARAM 8
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);

  int d0 = atoi(argv[3]);
  int n = atoi(argv[4]);
  int w = atoi(argv[5]);
  int u0 = atoi(argv[6]);
  int v0 = atoi(argv[7]);
  filter_func filter = filterSelect(argv[8]);

  process(argv[1], argv[2], d0, n, w, u0, v0,filter);
  return EXIT_SUCCESS;
}
