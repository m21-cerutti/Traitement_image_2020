#include <math.h>

#include <morphology.h>
#include <se.h>

void maximum(unsigned short *val, unsigned short *max)
{
  if (*val > *max)
  {
    *val = *max;
  }
}

void minimum(unsigned short *val, unsigned short *min)
{
  if (*val < *min)
  {
    *val = *min;
  }
}

void process(int s,
             int hs,
             pnm ims,
             pnm imd,
             void (*pf)(unsigned short *, unsigned short *))
{
  pnm shape = se(s, hs);

  int imsRows = pnm_get_height(ims);
  int imsCols = pnm_get_width(ims);

  for (int i = 0; i < imsRows; i++)
  {
    for (int j = 0; j < imsCols; j++)
    {

      unsigned short res = pnm_get_component(ims, i, j, 0);

      for (int x = -hs; x < hs+1; x++)
      {
        for (int y = -hs; y < hs+1; y++)
        {

          if ((i + y) >= imsRows || (j + x) >= imsCols || (i + y) < 0 || (j + x) < 0)
          {
            continue;
          }

          if (pnm_get_component(shape, y + hs, x + hs, 0) == 0)
            continue;

          unsigned short val = pnm_get_component(ims, i + y, j + x, 0);
          pf(&res, &val);
        }
      }

      for (int channel = 0; channel < 3; channel++)
      {
        pnm_set_component(imd, i, j, channel, res);
      }
    }
  }
}
