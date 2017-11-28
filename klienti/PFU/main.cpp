#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define POCET_LABOV 4
#define POCET_VEZI 5
#define POCET_BUDOV 9

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

int akoBudemUtocit;
bool defense = true;

struct policko {
    int cesty, vzdialenost, x, y;
    bool stavanie;
    policko() : cesty(0), vzdialenost(INT_MAX) {}
    policko(int _x, int _y, int _cesty, int _vzdialenost, bool _stavanie)
            : x(_x), y(_y), cesty(_cesty), vzdialenost(_vzdialenost), stavanie(_stavanie) {}
};

struct compare_policko {
    bool operator()(const policko &x, const policko &y) {
        if(x.stavanie && y.stavanie)
            return x.cesty == y.cesty ? x.vzdialenost < y.vzdialenost : x.cesty > y.cesty;
        if(x.stavanie && !y.stavanie)
            return true;
        return false;
    }
};

struct node {
    int vzdialenost, x, y;
    pair<int, int> predchodca;
    node() : vzdialenost(INT_MAX), predchodca(make_pair(-1, -1)) {}
};

vector<vector<policko> > policka;
vector<policko> linearne_policka;
vector<vector<node> > hra;
vector<int> perioda;
pair<int, int> pohar;
int dr[] = {1, 0, -1, 0}, dc[] = {0, 1, 0, -1};
int volne_idx = 0;

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

int spocitaj_spawn() {
    int ans = 0;
    for(int i = 0; i < mapa.h; i++)
        for(int j = 0; j < mapa.w; j++)
            if(mapa.pole[i][j] == SPAWN)
                ans++;
    return ans;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
    srand(time(NULL) * getpid());
    akoBudemUtocit = rand() % UTOCNIK_POCET_TYPOV;

    for(int i = 0; i < 8; i++) perioda.push_back(4);
    perioda.push_back(1);

    linearne_policka.resize(mapa.h * mapa.w);
    policka.resize(mapa.h);
    int R = mapa.h, S = mapa.w, idx = 0;
    for(int i = 0; i < R; i++) {
        policka[i].resize(S);
        for(int j = 0; j < S; j++) {
            policka[i][j].y = i;
            policka[i][j].x = j;
            if(mapa.pole[i][j] == CIEL)
                pohar = make_pair(i, j);
        }
    }

    queue<pair<int, int> > q;
    q.push(pohar);
    pair<int, int> kde;
    policka[pohar.first][pohar.second].vzdialenost = 0;
    while(!q.empty()) {
        kde = q.front();
        q.pop();
        if(mapa.pole[kde.first][kde.second] == POZEMOK) {
            policka[kde.first][kde.second].stavanie = true;
        }
        for(int i = 0; i < 4; i++) {
            int tr = kde.first + dr[i];
            int tc = kde.second + dc[i];
            if(tr >= 0 && tr < R && tc >= 0 && tc < S) {
                if(mapa.pole[tr][tc] == CESTA)
                    policka[kde.first][kde.second].cesty++;
                if(policka[tr][tc].vzdialenost == INT_MAX) {
                    q.push(make_pair(tr, tc));
                    policka[tr][tc].vzdialenost = policka[kde.first][kde.second].vzdialenost + 1;
                }
            }
        }
    }
    for(int i = 0; i < R; i++)
        for(int j = 0; j < S; j++)
            linearne_policka[idx++] = policka[i][j];
    sort(linearne_policka.begin(), linearne_policka.end(), compare_policko());
}


int minob() {
    int ans = 0;
    int mn = INT_MAX, mni = 1;
    for(int h = 1; h < stav.hraci.size(); h++) {
        ans = 0;
        if(stav.hraci[h].umrel) continue;
        for(int i = 0; i < stav.hraci[h].veze.size(); i++) {
            if(stav.hraci[h].veze[i].typ <= 4) {
                ans++;
            }
        }
        if(ans < mn) {
            mn = ans;
            mni = h;
        }
    }
    return mni;
}


int pocetVezi(int typ) {
    int result = 0;
    FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
    return result;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
int p_idx = 0, monsters = 0;
bool sec = false;
double mk = 1;
int mk2 = 21;
int all = 9;
void zistiTah() {
    if(!sec && monsters > all) {
        if(!vykonaj(Prikaz::buduj(linearne_policka[mk2].x, linearne_policka[mk2].y, VEZA_LAB_PRVY + 2))) {
            vykonaj(Prikaz::utoc(2, minob()));
            return;
        }
        mk2++;
        all += (int)((double)9 * mk);
        mk -= 0.3;
        sec = true;
    }
    if(!vykonaj(Prikaz::utoc(2, minob())))
       vykonaj(Prikaz::buduj(linearne_policka[20].x, linearne_policka[20].y, VEZA_LAB_PRVY + 2));
    if(vykonaj(Prikaz::buduj(linearne_policka[volne_idx].x, linearne_policka[volne_idx].y, perioda[p_idx]))) {
        volne_idx++;
        p_idx++;
        p_idx %= perioda.size();
        monsters++;
    }
    bool ok = true;
    int last = -1;
    for(int i = 1; i < stav.hraci[0].veze.size(); i++) {
        if(stav.hraci[0].veze[i].typ != LAB_KORITNACKA) {
            last = -1;
            continue;
        }
        if(last == 2 && stav.hraci[0].veze[i].energia != stav.hraci[0].veze[i - 1].energia)
            ok = false;
        last = 2;
    }
    if(ok) vykonaj(Prikaz::utoc(2, 1));
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

