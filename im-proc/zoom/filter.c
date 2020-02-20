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
      return 1;
    return 0;
}

double tent (int x)
{
    if (x >= -1 && x <= 1)
      return 1 - abs(x);
    return 0;
}

double bell (int x)
{
    int absX = abs(x);
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

//////////////////////////////////////
// Convolution

// TODO Change pbnm to double[] then copy to pnm. Use function normalize on out
void convolution(int factor, int rows, int cols, filter_func filter, int domain[2], pnm in, pnm out) 
{
  double max[3] ={0, 0, 0};
  for (int io = 0; io < (factor * rows); io++)
  {
    int ii = io/ factor;
    for (int jo = 0; jo < (factor * cols); jo++)
    {
      int ji = jo/ factor;

        // 3 Channels
        double pixel[3] ={0, 0, 0};
        for (int k = 0; k < 3; k++)
        {
          // Convolution
          for (int ic = (ii + domain[0]); ic <= (ii + domain[1]); ic++)
          {
            for (int jc = (ji + domain[0]); jc <= (ji +domain[1]); jc++)
            {
              //Border ignored (padding zero)
              if(jc > 0 && jc < cols && ic > 0 && ic < rows)
              {
                unsigned short val = pnm_get_component(in, ic, jc, k);
                pixel[k] += val * filter(jc - ji);
              }
            }
          }

          // Out
          for (int k = 0; k < 3; k++)
          {
            //TODO Instead trunc, normalize OR copy on double[] and then normalize function
            if(pixel[k] < 0) pixel[k] = 0;
            if(pixel[k] > 255) pixel[k] = 255;
            pnm_set_component(out, io, jo, k, pixel[k]);
          }
        }
    }
}

//////////////////////////////////////
// Main

void run(int factor, char* filterName, char* ims, char* imd) {
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

  pnm output = pnm_new(factor * cols, factor * rows, PnmRawPpm);

  convolution(factor, rows, cols, f, domain,  input, output);

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
