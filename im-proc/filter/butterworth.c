#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>

#include <bcl.h>
#include <fft.h>

#define dist(u,v) sqrt((u*u) + (v*v))

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

void
process(char* inp, char* out,
	int d0, int nx2, int ww, int u0, int v0,
	float (*apply)(int, int, int, int, int, int, int))
{
    pnm input = pnm_load(inp);

    int imsRows = pnm_get_height(input);
    int imsCols = pnm_get_width(input);

    //fftw_complex *freq_repr = forward(imsRows, imsCols, input);

    for (int i = 0; i < imsRows*imsCols; i++);
      // freq_repr[i] = apply(u, v, d0, n, w, u0, v0);
    (void)d0;
    (void)nx2;
    (void)ww;
    (void)u0;
    (void)v0;
    (void)apply;

    //unsigned short *newgray = backward(imsRows, imsCols, freq_repr);
    //grayToPnm(newgray, output, imsRows, imsCols);
    pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

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
  return EXIT_SUCCESS;
}
