#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

#define For(i,n) for(int i = 0; i<n; i++)
#define fst first
#define snd second
#define mp make_pair

typedef pair<int, int> pii;

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

// nase
vector< vector<int> > dis;
deque< pair<double,pii> > kam_veze;
vector< vector< double > > prob_travy;
int pocet_lab = 0;
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
    dis.resize(mapa.h, vector<int>(mapa.w, -47));

    For(i, mapa.h) For(j, mapa.w) if (mapa.zisti(j,i) == CIEL) ciel = make_pair(i,j);


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
                continue;
            }
            if (dis[sused.fst][sused.snd] != -47) continue;
            dis[sused.fst][sused.snd] = dis[kde.fst][kde.snd] + 1;
            q.push(sused);
        } 
    }

    //vector< vector<bool> > kratke(mapa.h, vector<bool>(mapa.w, false));
    //For(i, mapa.w) For(j, mapa.h) {
    //    if (mapa.pole[i][j] == SPAWN) {
    //        q.push({i, j});
    //        kratke[i][j] = true;
    //        while (!q.empty()) {
    //            pii kde = q.front(); q.pop();
    //            For(i,4) {
    //                pii sused = make_pair(kde.fst + dr[i], kde.snd + dc[i]);
    //                if (!mapa.priechodne(sused.snd, sused.fst)) continue;
    //                if (kratke[sused.fst][sused.snd]) continue;
    //                if (dis[sused.fst][sused.snd] > dis[kde.fst][kde.snd]) {
    //                    For(j,4) {
    //                        pii dalsi = make_pair(sused.fst + dr[j], sused.snd + dc[j]);
    //                        Teren teren = mapa.zisti(dalsi.snd, dalsi.fst);
    //                        if (teren == POZEMOK) {
    //                            pri_ceste[dalsi.fst][dalsi.snd]--;
    //                        }
    //                    }
    //                }
    //                kratke[sused.fst][sused.snd] = true;
    //                q.push(sused);
    //            }
    //        }
    //    }
    //}

    vector< pair<int, pii> > vsetky;
    vector< vector<double> > prob(mapa.h, vector<double>(mapa.w, 0.0));
    For(i, mapa.h) For(j, mapa.w) {
        if (mapa.zisti(j,i) == SPAWN) {
            prob[i][j] = 1.0;
            vsetky.push_back(make_pair(dis[i][j], mp(i,j)));
        }
        else if (mapa.priechodne(j,i)) {
            vsetky.push_back(make_pair(dis[i][j], mp(i,j)));
        }
    }
    sort(vsetky.begin(), vsetky.end());
    reverse(vsetky.begin(), vsetky.end());
    //cerr << "HAFHAF "<<vsetky.size()<<endl;
    For(k, vsetky.size()) {
        pair<int, pii> p = vsetky[k];
        cerr << "HAFHAF "<<prob[p.snd.fst][p.snd.snd]<<endl;
        vector<pii> dobry_sused;
        For(i,4) {
            pii sused = make_pair(p.snd.fst + dr[i], p.snd.snd + dc[i]);
            if (!mapa.priechodne(sused.snd, sused.fst)) continue;
            if (dis[sused.fst][sused.snd] +1 == p.fst) {
                dobry_sused.push_back(sused);
            }
        }
        //cerr << "kkoneic "<<dobry_sused.size()<<endl;
        if (dobry_sused.empty()) continue;
        For(i, dobry_sused.size()) {
            pii sus = dobry_sused[i];
            //cerr << "sused "<<sus.fst<<" "<<sus.snd<<endl;
            prob[sus.fst][sus.snd] += prob[p.snd.fst][p.snd.snd] / dobry_sused.size();
        }
        //cerr << "kon2 "<<endl;
    }

    prob_travy.resize(mapa.h, vector<double>(mapa.w, 0.0));
    For(i, mapa.h) For(j, mapa.w) {
        Teren teren = mapa.zisti(j,i);
        double suc = 0.0;
        For(k, 4) {
            pii sused = make_pair(i + dr[k], j + dc[k]);
            if (!mapa.priechodne(sused.snd, sused.fst)) continue;
            if (mapa.zisti(sused.snd, sused.fst) == CIEL) continue;
            suc += prob[sused.fst][sused.snd];
        }
        if (suc > 1e-9 && teren == POZEMOK) {
            suc += 1e-7*((double)rand()/(double)RAND_MAX);
            prob_travy[i][j] = suc;
            kam_veze.push_back(mp(suc, mp(i,j)));
        } else if (teren == POZEMOK) {
            stredne.insert(mp(i,j));
        }
    }
    sort(kam_veze.begin(), kam_veze.end());
    reverse(kam_veze.begin(), kam_veze.end());
    cerr << "kon2 "<<kam_veze.size()<<endl;
    cerr << "stredne "<<stredne.size()<<endl;
    For(i, kam_veze.size()) {
        cerr << kam_veze[i].fst << " "<<kam_veze[i].snd.fst<<" "<<kam_veze[i].snd.snd<<endl;
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

    //double neutocit = (double)rand() / (double)RAND_MAX;
    bool akcia = false;
    if(stav.cas / 250 >= pocet_lab ) {
        int mojLab = LAB_ZAJAC;
        pii kam = make_pair(-1, -1);
        bool stre = false;
        bool nerob_nic = false;
        if (!stredne.empty()) {
            kam = *stredne.begin();
            stre = true;
        } else {
            if (kam_veze.empty()) {
                if (stav.hraci[0].energia > zistiCenuVeze(stav, 0, LAB_ZAJAC)) {
                    int mini = -1, minj = -1;
                    For(i, stav.hraci[0].veze.size()) {
                        int x = stav.hraci[0].veze[i].x;
                        int y = stav.hraci[0].veze[i].y;
                        if (stav.hraci[0].veze[i].typ == LASER_RAPTOR) {
                            if (mini == -1 || mapa.zisti(minj, mini) == POZEMOK && prob_travy[mini][minj] > prob_travy[y][x]) {
                                mini = y;
                                minj = x;
                            }
                        }
                    }
                    if (minj != -1) {
                        vykonaj(Prikaz::buraj(minj, mini));
                        vykonaj(Prikaz::buduj(minj, mini, LAB_ZAJAC));
                    }
                    nerob_nic = true;
                }
            } else {
                kam = (kam_veze.back()).snd;
            }
        }
        if (!nerob_nic) {
            //cerr << "KAM "<<kam.fst<<" "<<kam.snd<<" "<<stav.hraci[0].energia<<endl;
            if (vykonaj(Prikaz::buduj(kam.snd, kam.fst, mojLab))) {
                if (stre) {
                    stredne.erase(kam);
                } else {
                    kam_veze.pop_back();
                }
                akcia = true;
                pocet_lab++;
            }
        }
    }
    if(stav.cas / 250 < pocet_lab || kam_veze.empty()) {
        if (!akcia) {
            if (vykonaj(Prikaz::buduj(kam_veze.front().snd.snd, kam_veze.front().snd.fst, zaujimave[mini]))) {
                // nic
                cerr << "vezax "<<kam_veze.front().snd.fst<<endl;
                cerr << "vezay "<<kam_veze.front().snd.snd<<endl;
                kam_veze.pop_front();
                akcia = true;
            } else {
                double r = (double)rand() / (double)RAND_MAX;
                //if (r < (6.0/(stav.cas+50.0))) {
                    if (vykonaj(Prikaz::buduj(kam_veze.front().snd.snd, kam_veze.front().snd.fst, LASER_RAPTOR))) {
                        kam_veze.pop_front();
                        cerr << "vezax "<<kam_veze.front().snd.fst<<endl;
                        cerr << "vezay "<<kam_veze.front().snd.snd<<endl;
                        akcia = true;
                    }
                //}
            }
        }
        if (kam_veze.empty()) {
            if (stav.hraci[0].energia > zistiCenuVeze(stav, 0, zaujimave[mini])) {
                int maxi = -1, maxj = -1;
                For(i, stav.hraci[0].veze.size()) {
                    int x = stav.hraci[0].veze[i].x;
                    int y = stav.hraci[0].veze[i].y;
                    if (stav.hraci[0].veze[i].typ == LASER_RAPTOR) {
                        if (maxi == -1 || prob_travy[maxi][maxj] < prob_travy[y][x]) {
                            maxi = y;
                            maxj = x;
                        }
                    }
                }
                if (maxj != -1) {
                    vykonaj(Prikaz::buraj(maxj, maxi));
                    vykonaj(Prikaz::buduj(maxj, maxi, zaujimave[mini]));
                }
            }
        }
    }
    if (pocet_lab > 0) {
        For(i, pocet_lab) {
            while (stav.hraci[nepriatel].umrel) nepriatel++;
            if(vykonaj(Prikaz::utoc(ZAJAC, nepriatel))) {
            }
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

