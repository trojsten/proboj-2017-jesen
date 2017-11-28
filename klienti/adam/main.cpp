// very important, much wow, such don't touch this
#define protected public
#define iii int

#include <bits/stdc++.h>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

const int INF=1e9;

typedef long long ll;
typedef unsigned long long ull;
typedef long double ld;

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

struct Point {
  int x, y;
  Point(){
    x = 0;
    y = 0;
  }
  Point(ll x, ll y){
    this->x = x;
    this->y = y;
  }
};

Point operator+(Point p1, Point p2) {
  return Point(p1.x+p2.x, p1.y+p2.y);
}

bool operator<(Point p1, Point p2) {
  return (p1.x+p1.y) < (p2.x+p2.y);
}

bool operator==(Point p1, Point p2) {
  return ((p1.x==p2.x) && (p1.y==p2.y));
}

vector<Point> DIR={Point(1, 0), Point(0, -1), Point(-1, 0), Point(0, 1)};

struct Policko {
  ld sanca;
  Point pos;
  int d_ciel;
  int d_spawn;
  int volne_okolo;
  Teren typ; // VODA, POZEMOK, CESTA, CIEL, SPAWN

  Policko() {
    pos = Point(-1, -1);
    sanca = d_ciel = d_spawn = volne_okolo = 0;
    typ = VODA;
  }
};

struct Sanca_Comp {
  bool operator() (Policko *a, Policko *b) {
    return (a->sanca * 50 + pow((ld)a->d_ciel, 0.3)) < (b->sanca * 50 + pow((ld)b->d_ciel, 0.3));
  }
};

struct Predpocitanie {
  Point ciel;
  vector<Point> spawny;
  vector<Policko*> pozemky_free;
  vector<Policko*> pozemky_occu;
  vector<vector<Policko*>> plocha;
  int co_idem_vytvorit;
  int na_koho;
  int kolko_este_utocim;
};

Predpocitanie predpocitanie;

bool policko_cesta(Point p) {
  return mapa.zisti(p.x, p.y) != VODA && predpocitanie.plocha[p.y][p.x]->typ == CESTA;
}

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, DRAK))) { /* zeby malo energie? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  cerr << "ZACIATOK 204" << endl;
  cerr << "RESIZE" << endl;
///////////////////////////////////////////////////////////////////////////
  predpocitanie.co_idem_vytvorit = 3;

  predpocitanie.plocha.resize(mapa.h, vector<Policko*>(mapa.w));
  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      predpocitanie.plocha[y][x] = new Policko();
    }
  }
 
  cerr << "NACITANIE" << endl;

  // nacitanie plochy
  {
    for(int y = 0; y < mapa.h; ++y) {
      for(int x = 0; x < mapa.w; ++x) {
        predpocitanie.plocha[y][x]->typ = mapa.zisti(x, y);
        predpocitanie.plocha[y][x]->pos = Point(x, y);

        switch(predpocitanie.plocha[y][x]->typ) {
          case VODA: break;
          case POZEMOK: break;
          case CESTA: break;
          case CIEL:
            predpocitanie.ciel = Point(x, y);
            break;
          case SPAWN:
            predpocitanie.spawny.push_back(Point(x, y));
            break;
          default:
            cerr << "WTF?!" << endl;
            break;
        }

        if((predpocitanie.plocha[y][x]->typ) >= 2) {
          predpocitanie.plocha[y][x]->typ = CESTA;
        }

        Point nex;
        for(int i = 0; i < 4; ++i) {
          nex = Point(x, y) + DIR[i];

          if(!policko_cesta(nex))
            continue;

          predpocitanie.plocha[y][x]->volne_okolo++;
        }
      }
    }
  }

  cerr << "CIEL JE " << predpocitanie.ciel.y << " " << predpocitanie.ciel.x << endl;

  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      cerr << predpocitanie.plocha[y][x]->typ << " ";
    }
    cerr << endl;
  }


  cerr << "BFS Z CIELA" << endl;

  // BFS z CIELa
  {
    vector<vector<bool>> seen(mapa.h, vector<bool>(mapa.w, false));
    queue<pair<Point,int>> q;
    q.push({predpocitanie.ciel, 0});
    
    Point curr;
    int dist;
    while(!q.empty()) {
      tie(curr, dist) = q.front();
      q.pop();
      
      seen[curr.y][curr.x] = true;
      predpocitanie.plocha[curr.y][curr.x]->d_ciel = dist;

      Point nex;
      for(int i = 0; i < 4; ++i) {
        nex = curr + DIR[i];

        if(!policko_cesta(nex) || seen[nex.y][nex.x])
          continue;

        q.push({nex, dist+1});
        seen[nex.y][nex.x] = true;
      }
    }
  }

  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      cerr << predpocitanie.plocha[y][x]->d_ciel << " ";
    }
    cerr << endl;
  }


  cerr << "BFS ZO SPAWNU" << endl;

  // BFS zo SPAWNov
  {
    vector<vector<bool>> seen(mapa.h, vector<bool>(mapa.w, false));
    queue<pair<Point,int>> q;
    for(Point spawn: predpocitanie.spawny) {
      q.push({spawn, 0});
    }
    
    Point curr;
    int dist;
    while(!q.empty()) {
      tie(curr, dist) = q.front();
      q.pop();
      
      seen[curr.y][curr.x] = true;
      predpocitanie.plocha[curr.y][curr.x]->d_spawn = dist;

      Point nex;
      for(int i = 0; i < 4; ++i) {
        nex = curr + DIR[i];

        if(!policko_cesta(nex) || seen[nex.y][nex.x])
          continue;

        q.push({nex, dist+1});
        seen[nex.y][nex.x] = true;
      }
    }
  }

  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      cerr << predpocitanie.plocha[y][x]->d_spawn << " ";
    }
    cerr << endl;
  }

  cerr << "VZDIALENOST POZEMKOV DO CIELA" << endl;

  // Pozemky vzdialenost do ciela
  {
    Point nex;
    ld score;
    int pocet;
    for(int y = 0; y < mapa.h; ++y) {
      for(int x = 0; x < mapa.w; ++x) {
        if(mapa.zisti(nex.x, nex.y) == VODA || predpocitanie.plocha[nex.y][nex.x]->typ == CESTA)
          continue;

        score=0;
        pocet=0;
        for(int i = 0; i < 4; ++i) {
          nex = Point(x, y) + DIR[i];

          if(!policko_cesta(nex))
            continue;
          
          pocet++;
          score += predpocitanie.plocha[nex.y][nex.x]->d_ciel;
        }
        predpocitanie.plocha[y][x]->d_ciel = score / pocet;
      }
    }
  }
  
    for(int y = 0; y < mapa.h; ++y) {
      for(int x = 0; x < mapa.w; ++x) {
        cerr << predpocitanie.plocha[y][x]->d_ciel << " ";
      }
      cerr << endl;
    }

  cerr << "ZISTENIE PRAVDEPODOBNOSTI" << endl;

  // Zistenie pravdepodobnosti
  {
    vector<vector<bool>> seen(mapa.h, vector<bool>(mapa.w, false));
    priority_queue<pair<int, Point>, vector<pair<int, Point>>, less<pair<int, Point>>> q;
    for(Point spawn: predpocitanie.spawny) {
      q.push({predpocitanie.plocha[spawn.y][spawn.x]->d_ciel, spawn});
      predpocitanie.plocha[spawn.y][spawn.x]->sanca = 1.0 / predpocitanie.spawny.size();
    }
    
    Point curr;
    ld dist;
    while(!q.empty()) {
      curr = q.top().second;
      q.pop();

      if(curr == predpocitanie.ciel)
        break;

      dist = predpocitanie.plocha[curr.y][curr.x]->sanca;
      cerr << curr.y << " " << curr.x << " " << dist << endl;
      
      seen[curr.y][curr.x] = true;

      Point nex;
      pair<int, int> best={INF, -1};
      for(int i = 0; i < 4; ++i) {
        nex = curr + DIR[i];

        if(mapa.zisti(nex.x, nex.y) == VODA ||
            predpocitanie.plocha[nex.y][nex.x]->typ != CESTA
          )
          continue;

        if(predpocitanie.plocha[nex.y][nex.x]->d_ciel < best.first)
          best = {predpocitanie.plocha[nex.y][nex.x]->d_ciel, 1};
        else if (predpocitanie.plocha[nex.y][nex.x]->d_ciel == best.first)
          best.second++;
        else;
      }

      if(best.second == -1)
        continue;

      for(int i = 0; i < 4; ++i) {
        nex = curr + DIR[i];

        if(mapa.zisti(nex.x, nex.y) == VODA ||
            predpocitanie.plocha[nex.y][nex.x]->typ != CESTA
          )
          continue;

        if(predpocitanie.plocha[nex.y][nex.x]->d_ciel == best.first) {
          predpocitanie.plocha[nex.y][nex.x]->sanca += dist / best.second;
          
          if(seen[nex.y][nex.x])
            continue;
          
          q.push({predpocitanie.plocha[nex.y][nex.x]->d_ciel, nex});
          seen[nex.y][nex.x] = true;
        }
      }
    }
  }

  cerr << setprecision(2) << fixed;
  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      cerr << predpocitanie.plocha[y][x]->sanca << " ";
    }
    cerr << endl;
  }


  cerr << "SUCET PRAVDEPODOBNOSTI PRE STENY" << endl;

  // Sucet pravdepodobnosti pre steny
  {
    Point nex;
    for(int y = 0; y < mapa.h; ++y) {
      for(int x = 0; x < mapa.w; ++x) {
        if(predpocitanie.plocha[y][x]->typ != POZEMOK)
          continue;

        for(int i = 0; i < 4; ++i) {
          nex = Point(x + DIR[i].x, y + DIR[i].y);

          if(mapa.zisti(nex.x, nex.y) == VODA ||
              predpocitanie.plocha[nex.y][nex.x]->typ != CESTA
            )
            continue;

          predpocitanie.plocha[y][x]->sanca += predpocitanie.plocha[nex.y][nex.x]->sanca;
        }
        predpocitanie.pozemky_free.push_back(predpocitanie.plocha[y][x]);
      }
    }
  }

  for(int y = 0; y < mapa.h; ++y) {
    for(int x = 0; x < mapa.w; ++x) {
      cerr << predpocitanie.plocha[y][x]->sanca << " ";
    }
    cerr << endl;
  }

  cerr << "KONIEC" << endl;
}

// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
  // (sem patri vas kod)

  cerr << "ZACINAM TAH" << endl;

  sort(predpocitanie.pozemky_free.begin(), predpocitanie.pozemky_free.end(), Sanca_Comp());

  cerr << "\tVOLNYCH JE " << predpocitanie.pozemky_free.size() << endl;
  while(!predpocitanie.pozemky_free.empty()) {
    Policko* p;
    if(predpocitanie.co_idem_vytvorit < 5) {
      p = predpocitanie.pozemky_free.back();
    }
    else {
      p = predpocitanie.pozemky_free.front();
    }

    if(predpocitanie.co_idem_vytvorit == 1) {
      predpocitanie.co_idem_vytvorit += (rand() % 2) * 2;
    }

    cerr << "CHCEM DAT NA " << p->pos.y << " " << p->pos.x << endl;
    if (vykonaj(Prikaz::buduj(p->pos.x, p->pos.y, predpocitanie.co_idem_vytvorit))) {
      cerr << "\tROBIM TAH" << endl;
      predpocitanie.pozemky_occu.push_back(p);
      predpocitanie.pozemky_free.erase(find(
        predpocitanie.pozemky_free.begin(),
        predpocitanie.pozemky_free.end(),
        p
      ));
      
      if(stav.cas < 600)
        predpocitanie.co_idem_vytvorit = rand() % 5;
      else {
        predpocitanie.co_idem_vytvorit = rand() % 9;
      }
    }
    else {
      break;
    }
  }


  if(predpocitanie.kolko_este_utocim <= 0) {
    do {
      predpocitanie.na_koho = 1 + rand() % (mapa.pocetHracov-1);
    }while(stav.hraci[predpocitanie.na_koho].umrel);
    predpocitanie.kolko_este_utocim = 50;
  }
  for(int i = 0; i < 4; ++i) {
    cerr << "UTOCIM " << i << " " << predpocitanie.na_koho << endl;
    if(!vykonaj(Prikaz::utoc(i, predpocitanie.na_koho)))
      cerr << "\t!!!NEVIEM UTOCIT!!!" << endl;
  }
  predpocitanie.kolko_este_utocim--;

  cerr << "\t!!!NEMAM KAM DAT!!!" << endl;

  cerr << "KONCIM TAH" << endl;
}


int main() {
  // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

  nacitaj(cin, mapa);
  inicializuj();

  while (cin.good()) {
    nacitaj(cin, stav);
    prikazy.clear();
    zistiTah();
    uloz(cout, prikazy);
    cout << ".\n" << flush;   // bodka a flush = koniec odpovede
  }

  return 0;
}

