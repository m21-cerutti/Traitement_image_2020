#include <float.h>
#include <stdlib.h>
#include <math.h>

#include <fft.h>

#define REAL 0
#define IMAG 1

fftw_complex *
forward(int rows, int cols, unsigned short* g_img)
{
  //Construction de l'image complexe
  fftw_complex *forward = malloc(rows*cols*sizeof(fftw_complex));
  for (int i = 0; i < rows * cols; i++) {
      forward[i] = g_img[i] + I * 0;
    }


  //Allocation de structure donnees complexes de meme taille que img source
  fftw_complex *complexData = malloc(rows*cols*sizeof(fftw_complex));

  //Initialisation et calcul de la transformé
	  fftw_plan plan = fftw_plan_dft_2d(rows,
                                cols,
                                forward,
                                complexData,
                                FFTW_FORWARD,
                                FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);

  //Libération espace mémoire
  free(forward);

  return (complexData);
}

unsigned short *
backward(int rows, int cols, fftw_complex* freq_repr)
{
  //Allocation de structure donnees
  fftw_complex *complexData = malloc(rows*cols*sizeof(fftw_complex));

  //Initialisation et calcul de la transformé
  fftw_plan plan = fftw_plan_dft_2d(rows,
                                cols,
                                freq_repr,
                                complexData,
                                FFTW_BACKWARD,
                                FFTW_ESTIMATE);
  fftw_execute(plan);
  fftw_destroy_plan(plan);

  double N = (double) rows * cols;
	unsigned short* backward = malloc(rows * cols * sizeof(unsigned short));
  for (int j = 0; j < rows * cols; j++) {
      //double val =
      backward[j] = creal(complexData[j]) / N;
  }

  free(complexData);
  return backward;
}

void
freq2spectra(int rows, int cols, fftw_complex* freq_repr, float* as, float* ps)
{
  for (int i = 0; i < rows*cols; i++) {
    fftw_complex c = freq_repr[i];

    double re = creal(c);
    double im = cimag(c);

    as[i] = sqrt(re*re + im*im);
    ps[i] = cimag(c);
  }
}

void
spectra2freq(int rows, int cols, float* as, float* ps, fftw_complex* freq_repr)
{
  for (int i = 0; i < rows*cols; i++) {
    double im = ps[i];
    double re = as[i] * cos(im);

    freq_repr[i] = im + I*re;
  }
}
