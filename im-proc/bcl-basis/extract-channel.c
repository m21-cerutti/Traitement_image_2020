#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>


void process(int channel, pnm source, char* output)
{
  size_t rows = pnm_get_height(source);
  size_t cols = pnm_get_width(source);

  pnm imd = pnm_new(cols, rows, PnmRawPpm);
  
  for (size_t i = 0; i < rows; i++)
  {
    for (size_t j = 0; j < cols; j++)
    {
        const unsigned short val = pnm_get_component(source, i, j, channel);
        pnm_set_component(imd, i, j, channel, val);
    }
  }
  pnm_save(imd, PnmRawPpm, output);
}

void usage (char *s, char *param)
{
  fprintf(stderr,"Usage: %s %s", s, "<channel> <ims> <imd>\n");
  fprintf(stderr,"%s to watch out, channel > 0 \n", param);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[])
{
  if (argc != PARAM+1) usage(argv[0], "Parameters");
  int channel = atoi(argv[1]);
  pnm source = pnm_load(argv[2]);
  char* dest_image = argv[3];

  if(channel < 0)
  {
    usage(argv[0], "Channel");
  }

  process(channel, source, dest_image);
  return EXIT_SUCCESS;
}
