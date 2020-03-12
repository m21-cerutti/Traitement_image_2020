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

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include <bcl.h>

#define D 3
#define NB_CHANNELS 3
//Must be squared
#define NB_JITTERED_SAMPLE 196
#define NB_NEIGHBOR 25

#define DEFAULT_SAMPLE_SIZE_SQUARED 5

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

typedef struct s_Pixel
{
  double data[NB_CHANNELS];
} Pixel;

typedef struct s_Pixel_Stats
{
  double data[2];
} Pixel_Stats;

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
      //Ignore border
      if(i < 0  ||  i > rows || j <0 || j > cols)
      {
        continue;
      }

      int index = positionToIndex(i, j, cols);
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

double rand_gen() {
   // return a uniformly distributed random value
   return ( (double)(rand()) + 1. )/( (double)(RAND_MAX) + 1. );
}
double normalRandom() {
   // return a normally distributed random value
   double v1=rand_gen();
   double v2=rand_gen();
   return cos(2*3.14*v2)*sqrt(-2.*log(v1));
}

void jitteredSelect(int *jitteredGrid, int rows, int cols )
{
  int offsetY = rows / sqrt(NB_JITTERED_SAMPLE);
  int offsetX = cols / sqrt(NB_JITTERED_SAMPLE);

  //printf("Y %d\n", offsetY);
  //printf("X %d\n", offsetX);

  int p = 0;
  for (int i = offsetY; i < rows; i+=offsetY)
  {
    for (int j = offsetX; j < cols; j+=offsetX)
    {
      //Random distribution
      //int newi = i + (int)((rand_gen()-0.5)*offsetY/2);
      //int newj = j + (int)((rand_gen()-0.5)*offsetX/2);

      //Gaussian
      int newi = i + (int)((normalRandom()-0.5)*offsetY/2);
      int newj = j + (int)((normalRandom()-0.5)*offsetX/2);

      //printf("%d %d\n", i, j);

      int index = positionToIndex(newi, newj, cols);
      //printf("%d\n", index);
      jitteredGrid[p] = index;
      //printf("%d\n", p);
      p++;
    }
  }
  //printf("p %d\n", p);
}
/*
void bestMatch(Pixel imtPixel, Pixel* neighbor, Pixel *jitteredGrid, Pixel match)
{
  for (int i = 0; i < NB_JITTERED_SAMPLE; i++)
  {

  }
}
*/

void transfer(Pixel imtPixel, Pixel bestMatch, Pixel *imdLAB, int index)
{
  imdLAB[index].data[0] = imtPixel.data[0];
  imdLAB[index].data[1] = bestMatch.data[1];
  imdLAB[index].data[2] = bestMatch.data[2];
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
  Pixel* imtLAB = (Pixel*)malloc(sizeof(Pixel)* imtRows * imtCols);
  rgbToLab(imtColors, imtLAB, imtRows, imtCols);

  //Jittered grid
  int* jitteredGrid = (int*)malloc(sizeof(int)* NB_JITTERED_SAMPLE);
  jitteredSelect(jitteredGrid, imsRows, imsCols);

  //Stats jittered
  double means[NB_CHANNELS], vars[NB_CHANNELS];
  //Pixel_Stats *jitteredStats = (Pixel_Stats*)malloc(sizeof(Pixel_Stats)* NB_JITTERED_SAMPLE);
  for (int p = 0; p < NB_JITTERED_SAMPLE; p++)
  {
    getNeighboursPixelStats(jitteredGrid[p], DEFAULT_SAMPLE_SIZE_SQUARED, imsLAB, imsRows, imsCols, means, vars);
    //jitteredStats[p].data[0] = means[0];
    //jitteredStats[p].data[1] = var[0];
  }

  //Stats grey
  //Pixel_Stats *imtStats = (Pixel_Stats*)malloc(sizeof(Pixel_Stats)* imtRows * imtCols);
  for (int index = 0; index < (imtRows * imtCols); index++)
  {
    //getNeighboursPixelStats(index, DEFAULT_SAMPLE_SIZE_SQUARED, imtLAB, imtRows, imtCols, means, var);
    //imtStats[index].data[0] = means[0];
    //imtStats[index].data[1] = var[0];
  }

  //Best match and copy
  Pixel* imdLAB = (Pixel*)malloc(sizeof(Pixel)* imtRows * imtCols);
  Pixel* imdColors = (Pixel*)malloc(sizeof(Pixel)* imtRows * imtCols);
  for (int i = 0; i < imtRows * imtCols; i++) {
    //int indexMatchJittered = bestMatch(imtStats[i], jitteredStats, match);
    //Pixel pix = imsLAB[jitteredGrid[indexMatchJittered]];
    //transfer(imsLAB[indexMatch], imtLAB[indexMatch], imdLAB[i], indexMatch);
  }

  //DEBUG JITTERED
  Pixel* imsJitteredDebug = (Pixel*)malloc(sizeof(Pixel)* imsRows * imsCols);
  for (int p = 0; p < NB_JITTERED_SAMPLE; p++)
  {
    int index = jitteredGrid[p];
    for (int k = 0; k < NB_CHANNELS; k++)
    {
      imsJitteredDebug[index].data[k] =  255;
    }
  }
  pnm debug = pnm_new(imsCols, imsRows, PnmRawPpm);
  imageArrayToPnm(imsJitteredDebug, debug, imsRows, imsCols);
  save_image(debug, "", "debug.ppm");

  //Reconvert
  labToRGB(imdLAB, imtColors, imtRows, imtCols);
  pnm output = pnm_new(imtCols, imtRows, PnmRawPpm);
  imageArrayToPnm(imdColors, output, imtRows, imtCols);
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
