#include <stdlib.h>
#include <stdio.h>

#include <morphology.h>
#include <se.h>

void
usage(char* s)
{
  fprintf(stderr,"%s <se> <hs> <imd>\n",s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int
main(int argc, char* argv[])
{
  if(argc != PARAM+1) usage(argv[0]);
  (void)argc;
  (void)argv;

  int shape = atoi(argv[1]);
  int hs = atoi(argv[2]);

  pnm output = se(shape, hs);
  pnm_save(output, PnmRawPpm, argv[3]);
  pnm_free(output);

  return EXIT_SUCCESS;
}
