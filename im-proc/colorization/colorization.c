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
  0.5773, 0.4082,  0.7071,
  0.5773, 0.4082, -0.7071,
  0.5773, -0.8165,  0.0000
};

double LMS2LAB[3*3] = {
  0.5773,  0.5773,  0.5773,
  0.4082,  0.4082, -0.8165,
  0.7071, -0.7071,  0.0000
};

//////////////////////

void indexToPosition(int index, int *i, int *j, const int cols)
{
  *i = index / cols;
  *j = index % cols;
}

int positionToIndex(int i, int j, const int cols)
{
  return i * cols + j;
}

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
      indexToPosition(index, &i, &j, cols);
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
      indexToPosition(index, &i, &j, cols);
      unsigned short val = fmax(0., fmin(255., source[index].data[k]));
      pnm_set_component(dest, i, j, k, val);
    }
  }
}

void getImageStats(Pixel* source, int rows, int cols, double means[NB_CHANNELS], double var[NB_CHANNELS])
{
  for (int k = 0; k < NB_CHANNELS; k++)
  {
    means[k] = 0;
    var[k] = 0;
  }

  for (int index = 0; index < (rows * cols); index++)
  {
    int i, j;
    indexToPosition(index, &i, &j, cols);
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

#define DEFAULT_SAMPLE_SIZE_SQUARED 5

void getNeighboursPixelStats(int indexPixel, int sampleSizeSquared, Pixel* source, int rows, int cols, double means[NB_CHANNELS], double var[NB_CHANNELS])
{
  for (int k = 0; k < NB_CHANNELS; k++)
  {
    means[k] = 0;
    var[k] = 0;
  }

  int ip, jp;
  indexToPosition(indexPixel, &ip, &jp, cols);

  for (int i = ip - sampleSizeSquared; i < (ip + sampleSizeSquared); i++)
  {
     for (int j = jp - sampleSizeSquared; i < (jp + sampleSizeSquared); j++)
    {
      int index = positionToIndex(i, j, cols);
      //Ignore border
      if(index < 0  ||  index > (rows * cols))
      {
        continue;
      }

      for (int k = 0; k < NB_CHANNELS; k++)
      {
        double val = source[index].data[k];
        means[k] += val;
        var[k] += val * val;
      }
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
      int indexo = positionToIndex(io, jo, cols2);
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
        int indexo = positionToIndex(io, jo, cols2);
        int index1 = positionToIndex(io, k1, cols1);
        int index2 = positionToIndex(k2, jo, cols2);

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
      int index = positionToIndex(i, j, cols);
      printf("\t%lf\t", M[index]);
    }
    printf(" ]\n");
  }
  printf("\n");
}

void rgbToLab (Pixel* rgb, Pixel* lab, int rows, int cols)
{
  Pixel* lms = (Pixel*)malloc(sizeof(Pixel)* rows * cols);

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int index = positionToIndex(i, j, cols);
      matrixProduct(RGB2LMS, D, D, rgb[index].data, D, 1, lms[index].data);
      matrixProduct(LMS2LAB, D, D, lms[index].data, D, 1, lab[index].data);
    }
  }

  free(lms);
}

void labToRGB(Pixel* lab, Pixel* rgb, int rows, int cols)
{
  Pixel* lms = (Pixel*)malloc(sizeof(Pixel)* rows * cols);

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      int index = positionToIndex(i, j, cols);
      matrixProduct(LAB2LMS, D, D, lab[index].data, D, 1, lms[index].data);
      matrixProduct(LMS2RGB, D, D, lms[index].data, D, 1, rgb[index].data);
    }
  }

  free(lms);
}
//////////////////////

void process(char *ims, char *imt, char* imd){
  pnm imsInput = pnm_load(ims);

  int imsRows = pnm_get_height(imsInput);
  int imsCols = pnm_get_width(imsInput);
  Pixel* imsColors = (Pixel*)malloc(sizeof(Pixel) * imsRows * imsCols);
  pnmToImageArray(imsInput, imsColors, imsRows, imsCols);
  Pixel* imsLAB= (Pixel*)malloc(sizeof(Pixel)* imsRows * imsCols);
  rgbToLab(imsColors, imsLAB, imsRows, imsCols);

  pnm imtInput = pnm_load(imt);

  int imtCols = pnm_get_width(imtInput);
  int imtRows = pnm_get_height(imtInput);
  Pixel* imtColors = (Pixel*)malloc(sizeof(Pixel) * imtRows * imtCols);
  pnmToImageArray(imtInput, imtColors, imtRows, imtCols);
  Pixel* imtLAB= (Pixel*)malloc(sizeof(Pixel)* imtRows * imtCols);
  rgbToLab(imtColors, imtLAB, imtRows, imtCols);

  //TODO
  
  labToRGB(imtLAB, imtColors, imtRows, imtCols);

  pnm output = pnm_new(imtCols, imtRows, PnmRawPpm);
  imageArrayToPnm(imtColors, output, imtRows, imtCols);
  save_image(output, "", imd);

  free(imsColors);
  free(imsLAB);
  free(imtColors);
  free(imtLAB);
  pnm_free(imsInput);
  pnm_free(imtInput);
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
