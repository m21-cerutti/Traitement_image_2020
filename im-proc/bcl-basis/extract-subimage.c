#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>


void process(const size_t i0, const size_t j0, const size_t rows, const size_t cols, pnm source, char* output)
{
  pnm imd = pnm_new(cols, rows, PnmRawPpm);
  
  for (size_t i = 0; i < rows; i++)
  {
    for (size_t j = 0; j < cols; j++)
    {
       for (size_t k = 0; k < 3; k++)
      {
        const unsigned short val = pnm_get_component(source, i0 + i, j0 + j, k);
        pnm_set_component(imd, i, j, k, val);
      }
    }
  }
  pnm_save(imd, PnmRawPpm, output);
}

void usage (char *s, char *param)
{
  fprintf(stderr,"Usage: %s %s", s, "<i> <j> <rows> <cols> <ims> <imd>\n");
  fprintf(stderr,"%s to watch out, must be i>0 && j>0 && i + rows < height(ims) && j + cols < width(ims)\n", param);
  exit(EXIT_FAILURE);
}

#define PARAM 6
int main(int argc, char *argv[])
{
  if (argc != PARAM+1) usage(argv[0], "Parameters");
  int i0 = atoi(argv[1]);
  int j0 = atoi(argv[2]);
  int rows = atoi(argv[3]);
  int cols = atoi(argv[4]);
  pnm source = pnm_load(argv[5]);
  char* dest_image = argv[6];

  if(i0 < 0)
  {
    usage(argv[0], "i0");
  }

  if(j0 < 0)
  {
    usage(argv[0], "j0");
  }

  if(i0 + rows > pnm_get_height(source))
  {
    usage(argv[0], "rows");
  }

  if(j0 + cols > pnm_get_width(source))
  {
    usage(argv[0], "cols");
  }

  process(i0, j0, rows, cols, source, dest_image);
  return EXIT_SUCCESS;
}
