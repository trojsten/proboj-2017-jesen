
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;

#include "mapa.h"

#define chyba(...) (fprintf(stderr, __VA_ARGS__), false)

bool nacitajMapu(Mapa& mapa, string filename, int pocetHracov) {
  FILE *in = fopen(filename.c_str(), "r");
  if (!in) return chyba("neviem citat '%s'\n", filename.c_str());

  if (fgetc(in) != 'P') return chyba("'%s' ma zly format, chcem raw PPM\n", filename.c_str());
  if (fgetc(in) != '6') return chyba("'%s' ma zly format, chcem raw PPM\n", filename.c_str());

  // podporujeme komentare len medzi headerom a zvyskom (aj ked PPM standard umoznuje skoro kdekolvek)
  char c;
  fscanf(in, " %c", &c);
  while (c == '#') {
    while (c != '\n') c = fgetc(in);
    fscanf(in, " %c", &c);
  }
  ungetc(c, in);

  unsigned w, h, maxval;
  fscanf(in, "%u%u%u", &w, &h, &maxval);
  fgetc(in);
  if (maxval != 255) return chyba("'%s' ma zlu farebnu hlbku, podporujem len 24bpp\n", filename.c_str());

  mapa.w = w;
  mapa.h = h;
  mapa.pocetHracov = pocetHracov;
  mapa.pole.clear();
  mapa.pole.resize(h);

  vector<int> pocty;

  for (unsigned y = 0; y < h; y++) {
    mapa.pole[y].resize(w);
    for (unsigned x = 0; x < w; x++) {
      int r = fgetc(in);
      int g = fgetc(in);
      int b = fgetc(in);
      if (r == EOF || g == EOF || b == EOF) return chyba("necakany EOF pri citani '%s'\n", filename.c_str());
      if (r == 255 && g == 255 && b == 255) mapa.pole[y][x] = CESTA;
      else if (r == 0 && g == 0 && b == 0) mapa.pole[y][x] = POZEMOK;
      else if (r == 255 && g == 0 && b == 0) mapa.pole[y][x] = SPAWN;
      else if (r == 0 && g == 0 && b == 255) mapa.pole[y][x] = VODA;
      else if (r == 0 && g == 255 && b == 0) mapa.pole[y][x] = CIEL;
      else return chyba("zla farba %d,%d,%d na pozicii %d,%d v '%s'\n", r, g, b, x, y, filename.c_str());
      pocty.resize(max((int)pocty.size(), mapa.pole[y][x] + 1));
      pocty[mapa.pole[y][x]]++;
    }
  }

  if (pocty[CIEL] != 1) return chyba("cielov je %d namiesto 1\n", pocty[CIEL]);
  if (pocty[SPAWN] < 1) return chyba("v mape nie je ziaden spawn\n");

//   mapa.loziska.clear();
//   mapa.loziska.resize(h);
//   for (unsigned y = 0; y < h; y++) {
//     mapa.loziska[y].resize(w, 0);
//     for (unsigned x = 0; x < w; x++) {
//       if (mapa.pole[y][x] == MAPA_POZEMOK) {
//         // L ~ Geo(1/2)
//         // sum_{k=0}^inf 3k^3(1/2)^(k+1) = 39
//         while (rand() % 2) mapa.loziska[y][x]++;
//       }
//     }
//   }

  fclose(in);
  return true;
}

