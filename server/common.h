
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a tak podobne

#include <vector>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)


#define PRIKAZ_BUDUJ 1
#define PRIKAZ_BURAJ 2
#define PRIKAZ_AKTIVUJ 3
#define PRIKAZ_UTOC 4

struct Prikaz {
  int typ;
  int x;   // vyznam x,y,a,b zavisi od typu, vid staticke funkcie nizsie
  int y;
  int a;
  int b;
  static Prikaz buduj(int x, int y, int typ) {
    return (Prikaz){ PRIKAZ_BUDUJ, x, y, typ, -1 };
  }
  static Prikaz buraj(int x, int y) {
    return (Prikaz){ PRIKAZ_BURAJ, x, y, -1, -1 };
  }
  static Prikaz aktivuj(int x, int y, int destx, int desty) {
    return (Prikaz){ PRIKAZ_AKTIVUJ, x, y, destx, desty };
  }
  static Prikaz utoc(int typ, int hrac) {
    return (Prikaz){ PRIKAZ_UTOC, -1, -1, typ, hrac };
  }
};


typedef std::vector<Prikaz> Odpoved;


#define VEZA_LASER    0
#define VEZA_POSTREK  1
#define VEZA_LUPA     2
#define VEZA_CARY     3
#define VEZA_SIPY     4
#define VEZA_LIENKY   5
#define VEZA_PLACACKA 6

#define VEZA_ZBERAC    7
#define VEZA_NASAVAC   8
#define VEZA_MOTIVATOR 9

#define VEZA_LAB_VOSKA    10
#define VEZA_LAB_MUCHA    11
#define VEZA_LAB_HUSENICA 12
#define VEZA_LAB_SLIMAK   13
#define VEZA_LAB_MEDVED   14

#define VEZA_POCET_BOJOVYCH 7
#define VEZA_LAB_PRVY 10
#define VEZA_POCET_TYPOV 15

struct Veza {
  int x;
  int y;
  int typ;
  int energia;
  int baseLevel;
  std::vector<int> levelBonusy;   // pre kazdy bonus ze kolko kol plati
  int terazTahala;

  int getLevel() const { return baseLevel + levelBonusy.size(); }
};


#define UTOCNIK_VOSKA    0
#define UTOCNIK_MUCHA    1
#define UTOCNIK_HUSENICA 2
#define UTOCNIK_SLIMAK   3
#define UTOCNIK_MEDVED   4

#define UTOCNIK_POCET_TYPOV 5

struct Utocnik {
  int x;
  int y;
  int typ;
  int hp;
  int ktorehoHraca;
  int level;

  int pohybovyTimer;
  int kolkoSpomaleny;
};


struct Hrac {
  int body;
  int drevo;
  int umrel;
  std::vector<Veza> veze;
  std::vector<Utocnik> utocnici;
  std::vector<Utocnik> prichadzajuci;   // klienti nevidia
  std::vector<int> mapovanie;   // klienti nevidia
};


struct Stav {
  std::vector<Hrac> hraci;
  int cas;
};


#define MAPA_VODA    0
#define MAPA_POZEMOK 1
#define MAPA_CESTA   2
#define MAPA_CIEL    3
#define MAPA_SPAWN   4

struct Mapa {
  int pocetHracov;
  int w;
  int h;
  std::vector<std::vector<int> > pole;
  std::vector<std::vector<int> > loziska;

  int zisti(int x, int y) const {
    return (x >= 0 && x < w && y >= 0 && y < h ? pole[y][x] : MAPA_VODA);
  }
  bool priechodne(int x, int y) const {
    int m = zisti(x, y);
    return m == MAPA_CESTA || m == MAPA_CIEL || m == MAPA_SPAWN;
  }
};

#define FORMAT_VERSION 2

#endif

#ifdef reflection
// tieto udaje pouziva marshal.cpp aby vedel ako tie struktury ukladat a nacitavat

reflection(Prikaz);
  member(typ);
  member(x);
  member(y);
  member(a);
  member(b);
end();

reflection(Veza);
  member(x);
  member(y);
  member(typ);
  member(energia);
  member(baseLevel);
  member(levelBonusy);
  member(terazTahala);
end();

reflection(Utocnik);
  member(x);
  member(y);
  member(typ);
  member(hp);
  member(ktorehoHraca);
  member(level);
  member(pohybovyTimer);
  member(kolkoSpomaleny);
end();

reflection(Hrac);
  member(body);
  member(drevo);
  member(umrel);
  member(veze);
  member(utocnici);
  member(prichadzajuci);
  member(mapovanie);
end();

reflection(Stav);
  member(hraci);
  member(cas);
end();

reflection(Mapa);
  member(pocetHracov);
  member(w);
  member(h);
  member(pole);
  member(loziska);
end();

#endif
