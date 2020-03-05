/**
 * @file color-transfert
 * @brief transfert color from source image to target image.
 *        Method from Reinhard et al. :
 *        Erik Reinhard, Michael Ashikhmin, Bruce Gooch and Peter Shirley,
 *        'Color Transfer between Images', IEEE CGA special issue on
 *        Applied Perception, Vol 21, No 5, pp 34-41, September - October 2001
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <math.h>

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

/* CHANGE TO COLS IN CODE
int positionToIndex(int i, int j, const int cols)
{
  return i * cols + j;
}
*/

void save_image(pnm img, char *prefix, char *name)
{
  int output_size = strlen(prefix) +strlen(name);
  char output[output_size];

  char *bname, *dname;
  dname = dir_name(name);
  bname = base_name(name);
  sprintf(output, "%s/%s%s", dname, prefix, bname);
  pnm_save(img, PnmRawPpm, output);
}

//////////////////////

#define NB_CHANNELS 3

typedef struct s_Pixel
{
  double data[NB_CHANNELS];
} Pixel;

void pnmToImageArray(pnm source, Pixel* dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    for (int k = 0; k < 3; k++)
    {
      int i, j;
      indexToPosition(index, &i, &j, rows);
      unsigned short val = pnm_get_component(source, i, j, k);
      dest[index].data[k] = val;
    }
  }
}

void imageArrayToPnm(Pixel* source, pnm dest, int rows, int cols)
{
  for (int index = 0; index < (rows * cols); index++)
  {
    for (int k = 0; k < 3; k++)
    {
      int i, j;
      indexToPosition(index, &i, &j, rows);
      unsigned short val = fmax(0., fmin(255., source[index].data[k]));
      pnm_set_component(dest, i, j, k, val);
    }
  }
}

void getImageStats(Pixel* source, int rows, int cols, double means[NB_CHANNELS], double var[NB_CHANNELS])
{
  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, rows);
    for (int k = 0; k < NB_CHANNELS; k++)
    {
      double val = source[index].data[k];
      means[k] += val;
      var[k] += val * val;
    }
  }

  for (int k = 0; k < NB_CHANNELS; k++)
  {
    means[k] /= rows * cols;
    var[k] = (var[k] / (rows * cols)) - means[k] * means[k];
  }
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

//////////////////////

void process(char *ims, char *imt, char* imd){
  pnm input = pnm_load(ims);

  int cols = pnm_get_width(input);
  int rows = pnm_get_height(input);

  Pixel* colors = (Pixel*)malloc(sizeof(Pixel) * rows * cols);
  pnmToImageArray(input, colors, rows, cols);
  Pixel* lms= (Pixel*)malloc(sizeof(Pixel)* rows * cols);

  Pixel* test= (Pixel*)malloc(sizeof(Pixel) * rows * cols);

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int index = positionToIndex(i, j, rows);
      matrixProduct(RGB2LMS, D, D, colors[index].data, D, 1, lms[index].data);
      //printf("\nLMS\n");
      //printMatrix(lms[index].data, 3, 1);
      matrixProduct(LMS2RGB, D, D, lms[index].data, D, 1, test[index].data);
      //printf("\nTEST\n");
      //printMatrix(test[index].data, 3, 1);
    }
  }

  //Stats
  double means[NB_CHANNELS]; 
  double var[NB_CHANNELS];
  getImageStats(lms, rows, cols, means, var);

  printf("\nLMS\n");
  for (int k = 0; k < 3; k++)
  {
    printf("Mean channel %d : %lf\n", k ,means[k]);
    printf("Variance channel %d : %lf\n", k , var[k]);
    printf("Deviation channel %d : %lf\n", k , sqrt(var[k]));
  }

  getImageStats(colors, rows, cols, means, var);

  printf("\nColors\n");
  for (int k = 0; k < 3; k++)
  {
    printf("Mean channel %d : %lf\n", k ,means[k]);
    printf("Variance channel %d : %lf\n", k , var[k]);
    printf("Deviation channel %d : %lf\n", k , sqrt(var[k]));
  }

  getImageStats(test, rows, cols, means, var);

  printf("\nTest\n");
  for (int k = 0; k < 3; k++)
  {
    printf("Mean channel %d : %lf\n", k ,means[k]);
    printf("Variance channel %d : %lf\n", k , var[k]);
    printf("Deviation channel %d : %lf\n", k , sqrt(var[k]));
  }

  (void)imt;
  pnm output = pnm_new(cols, rows, PnmRawPpm);
  imageArrayToPnm(test, output, rows, cols);
  save_image(output, "", imd);
  free(colors);
  free(lms);
  free(test);
  pnm_free(input);
  pnm_free(output);
}

void usage (char *s){
  fprintf(stderr, "Usage: %s <ims> <imt> <imd> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  process(argv[1], argv[2], argv[3]);
  return EXIT_SUCCESS;
}
