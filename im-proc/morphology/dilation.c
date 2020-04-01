#include <stdlib.h>
#include <stdio.h>

#include <morphology.h>

void
usage(char* s)
{
  fprintf(stderr,"%s <se> <hs> <ims> <imd>\n",s);
  exit(EXIT_FAILURE);
}

#define PARAM 4
int
main(int argc, char* argv[])
{
  if(argc != PARAM+1) usage(argv[0]);

  int s = atoi(argv[1]);
  int hs = atoi(argv[2]);

  pnm source = pnm_load(argv[3]);
  pnm output = pnm_new(pnm_get_width(source), pnm_get_height(source), PnmRawPpm);

  //process(s, hs, source, output, &maximum); //Resultat et formule contradictoire
  process(s, hs, source, output, &minimum);

  pnm_save(output, PnmRawPpm, argv[4]);

  pnm_free(source);
  pnm_free(output);
  return EXIT_SUCCESS;
}
