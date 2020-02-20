#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <fft.h>

#include <math.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#define FOLDER_TEST ""

//////////////////////////////////////
// Utilities fonctions

void indexToPosition(int index, int *i, int *j, const int rows)
{
  *i = index % rows;
  *j = index / rows;
}

int positionToIndex(int i, int j, const int rows)
{
  return i * rows + j;
}

void getSizePnm(pnm source, int *rows, int *cols)
{
  *cols = pnm_get_width(source);
  *rows = pnm_get_height(source);
}

void save_image(pnm img, char *prefix, char *name)
{
  int output_size = strlen(prefix) +strlen(FOLDER_TEST) + strlen(name);
  char output[output_size];

  char *bname, *dname;
  dname = dir_name(name);
  bname = base_name(name);
  sprintf(output, "%s/%s%s%s", dname, FOLDER_TEST, prefix, bname);
  pnm_save(img, PnmRawPpm, output);
}



//////////////////////////////////////
// Filter

typedef double (*filter_func)(int x); 

double box (int x)
{
    if (x >= -0.5 && x < 0.5)
      return 1.0;
    return 0.0;
}

double tent (int x)
{
    if (x >= -1 && x <= 1)
      return 1.0 - abs(x);
    return 0;
}

double bell (int x)
{
    double absX = abs(x);
    if (absX <= 0.5)
      return 0.75 - pow(x,2);
    if (0.5 < absX && absX < 1.5)
      return 0.5 * pow((absX - 0.5),2);
    return 0;
}

double mitchellNetravali (int x)
{
  if (-1 < x && x < 1)
    return 7./6. * pow(abs(x),3.) - 2. * pow(x,2.) + 8./9.;
  if ((-2 < x && x < -1) || (1 < x && x < 2))
    return -7./18. * pow(abs(x),3.) + 2. * pow(x,2.) - 10./3. * abs(x) + 16./9.;
  return 0;
}

double double_get_component(double* source, int rows, int i, int j, int channel)
{
  return source[i * (rows + channel) + j + channel];
}

void double_set_component(double* source, int rows, int i, int j, int channel, double value)
{
  source[i * (rows + channel) + j + channel] = value;
}

void rotate(int rows, int cols, double* in, double* out) 
{
  for (int i = 0; i < (rows); i++)
  {
    for (int j = 0; j < (cols); j++)
    {
      for (int channel = 0; channel < 3; channel++)
      {
        unsigned short val = double_get_component(in, rows, i, j, channel);
        double_set_component(out, rows, j, i, channel, val);
      }
    }
  }
}

void pnmToDouble(pnm source, double *out, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      for (int channel = 0; channel < 3; channel++)
      {
        unsigned short val = pnm_get_component(source, i, j, channel);
        double_set_component(out, rows, i, j, channel, val);
      }
    }
  }
}

void doubleToPnm(double* source, pnm out, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      for (int channel = 0; channel< 3; channel++)
      {
        double val = double_get_component(source, rows, i, j, channel);
        pnm_set_component(out, i, j, channel, val);
      }
    }
  }
}

//////////////////////////////////////
// Convolution

void convolution1D(int factor, int rows, int cols, filter_func filter, int domain[2], double* in, double* out) 
{
  for (int i = 0; i < (rows); i++)
  {
    for (int jo = 0; jo < (factor * cols); jo++)
    {
      int ji = jo / (factor*1.0);

      // 3 Channels
      double pixel[3] ={0., 0., 0.};
      for (int k = 0; k < 3; k++)
      {
        // Convolution
        for (int jc = (ji + domain[0]); jc <= (ji +domain[1]); jc++)
        {
          //Border ignored (padding zero)
          if(jc >= 0 && jc < cols)
          {
            double val = double_get_component(in, rows, i, jc, k);
            pixel[k] += (double)val * filter(jc - ji);
          }
        }
        // Out
        for (int k = 0; k < 3; k++)
        {
          double_set_component(out, rows, i, jo, k, pixel[k]);
        }
      }
    }
  }
}

//////////////////////////////////////
// Main

void run(int factor, char* filterName, char* ims, char* imd) 
{
  (void)(filterName);
	pnm input = pnm_load(ims);

	int cols, rows;
  getSizePnm(input, &rows, &cols);

  filter_func f;
  int domain[2];
  if (strcmp(filterName, "box") == 0) 
  {
    f = (filter_func)box;
    domain[0] = -1;
    domain[1] = 1;
  } 
  else if (strcmp(filterName, "tent") == 0)
  {
    f = (filter_func)tent;
    domain[0] = -1;
    domain[1] = 1;
  }
  else if (strcmp(filterName, "bell") == 0)
  {
    f = (filter_func)bell;

    domain[0] = -2;
    domain[1] = 2;
  }
  else if (strcmp(filterName, "mitch") == 0)
  {
    f = (filter_func)mitchellNetravali;

    domain[0] = -3;
    domain[1] = 3;
  }
  else
  {
    fprintf(stderr, "Filter %s not known. Please use box, tent, bell or mitch.\n", filterName);
    exit(1);
  }

  double dinput [cols * rows * 3];
  pnmToDouble(input, dinput, rows, cols);

  double tmp [cols * factor * rows * 3];
  convolution1D(factor, rows, cols, f, domain,  dinput, tmp);

  double r_tmp[cols * rows * factor * 3];
  rotate(rows, cols * factor, tmp, r_tmp);

  double tmp3[factor * cols * factor * rows * 3];
  convolution1D(factor, rows * factor, cols, f, domain, r_tmp, tmp3);

  double doutput[factor * cols * factor * rows * 3];
  rotate(rows * factor, cols * factor, tmp3, doutput);

  pnm output = pnm_new(factor * cols, factor * rows, PnmRawPpm);
  doubleToPnm( doutput, output, rows * factor, cols * factor);
  save_image(output, "", imd);

  pnm_free(input);
  pnm_free(output);
}

void
usage(const char *s)
{
  fprintf(stderr, "Usage: %s <factor> <filter-name> <ims> <imd> \n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 4
int
main(int argc, char *argv[])
{
  if (argc != PARAM+1) usage(argv[0]);
	int factor = atoi(argv[1]);
	if(factor < 1) {
		printf("Factor must be superior to 1\n");
		return EXIT_FAILURE;
	}
  run(factor, argv[2], argv[3], argv[4]);
  return EXIT_SUCCESS;
}
