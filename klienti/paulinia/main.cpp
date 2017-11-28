#include <ctime>
#include <unistd.h>
#include<bits/stdc++.h>

using namespace std;
// using namespace Prikaz;

#define FOR(i,n)	for(int i=0;i<(int)n;i++)
#define FOB(i,n)	for(int i=n;i>=1;i--)
#define MP(x,y)	make_pair((x),(y))
#define ii pair<int, int>
#define lli long long int
#define ulli unsigned long long int
#define lili pair<lli, lli>
#ifdef EBUG
#define DBG	if(1)
#else
#define DBG	if(0)
#endif
#define SIZE(x) int(x.size())
const int infinity = 2000000999 / 2;
const long long int inff = 4000000000000000999;

template <class T, class U>
ostream& operator<<(ostream& out, const pair<T, U> &par) {
    out << "[" << par.first << ";" << par.second << "]";
    return out;
}

template <class T>
ostream& operator<<(ostream& out, const vector<T>& v) {
    FOR(i, v.size()){
        if(i) out << " ";
        out << v[i];
    }
    out << endl;
    return out;
}

#include "common.h"
#include "marshal.h"
#include "update.h"

static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;

const int vezaCena0[VEZA_POCET_TYPOV] = { 159, 133, 147, 296,  24,   472,    480,        461,      486 };
const int vezaCena1[VEZA_POCET_TYPOV] = { 10,   17,   7,  10,   5,   0,       0,         0,         0 };
const int vezaCena2[VEZA_POCET_TYPOV] = { 0,    0,    0,   5,   0,   0,       0,         0,         0 };

bool zastavane = false;

int pocetVezi(int typ) {
    int result = 0;
    FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
    return result;
}

int zistiCenu(int type){
    int cnt = pocetVezi(type);
    return vezaCena0[type] + vezaCena1[type] * cnt + vezaCena2[type] * cnt * cnt;
}

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Pkaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
    bool uspech = vykonajPrikaz(mapa, stav, 0, p);
    cerr << "Robi prikaz " << p.a << " A uspesny? " << uspech << endl;
    if (uspech) prikazy.push_back(p);   // potom to posleme serveru
    return uspech;
}

int add_tower(queue<ii> &Q, vector<vector<bool> > &M, priority_queue<ii> &defense, vector<vector<bool> > &reachable){
    if(!Q.size()){
        zastavane = true;
        cerr << "WTF" << endl;
        return -1;
    }
    int x = Q.front().first, y = Q.front().second;
    M[x][y] = true;
    Q.pop();
    if(mapa.zisti(x, y) == POZEMOK){
        int type = defense.top().second;
        int cena = defense.top().first;
        if(!vykonaj(Prikaz::buduj(x, y, type))){
            Q.push({x, y});
            return -1;
        }
        defense.pop();
        cerr << "Cena oneho je " << cena << " a typ " << type << endl;
        cerr << "Na vrchu je " << defense.top() << endl;
        defense.push({-zistiCenu(type), type});
//         cerr << "Na vrchu je " << defense.top() << endl;
        return 1;
    }
    
    if(!mapa.priechodne(x, y)) return 0;
    if(!reachable[x][y]) return 0;
    
    FOR(i, 4){
        if(mapa.priechodne(x + DX[i], y + DY[i]) || mapa.zisti(x + DX[i], y + DY[i]) == POZEMOK){
            if(!M[x + DX[i]][y + DY[i]]){
                M[x + DX[i]][y + DY[i]] = true;
                Q.push({x + DX[i], y + DY[i]});
            }
        }   
    }
    
    return 0;
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj(queue<ii> &Q, vector<vector<bool> > &M, priority_queue<ii> &defense, vector<vector<bool> > &reachable, vector<ii> &tolabs){
    M.resize(mapa.w, vector<bool>(mapa.h, 0));
    reachable.resize(mapa.w, vector<bool>(mapa.h, 0));
    srand(time(NULL) * getpid());
    int x = -1, y = -1;
    FOR(i, mapa.w){
        FOR(j, mapa.h){
            if(mapa.zisti(i, j) == CIEL){
                x = i, y = j;
                break;
            }
        }
    }
    
    
    M[x][y] = true;
    
    FOR(i, 4){
        if((mapa.priechodne(x + DX[i], y + DY[i]) || mapa.zisti(x + DX[i], y + DY[i]) == POZEMOK) && (!M[x + DX[i]][y + DY[i]])){
            int X = x + DX[i], Y = y + DY[i];
            if(mapa.zisti(X, Y) == POZEMOK){
                bool dasa = 0;
                FOR(j, 4){
                    if(mapa.zisti(X + DX[j], Y + DY[j]) == POZEMOK && reachable[X + DX[j]][Y + DY[j]]){
                        dasa = 1;
                        break;
                    }
                }
                if(dasa) Q.push({X, Y});
            }
            else{
                Q.push({x + DX[i], y + DY[i]});
            }
        }
    }
    
    auto chcem = {DRAK, TEMNY_CARODEJNIK, LASER_RAPTOR};
    
//     random_shuffle(chcem.begin(), chcem.end());
    
    for(TypBudovy type : chcem){
        defense.push({0, (int)type});
    }
    
    vector<vector<int> > dist(mapa.w, vector<int>(mapa.h, infinity));
    
    queue<ii> FR;
    FR.push({x, y});
    dist[x][y] = 0;
    while(FR.size()){
        int X = FR.front().first, Y = FR.front().second;
        FR.pop();
        int cn = dist[X][Y];
        FOR(i, 4){
            if(mapa.priechodne(X + DX[i], Y + DY[i]) && dist[X + DX[i]][Y + DY[i]] > cn){
                dist[X + DX[i]][Y + DY[i]] = cn + 1;
                FR.push({X + DX[i], Y + DY[i]});
            }
        }
    }
    
//     cerr << "dist:\n " << dist;
    
    FOR(i, mapa.w){
        FOR(j, mapa.h){
            if(mapa.zisti(i, j) != SPAWN) continue;
            queue<ii> F;
            F.push({i, j});
            vector<vector<int> > vzd(mapa.w, vector<int>(mapa.h, infinity));
            vzd[i][j] = 0;
            
            while(F.size()){
                int X = F.front().first, Y = F.front().second;
                F.pop();
                int cn = vzd[X][Y];
                FOR(l, 4){
//                     cerr << "Mozem ist do " << X + DX[l] << "|" << Y + DY[l] << endl;
//                     if(mapa.priechodne(X + DX[l], Y + DY[l]) != VODA) cerr << "Je to " << mapa.priechodne(X + DX[l], Y + DY[l]) << endl;
                    if(mapa.priechodne(X + DX[l], Y + DY[l]) && vzd[X + DX[l]][Y + DY[l]] > cn){
//                         if(reachable[X + DX[l]][Y + DY[l]]) continue;
                        vzd[X + DX[l]][Y + DY[l]] = cn + 1;
                        F.push({X + DX[l], Y + DY[l]});
                    }
                }
            }
            
            FOR(l, mapa.w){
                FOR(k, mapa.h){
//                     cerr << vzd[l][k] << "+" << dist[l][k] << "|"; 
                    if(vzd[l][k] + dist[l][k] == dist[i][j]) reachable[l][k] = true;
                }
//                 cerr << endl;
            }
            
        }
    }
    
    reachable[x][y] = false;
    
    cerr << "reachable:\n " << reachable;
    
    FOR(i, mapa.w){
        FOR(j, mapa.h){
            if(mapa.zisti(i, j) == POZEMOK){
                bool dasa = 1;
                cerr << "Moze byt " << i << " | " << j << endl;
                FOR(l, 4){
                    if(mapa.priechodne(i + DX[l], j + DY[l])){
//                         cerr << "prve /*/*priechodne*/*/ " << endl;
                        if(reachable[i + DX[l]][j + DY[l]]){
//                             cerr << "ee  " << endl;
                            dasa = 0;
                            break;
                        }
//                         cerr << "Moze " << endl;
                    }
                }
                if(dasa) tolabs.push_back({i, j});
            }
        }
    }
    
    cerr << "Tolabs: " << tolabs;
    
}

// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah(queue<ii> &Q, vector<vector<bool> > &M, priority_queue<ii> &defense, vector<vector<bool> > &reachable, vector<ii> &tolabs) {
    auto utocne =  {LAB_ZAJAC,
                    LAB_ZOMBIE,
                    LAB_KORITNACKA,
                    LAB_JEDNOROZEC};
    if(!zastavane) while(add_tower(Q, M, defense, reachable) >= 0);
    if(stav.hraci[0].veze.size()){
        int nahodny = rand() % stav.hraci[0].veze.size();
        if(stav.hraci[0].veze[nahodny].typ != TEMNY_CARODEJNIK){
            int jeho = zistiCenu(TEMNY_CARODEJNIK);
            if(stav.hraci[0].energia > jeho){
                int x = stav.hraci[0].veze[nahodny].x, y = stav.hraci[0].veze[nahodny].y;
                vykonaj(Prikaz::buraj(x, y));
                if(!vykonaj(Prikaz::buduj(x, y, TEMNY_CARODEJNIK))){
                    zastavane = false;
                    Q.push({x, y});
                    cerr << "FUCK" << endl;
                }
            }
        }
        if(rand() % 20 == 0 && tolabs.size()){
            auto mini = LAB_ZAJAC;
            for(auto u : utocne){
                if(zistiCenu(u) < zistiCenu(mini)) mini = u;
            }
            vykonaj(Prikaz::buduj(tolabs.back().first, tolabs.back().second, mini));
        }
        int mn = 1;
        FOR(i, stav.hraci.size()){
            if(stav.hraci[mn].veze.size() < stav.hraci[i].veze.size()) mn = i;
        }
        for(auto u : utocne){
            while(vykonaj(Prikaz::utoc(u, mn)));
        }
    }
}


int main() {
    // v tejto funkcii su vseobecne veci, nemusite ju menit (ale mozte).

    nacitaj(cin, mapa);
    queue<ii> Q;
    vector<vector<bool> > bolo, reachable;
    priority_queue<ii> defense;
    vector<ii> tolabs;
    inicializuj(Q, bolo, defense, reachable, tolabs);


    while (cin.good()) {
        nacitaj(cin, stav);
        prikazy.clear();
        zistiTah(Q, bolo, defense, reachable, tolabs);
        uloz(cout, prikazy);
        cout << ".\n" << flush;   // bodka a flush = koniec odpovede
    }

    return 0;
}

