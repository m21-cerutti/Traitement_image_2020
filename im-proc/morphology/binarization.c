#include <stdlib.h>
#include <stdio.h>

#include <pnm.h>

void
process(int min, int max, char* ims, char* imd)
{
  pnm input = pnm_load(ims);

  int inputRows = pnm_get_height(input);
  int inputCols = pnm_get_width(input);

  pnm output = pnm_new(inputCols, inputRows, PnmRawPpm);

  for (int i = 0; i < inputRows; i++) {
    for (int j = 0; j < inputCols; j++) {

      unsigned short val = pnm_get_component(input, i, j, 0);
      val = (val >= min && val <= max) ? 255 : 0;
      for (int channel = 0; channel < 3; channel++) {
        pnm_set_component(output, i, j, channel, val);
      }
    }
  }

  pnm_save(output, PnmRawPpm, imd);

  pnm_free(input);
  pnm_free(output);

}

void
usage(char* s)
{
  fprintf(stderr,"%s <min> <max> <ims> <imd>\n",s);
  exit(EXIT_FAILURE);
}

#define PARAM 4
int
main(int argc, char* argv[])
{
  if(argc != PARAM+1) usage(argv[0]);

  int min = atoi(argv[1]);
  int max = atoi(argv[2]);

  process(min, max, argv[3], argv[4]);

  return EXIT_SUCCESS;
}
