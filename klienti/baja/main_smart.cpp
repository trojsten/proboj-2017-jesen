#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
#include <set>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define For(i,n) for(int i = 0; i<n; i++)
#define fst first
#define snd second

typedef pair<int, int> pii;

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

// nase
vector< vector<int> > dis;
vector< vector<int> > pri_ceste;
vector< set<pii> > kam_veze;
bool mame_lab = false;

pii ciel;
int dr[] = {0,1,0,-1};
int dc[] = {1,0,-1,0};

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
    bool uspech = vykonajPrikaz(mapa, stav, 0, p);
    if (uspech) prikazy.push_back(p);   // potom to posleme serveru
    return uspech;
}


// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
    srand(time(NULL) * getpid());
    dis.resize(mapa.h, vector<int>(mapa.w, -1));
    pri_ceste.resize(mapa.h, vector<int>(mapa.w, 0));

    For(i, mapa.w) For(j, mapa.h) if (mapa.pole[i][j] == CIEL) ciel = make_pair(i,j);

    queue<pii> q;
    q.push(ciel);
    dis[ciel.fst][ciel.snd] = 0;
    while (!q.empty()) {
        pii kde = q.front(); q.pop();
        Teren teren = mapa.zisti(kde.fst, kde.snd);
        if (teren != CESTA) continue;
        For(i,4) {
            pii sused = make_pair(kde.fst + dr[i], kde.snd + dc[i]);
            Teren teren = mapa.zisti(sused.fst, sused.snd);
            if (teren == VODA) continue;
            if (teren == POZEMOK) {
                pri_ceste[sused.fst][sused.snd]++;
                continue;
            }
            if (dis[sused.fst][sused.snd] != -1) continue;
            dis[sused.fst][sused.snd] = dis[kde.fst][kde.snd] + 1;
            q.push(sused);
        } 
    }

    kam_veze.resize(4);
    For(i, mapa.w) For(j, mapa.h) if (pri_ceste[i][j] > 0) {
        kam_veze[pri_ceste[i][j]-1].insert({i,j});
    }
}

vector<int> pocetVezi() {
    vector<int> res(VEZA_POCET_TYPOV, 0);
    FOREACH(it, stav.hraci[0].veze) res[it->typ]++;
    return res;
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
    //int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
    //int naKoho = 1 + rand() % (mapa.pocetHracov-1);
    //if(!vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho)))
    //    vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab));
    //else
    //    vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, rand()%VEZA_POCET_TYPOV));
    
    vector<int> pocty = pocetVezi();
    int zauj_count = 3;
    int zaujimave[zauj_count] = {TROLL, DRAK, TEMNY_CARODEJNIK};
    int mini = TROLL;
    For(i, zauj_count) {
        if (pocty[mini] > pocty[zaujimave[i]]) mini = i;
    }
    vector<pii> kam(4, make_pair(-1, -1));;
    int najviac = -1, najmenej = -1;
    for (int i = 0; i<3; i++) {
        if (kam_veze[i].size() > 0) {
            int ind = rand() % kam_veze[i].size();
            set<pii>::iterator ktory = kam_veze[i].begin();
            For(k,ind-1) ktory++;
            //kam[i] = kam_veze[i][ind];
            kam[i] = *ktory;
            najviac = i;
            if (najmenej == -1) najmenej = i;
        }
    }

    double neutocit = (double)rand() / (double)RAND_MAX;
    bool akcia = false;
    if (!mame_lab) {
        int mojLab = LAB_ZAJAC;
        if (vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab))) {
            mame_lab = true;
            akcia = true;
        }
    } else if (neutocit < 0.5) {
        if (najviac != -1 && vykonaj(Prikaz::buduj(kam[najviac].fst, kam[najviac].snd, zaujimave[mini]))) {
            // nic
            kam_veze[najviac].erase(kam[najviac]);
            akcia = true;
        } else {
            double r = (double)rand() / (double)RAND_MAX;
            if (r < 0.1) {
                if (najmenej != -1 && vykonaj(Prikaz::buduj(kam[najmenej].fst, kam[najmenej].snd, LASER_RAPTOR))) {
                    kam_veze[najmenej].erase(kam[najmenej]);
                    akcia = true;
                }
            }
        }
    } else if (mame_lab && !akcia) {
        int naKoho = 1 + rand() % (mapa.pocetHracov-1);
        !vykonaj(Prikaz::utoc(LAB_ZAJAC, naKoho));
    }

    
    // int i_kam = rand() % (kam_veze.size());
    // pii kam = kam_veze[i_kam];
    // int typ = 1;
    // double pr[5] = {0.35, 0.0, 0.35, 0.3, 0.0};
    // double r = (double)rand() / (double)RAND_MAX;
    // double suc = 0.0;
    // For(i, 5) {
    //     suc += pr[i];
    //     if (r < suc) {
    //         typ = i;
    //         break;
    //     }
    // }

    // if (vykonaj(Prikaz::buduj(kam.fst, kam.snd, typ))) {
    //     // nic
    // } else {
    //     vykonaj(Prikaz::buduj(kam.fst, kam.snd, LASER_RAPTOR));
    // }
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

