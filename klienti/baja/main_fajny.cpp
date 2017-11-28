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
int nepriatel = 1;

set<pii> stredne;

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
        Teren kde_teren = mapa.zisti(kde.snd, kde.fst);
        if (!mapa.priechodne(kde.snd, kde.fst)) continue;
        For(i,4) {
            pii sused = make_pair(kde.fst + dr[i], kde.snd + dc[i]);
            Teren teren = mapa.zisti(sused.snd, sused.fst);
            if (teren == VODA) continue;
            if (kde_teren == CESTA && teren == POZEMOK) {
                pri_ceste[sused.fst][sused.snd]++;
                continue;
            }
            if (dis[sused.fst][sused.snd] != -1) continue;
            dis[sused.fst][sused.snd] = dis[kde.fst][kde.snd] + 1;
            q.push(sused);
        } 
    }

    kam_veze.resize(4);
    For(i, mapa.w) For(j, mapa.h) {
        Teren teren = mapa.zisti(j,i);
        if (pri_ceste[i][j] > 0) {
            kam_veze[pri_ceste[i][j]-1].insert({i,j});
        } else if ( teren == POZEMOK) {
            stredne.insert({i,j});
        }
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
        pii kam = make_pair(-1, -1);
        if (stredne.empty()) {
            For(i,4) {
                if (kam_veze[i].empty()) continue;
                kam = *kam_veze[i].begin();
                kam_veze[i].erase(kam);
                break;
            }
        } else {
            kam = *stredne.begin();
        }
        if (vykonaj(Prikaz::buduj(kam.snd, kam.fst, mojLab))) {
            mame_lab = true;
            akcia = true;
        }
    } else {
        if (najviac != -1 && vykonaj(Prikaz::buduj(kam[najviac].snd, kam[najviac].fst, zaujimave[mini]))) {
            // nic
            kam_veze[najviac].erase(kam[najviac]);
            akcia = true;
        } else {
            double r = (double)rand() / (double)RAND_MAX;
            if (r < (4.0/(stav.cas+50.0))) {
                if (najmenej != -1 && vykonaj(Prikaz::buduj(kam[najmenej].snd, kam[najmenej].fst, LASER_RAPTOR))) {
                    kam_veze[najmenej].erase(kam[najmenej]);
                    akcia = true;
                }
            }
        }
    }
    if (mame_lab) {
        while (stav.hraci[nepriatel].umrel) nepriatel++;
        if(vykonaj(Prikaz::utoc(ZAJAC, nepriatel))) {
        }
    }




    //if (kam_veze.empty()) return;
    //int i_kam = rand() % (kam_veze.size());
    //set<pii>::iterator kam_it = kam_veze.begin();
    //For(i, i_kam-1) kam_it++;
    //pii kam = *kam_it;
    //int typ = 1;
    //double pr[5] = {0.4, 0.0, 0.4, 0.2, 0.0};
    //double r = (double)rand() / (double)RAND_MAX;
    //double suc = 0.0;
    //For(i, 5) {
    //    suc += pr[i];
    //    if (r < suc) {
    //        typ = i;
    //        break;
    //    }
    //}

    //if (vykonaj(Prikaz::buduj(kam.snd, kam.fst, typ))) {
    //    kam_veze.erase(kam);
    //    // nic
    //} else {
    //    if (vykonaj(Prikaz::buduj(kam.snd, kam.fst, LASER_RAPTOR))) {
    //        kam_veze.erase(kam);
    //    }
    //}
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

