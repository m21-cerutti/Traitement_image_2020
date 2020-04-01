#include <math.h>

#include <morphology.h>
#include <se.h>

#define NB_CHANNEL 3
#define NB_BITS_CHANNEL 8
#define NUMBER_VALUES 0x0000FF //255

typedef unsigned short color3s;

void maximum(color3s *val, color3s *max)
{
  if (*val < *max)
  {
    *max = *val;
  }
}

void minimum(color3s *val, color3s *min)
{
  if (*val > *min)
  {
    *min = *val;
  }
}

color3s extractColor(pnm source, int i, int j)
{
  color3s color = 0;
  for (int c = 0; c < NB_CHANNEL; c++)
  {
    unsigned short val = pnm_get_component(source, i, j, c);
    color3s channel_col = val << (c * NB_BITS_CHANNEL);
    color = color|channel_col;
  }
  return color;
}

void putColor(pnm source, int i, int j, color3s color)
{
  for (int c = 0; c < NB_CHANNEL; c++)
  {
    color3s channel_col = (color >> (c * NB_BITS_CHANNEL));
    unsigned short val = channel_col & NUMBER_VALUES;
    pnm_set_component(source, i, j, c, val);
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
      color3s res = extractColor(ims, i, j);
      for (int x = -hs; x < hs + 1; x++)
      {
        for (int y = -hs; y < hs + 1; y++)
        {
          if ((i + y) >= imsRows || (j + x) >= imsCols || (i + y) < 0 || (j + x) < 0)
            continue;

          if (pnm_get_component(shape, y + hs, x + hs, 0) != 255)
            continue;

          color3s val = extractColor(ims, i + y, j + x);
          pf(&val, &res);
        }
      }
      putColor(imd, i, j, res);
    }
  }
}
