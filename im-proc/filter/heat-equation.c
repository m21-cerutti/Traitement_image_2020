#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>

int
heat_equation()
{
  return 0;
}

void
process(int n, char *ims, char *imd)
{
  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      int res = pnm_get_component(input, i, j, 0);
      for (int k = 0; k < n; k++) {
        //heat_equation();
      }

      for (int channel = 0; channel < 3; channel++)
        pnm_set_component(output, i, j, channel, res);
    }
  }

  pnm_save(output, PnmRawPpm, imd);
}

void
usage (char *s){
  fprintf(stderr, "Usage: %s <n> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  int n = atoi(argv[1]);
  process(n, argv[2], argv[3]);
  return EXIT_SUCCESS;
}
