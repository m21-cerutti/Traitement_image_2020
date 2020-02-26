/**
 * @file color-transfert
 * @brief transfert color from source image to target image.
 *        Method from Reinhard et al. :
 *        Erik Reinhard, Michael Ashikhmin, Bruce Gooch and Peter Shirley,
 *        'Color Transfer between Images', IEEE CGA special issue on
 *        Applied Perception, Vol 21, No 5, pp 34-41, September - October 2001
 */

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <bcl.h>

#define D 3

float RGB2LMS[D][D] = {
  {0.3811, 0.5783, 0.0402},
  {0.1967, 0.7244, 0.0782},
  {0.0241, 0.1288, 0.8444}
};

float LMS2RGB[D][D] = {
  {4.4679, -3.5873, 0.1193},
  {-1.2186, 2.3809, -0.1624},
  {0.0497, -0.2439, 1.2045}
}

float LAB2LMS[D][D] = {
  {sqrt(3)/3, sqrt(6)/6, sqrt(2)/2},
  {sqrt(3)/3, sqrt(6)/6, -sqrt(2)/2},
  {sqrt(3)/3, -sqrt(6)/3, 0}
}

float LMS2LAB[D][D] = {
  {1/sqrt(3), 1/sqrt(3), 1/sqrt(3)},
  {1/sqrt(6), 1/sqrt(6), -1/sqrt(6)},
  {1/sqrt(2), -sqrt(2), 0}
}

void matrixProduct(float* M1, float* M2, float* res) {
  for (int rows = 0; rows < D; rows++)
  {
    res[rows][cols] = 0;
    for (int cols = 0; cols < D; cols++)
    {
      res[rows][cols] += M1[rows][cols] * M2[rows][cols];
    }
  }
}

void
process(char *ims, char *imt, char* imd){
  pnm input = pnm_load(ims);

  int cols = pnm_get_width(input);
  int rows = pnm_get_height(input);

  pnm output = pnm_new(cols, rows, PnmRawPpm);

  float res[cols][rows][3];
  unsigned short *color = malloc(sizeof(unsigned short) * rows * cols);

  for (int channel = 0; channel < 3; channel++)
  {
    pnm_get_channel(input, color, channel);
    matrixProduct(RGB2LMS, color, res);
    matrixProduct(LMS2RGB, res, color);
    pnm_set_channel(output, color, channel);
  }

  pnm_save(output,PnmRawPpm, imd);
  //(void) ims;
  (void) imt;
  //(void) imd;

}

void
usage (char *s){
  fprintf(stderr, "Usage: %s <ims> <imt> <imd> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  process(argv[1], argv[2], argv[3]);
  return EXIT_SUCCESS;
}
