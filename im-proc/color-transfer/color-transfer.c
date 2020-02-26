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

double RGB2LMS[3*3] = {
  0.3811, 0.5783, 0.0402,
  0.1967, 0.7244, 0.0782,
  0.0241, 0.1288, 0.8444
};

double LMS2RGB[3*3] = {
  4.4679, -3.5873, 0.1193,
  -1.2186, 2.3809, -0.1624,
  0.0497, -0.2439, 1.2045
};

double LAB2LMS[3*3] = {
  sqrt(3)/3, sqrt(6)/6, sqrt(2)/2,
  sqrt(3)/3, sqrt(6)/6, -sqrt(2)/2,
  sqrt(3)/3, -sqrt(6)/3, 0
};

double LMS2LAB[3*3] = {
  1/sqrt(3), 1/sqrt(3), 1/sqrt(3),
  1/sqrt(6), 1/sqrt(6), -1/sqrt(6),
  1/sqrt(2), -sqrt(2), 0
};

//////////////////////
void indexToPosition(int index, int *i, int *j, const int rows)
{
  *i = index % rows;
  *j = index / rows;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

//////////////////////

void matrixProduct(double* M1, int rows1, int cols1, double* M2, int rows2, int cols2, double* out) 
{
  if( cols1 != rows2)
  {
    fprintf(stderr, "Error matrix.");
    exit(1);
  }

  for (int io = 0; io < rows1; io++)
  {
    for (int jo = 0; jo < cols2; jo++)
    {
      int indexo = positionToIndex(io, jo, rows1);
      out[indexo] = 0;
    }
  }

  for (int io = 0; io < rows1; io++)
  {
    for (int jo = 0; jo < cols2; jo++)
    {
      int k1 =0, k2 =0;
      for (; k1 < cols1 && k2 < rows2; k1++, k2++)
      {
        int indexo = positionToIndex(io, jo, rows1);
        int index1 = positionToIndex(io, k1, rows1);
        int index2 = positionToIndex(k2, jo, rows2);

        out[indexo] += M1[index1] * M2[index2];
      }
    }
  }
}

void printMatrix(double* M, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    printf("[ ");
    for (int j = 0; j < cols; j++)
    {
      int index = positionToIndex(i, j, rows);
      printf("\t%lf\t", M[index]);
    }
    printf(" ]\n");
  }
  printf("\n");
}

/*
void process(char *ims, char *imt, char* imd){
  pnm input = pnm_load(ims);

  int cols = pnm_get_width(input);
  int rows = pnm_get_height(input);

  pnm output = pnm_new(cols, rows, PnmRawPpm);

  float res[cols][rows][3];
  unsigned short *color = malloc(sizeof(unsigned short) * rows * cols);

  for (int channel = 0; channel < 3; channel++)
  {
    pnm_get_channel(input, color, channel);
    //matrixProduct(RGB2LMS, color, res);
    //matrixProduct(LMS2RGB, res, color);
    pnm_set_channel(output, color, channel);
  }

  pnm_save(output,PnmRawPpm, imd);
  //(void) ims;
  (void) imt;
  //(void) imd;

}
*/
void usage (char *s){
  fprintf(stderr, "Usage: %s <ims> <imt> <imd> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  //process(argv[1], argv[2], argv[3]);

 /*
  printMatrix(RGB2LMS, D, D);
  printMatrix(LMS2RGB, D, D);
  printMatrix(LAB2LMS, D, D);
  printMatrix(LMS2LAB, D, D);
  

  double M[3] = {
  1., 
  1., 
  1.,   
  };

  double R[3] = {
  0., 
  0., 
  0.,   
  };
  
  matrixProduct(RGB2LMS, D, D, M, D, 1, R);
  */

  printMatrix(R, 3, 1);
  return EXIT_SUCCESS;
}
