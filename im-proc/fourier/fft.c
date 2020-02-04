#include <float.h>
#include <stdlib.h>
#include <math.h>

#include <fft.h>

void execFFT(int rows, int cols, fftw_complex* grayImage, fftw_complex* spectrum, int mode)
{
  fftw_plan plan = fftw_plan_dft_2d(rows, cols, grayImage, spectrum, mode, FFTW_ESTIMATE);
  fftw_execute(plan);
	fftw_destroy_plan(plan);
}

fftw_complex *forward(int rows, int cols, unsigned short* g_img)
{
  fftw_complex grayImage[rows*cols];
  for (int i =0; i<(rows*cols) ; i++)
  {
      grayImage[i] = (double)g_img[i];
  }
  int size = sizeof(fftw_complex)*rows*cols;
  fftw_complex *spectrum = (fftw_complex *)malloc(size);
  execFFT(rows, cols, grayImage, spectrum, FFTW_FORWARD);
  return spectrum;
}

unsigned short *backward(int rows, int cols, fftw_complex* freq_repr)
{
  fftw_complex grayImage[rows*cols];
  execFFT(rows, cols, freq_repr, grayImage, FFTW_BACKWARD);
  unsigned short *grayReal = malloc(sizeof(unsigned short)*rows*cols);
  for (int i = 0; i<(rows*cols); i++)
  {
      grayReal[i] = creal(grayImage[i])/(rows*cols);
  }
  return grayReal;
}

void freq2spectra(int rows, int cols, fftw_complex* freq_repr, float* as, float* ps) 
{
	for(int i=0; i<rows*cols; i++)
  {
		as[i]= cabsf(freq_repr[i]);
		ps[i]= cargf(freq_repr[i]);
  }
}

void spectra2freq(int rows, int cols, float* as, float* ps, fftw_complex* freq_repr)
{
 for(int i=0; i<rows*cols; i++)
  {
		float r = as[i];
		float phi = ps[i];
    freq_repr[i] = r*cosf(phi) + I * r * sinf(phi);
  }
}
