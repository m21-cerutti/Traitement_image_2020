#include <stdlib.h>
#include <stdio.h>

#include <morphology.h>

unsigned short add(unsigned short a, unsigned short b)
{
  int res = (int)a + (int)b;
  return (res > 255) ? 255 : res;
}

void usage(char *s)
{
  fprintf(stderr, "%s <ims-1> <ims-2> <imd>\n", s);
  exit(EXIT_FAILURE);
}

#define PARAM 3
int main(int argc, char *argv[])
{
  if (argc != PARAM + 1)
    usage(argv[0]);

  pnm ims1 = pnm_load(argv[1]);
  pnm ims2 = pnm_load(argv[2]);

  int imsRows = pnm_get_width(ims1);
  int imsCols = pnm_get_height(ims1);

  pnm output = pnm_new(imsRows, imsCols, PnmRawPpm);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {
      unsigned short a = pnm_get_component(ims1, i, j, 0);
      unsigned short b = pnm_get_component(ims2, i, j, 0);
      unsigned short res = add(a, b);
      for (int channel = 0; channel < 3; channel++)
        pnm_set_component(output, i, j, channel, res);
    }
  }

  pnm_save(output, PnmRawPpm, argv[3]);

  pnm_free(ims1);
  pnm_free(ims2);
  pnm_free(output);

  return EXIT_SUCCESS;
}
