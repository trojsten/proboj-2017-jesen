
#ifndef COMMON_H
#define COMMON_H

// vseobecne datove struktury a tak podobne

#include <vector>

#define FOREACH(it,c) for (typeof((c).begin()) it = (c).begin(); it != (c).end(); ++it)

enum TypPrikazu{
    BUDUJ,
    BURAJ,
    UTOC
};

struct Prikaz {
  TypPrikazu typ;
  int x;   // vyznam x,y,a,b zavisi od typu, vid staticke funkcie nizsie
  int y;
  int a;
  int b;
  static Prikaz buduj(int x, int y, int typ) {
    return (Prikaz){ BUDUJ, x, y, typ, -1 };  //postavi vezu typ na suradnice x,y
  }
  static Prikaz buraj(int x, int y) {
    return (Prikaz){ BURAJ, x, y, -1, -1 };   //buranie je zadarmo
  }
  static Prikaz utoc(int typ, int hrac) {
    return (Prikaz){ UTOC, -1, -1, typ, hrac };
  }
};


typedef std::vector<Prikaz> Odpoved;

//konstanty (ceny a damage pre jednotlive dvojice) este niesu
enum TypBudovy{
    TROLL,              //sedí pri ceste, mláti kyjakom. Proti zombie
    HYDRA,              //chomp, môže aj na viac strán. Loví zajace (iba proti zajacom)
    DRAK,               //chrlí oheň na všetko
    TEMNY_CARODEJNIK,   //čaruje
    LASER_RAPTOR,       // TODO

    LAB_ZAJAC,
    LAB_ZOMBIE,
    LAB_KORITNACKA,
    LAB_JEDNOROZEC
};

#define VEZA_POCET_BOJOVYCH 5
#define VEZA_LAB_PRVY 6
#define VEZA_POCET_TYPOV 9

struct Veza {
  int x;
  int y;
  int typ;
  //int energia;      //TODO nejaký cooldown pre laby
  int terazTahala; //aby nemohol burat v kole ked striela TODO mozno zrusim
};
//konstanty (rychlost a hp) este niesu
enum TypUtocnika{
    ZAJAC,
    ZOMBIE,
    KORITNACKA,
    JEDNOROZEC
};

#define UTOCNIK_POCET_TYPOV 4

struct Utocnik {
  int x;
  int y;
  TypUtocnika typ;
  int hp;
  int ktorehoHraca;
  int pohybovyTimer; //kolko tahov bude este na danom policku
};


struct Hrac {
  int body;
  int energia;
  int umrel;   //TODO int? nie nahodou bool?
  std::vector<Veza> veze;
  std::vector<Utocnik> utocnici;        //už v hracom poli
  std::vector<Utocnik> prichadzajuci;   // klienti nevidia  //este len maju zautocita su na spawne TODO načo je to tu?
  std::vector<int> mapovanie;   // klienti nevidia   //zrušiť, slúžilo na náhodné prečíslovanie protihráčov pri maskovaní stavu, načo keď oni sú náhodne vybratí? treba to aby klient mal číslo 0
};


struct Stav {
  std::vector<Hrac> hraci;
  int cas;
};

enum Teren{
    VODA,
    POZEMOK,
    CESTA,
    CIEL,
    SPAWN
};

struct Mapa {
  int pocetHracov;
  int w;
  int h;
  std::vector<std::vector<Teren> > pole;

  Teren zisti(int x, int y) const {
    return (x >= 0 && x < w && y >= 0 && y < h ? pole[y][x] : VODA);
  }
  bool priechodne(int x, int y) const {
    int m = zisti(x, y);
    return m == CESTA || m == CIEL || m == SPAWN;
  }
};


#endif

#ifdef reflectenum
reflectenum(TypPrikazu);
reflectenum(TypBudovy);
reflectenum(TypUtocnika);
reflectenum(Teren);
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
  //member(energia);
  member(terazTahala);
end();

reflection(Utocnik);
  member(x);
  member(y);
  member(typ);
  member(hp);
  member(ktorehoHraca);
  member(pohybovyTimer);
end();

reflection(Hrac);
  member(body);
  member(energia);
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
end();

#endif
