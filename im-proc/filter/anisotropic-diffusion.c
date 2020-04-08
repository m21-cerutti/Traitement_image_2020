#include <stdlib.h>
#include <stdio.h>

#include <bcl.h>


void
process(int n, int lambda, char *ims, char *imd)
{
  (void) n;
  (void) lambda;

  pnm input = pnm_load(ims);

  int imsRows = pnm_get_height(input);
  int imsCols = pnm_get_width(input);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      int res = pnm_get_component(output, i, j, 0);

      //anistropic-diffusion

      for (int channel = 0; channel < 3; channel++)
        pnm_set_component(output, i, j, channel, res);
    }
  }

  pnm_save(output, PnmRawPpm, imd);
}

void
usage (char *s){
  fprintf(stderr, "Usage: %s <n> <lambda> <function> <ims> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 5
int
main(int argc, char *argv[]){
  if (argc != PARAM+1) usage(argv[0]);
  int n = atoi(argv[1]);
  int lambda = atoi(argv[2]);
  process(n, lambda, argv[3], argv[4]);
  return EXIT_SUCCESS;
}
