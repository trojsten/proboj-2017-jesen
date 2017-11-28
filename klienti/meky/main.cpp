#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;
vector<vector<int> > pocet_okolitych_ciest;
	
int akoBudemUtocit;
int naKoho = 1;

#define For(i,n) for(int i=0; i<(n); i++)
typedef pair<int,int> pii;

// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
  bool uspech = vykonajPrikaz(mapa, stav, 0, p);
  if (uspech) prikazy.push_back(p);   // potom to posleme serveru
  return uspech;
}

void evaluate(){
  pocet_okolitych_ciest.resize(mapa.w);
  int dx[] = {0,0,1,-1};
  int dy[] = {1,-1,0,0};
  for(int x = 0; x < mapa.w; x++){
	for(int y = 0; y < mapa.h; y++){
		if(mapa.zisti(x,y) == POZEMOK) {
                    pocet_okolitych_ciest[x].push_back(0);
			for(int i = 0; i < 4; i++)
				if(mapa.priechodne(x+dx[i], y+dy[i]))
					pocet_okolitych_ciest[x][y]++;
		}
                else {
                    pocet_okolitych_ciest[x].push_back(-1);
                }
	}
  }
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
  srand(time(NULL) * getpid());
  akoBudemUtocit = 0; //rand() % UTOCNIK_POCET_TYPOV;
  evaluate();
}


int pocetVezi(int typ) {
  int result = 0;
  FOREACH(it, stav.hraci[0].veze) if (it->typ == typ) result++;
  return result;
}

void BFS(int x, int y, vector<vector<int> > &vzd) {
    queue<int> Q;
    Q.push(x); Q.push(y); Q.push(0);
    vzd[x][y] = 0;
    while(!Q.empty()) {
        int x=Q.front(); Q.pop();
        int y=Q.front(); Q.pop();
        int d=Q.front(); Q.pop();
        int dx[]={0,0,1,-1};
        int dy[]={1,-1,0,0};
        For(i,4) {
            int x1=x+dx[i],y1=y+dy[i];
            if(!mapa.priechodne(x1,y1) || vzd[x1][y1] != -1)
                continue;
            vzd[x1][y1] = d+1;
            Q.push(x1); Q.push(y1); Q.push(d+1);
        }
    }
}

int hrac_na_utok = 1;

bool cond(pair<pii, pii> a, pair<pii, pii> b) {
    if(a.first.second < b.first.second) return true;
    if(a.first.second == b.first.second) {
        if(a.first.first > b.first.first) return true;
    }
    return false;
}

int pocet_magov = 0;
int bojovnik = TEMNY_CARODEJNIK;
int veze=0;
int laby=0;

int pocet_raptorov = 10;
vector<pii> miesta_raptorov;

int poradie[]= {LAB_ZAJAC, LAB_KORITNACKA, LAB_ZOMBIE, LAB_JEDNOROZEC};

int poradie_obrana[] = {TEMNY_CARODEJNIK, DRAK, TROLL, HYDRA};
int pocet_obrana[] = {4, 4, 4, 2};

int cld[] = {7, 9, 19, 30};

int pocet_druhu = 0;

bool rychla_hra = false;
int ktory_lab = 0;
int ktora_veza = 0;
// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zaciatocnaObrana(){
     if(stav.hraci[0].utocnici.size() == 0)
	return;
     rychla_hra = true;
     laby = 1;
     int start_formacia[] = {0, 0, 0, 0};
     for(int i = 0; i < stav.hraci[0].utocnici.size();i++){
	start_formacia[(int)stav.hraci[0].utocnici[i].typ]++;
     }
     if(start_formacia[0] == stav.hraci[0].utocnici.size()){
	poradie_obrana[0] = HYDRA;
        pocet_obrana[0]=3;
	poradie_obrana[1] = DRAK;
	poradie_obrana[2] = TEMNY_CARODEJNIK;
	poradie_obrana[3] = TROLL;
     }
     else if(start_formacia[0] == 0){
	poradie_obrana[2] = DRAK;
	poradie_obrana[0] = TROLL;
	poradie_obrana[1] = TEMNY_CARODEJNIK;
	poradie_obrana[3] = HYDRA;
     }
     else {
	poradie_obrana[0] = DRAK;
	poradie_obrana[1] = HYDRA;
	poradie_obrana[2] = TEMNY_CARODEJNIK;
	poradie_obrana[3] = TROLL;
     }
}

bool mozem_lab = true;

void zistiTah() {
  if(stav.cas < 60) return;
  if(stav.cas == 60) zaciatocnaObrana();
  if((!rychla_hra || veze>=3) && mozem_lab) {
    //staviam laby
    int mini = 10;
    vector<pii> K;
    For(i,mapa.w) For(j,mapa.h) {
        if(pocet_okolitych_ciest[i][j] < 0) continue;
        if(mini == pocet_okolitych_ciest[i][j]) {
            K.push_back({i,j});
        }
        else if(mini > pocet_okolitych_ciest[i][j]) {
            K.clear();
            K.push_back({i,j});
            mini = pocet_okolitych_ciest[i][j];
        }
    }
    if(K.size() != 0) {
    int kde = rand()%K.size();
    if(vykonaj(Prikaz::buduj(K[kde].first, K[kde].second, poradie[ktory_lab]))) {
        pocet_okolitych_ciest[K[kde].first][K[kde].second] = -2;
        laby++;
        mozem_lab = false;
        ktory_lab = (ktory_lab+1)%4;
    }
    }
  }
  // utok na hraca
  int mini_obrana = 10000;
  for(int i=1; i<stav.hraci.size(); i++) {
      if(stav.hraci[i].umrel) continue;
      int utok=0;
      For(j,stav.hraci[i].veze.size()) {
          if(stav.hraci[i].veze[j].typ == 4)
              utok += 2;
          if(stav.hraci[i].veze[j].typ == 1)
              utok += 5;
          if(stav.hraci[i].veze[j].typ == 0)
              utok += 1;
          if(stav.hraci[i].veze[j].typ == 2)
              utok += 3;
          if(stav.hraci[i].veze[j].typ == 3)
              utok += 7;
      }
      if(utok < mini_obrana) {
          mini_obrana = utok;
          hrac_na_utok = i;
      }
  }
  For(i,4) {
      if(stav.cas % cld[i] == 0) { 
        while(vykonaj(Prikaz::utoc(i, hrac_na_utok)));
      }
  }
  // stavanie obrany
  if (stav.hraci[0].utocnici.size() != 0 && laby*5 > veze) {
    vector<vector<int> > vzd; vzd.resize(mapa.w);
    For(i,mapa.w) vzd[i].resize(mapa.h, -1);
    For(i,mapa.w) For(j,mapa.h)
        if(mapa.zisti(i,j) == CIEL)
            BFS(i,j,vzd);
    For(i,mapa.w) For(j,mapa.h) {
        if(pocet_okolitych_ciest[i][j] < 0) continue;
        vzd[i][j] = 1000000;
        int dx[]={0,0,-1,1};
        int dy[]={1,-1,0,0};
        For(k, 4) {
            int x=i+dx[k], y=j+dy[k];
            if(mapa.priechodne(x,y)) {
                if(vzd[x][y] <= 0) continue;
                vzd[i][j]=min(vzd[i][j], vzd[x][y]);
            }
        }
    }
    while(1) {
        /*if(stav.cas >= 1200 && miesta_raptorov.size() > 0) {
            int len = miesta_raptorov.size();
            K.push_back({{-1, 1000}, {miesta_raptorov[len-1].first, miesta_raptorov[len-1].second}});
        }*/
        vector<pair<pii, pii> > K;
        For(i,mapa.w) For(j,mapa.h) {
            if(pocet_okolitych_ciest[i][j] < 1) continue;
            K.push_back({{pocet_okolitych_ciest[i][j], vzd[i][j]},{i,j}});
        }
        sort(K.begin(), K.end(), cond);
        if(K.size() == 0 && miesta_raptorov.size() > 0) {
            int x=miesta_raptorov[miesta_raptorov.size()-1].first,y=miesta_raptorov[miesta_raptorov.size()-1].second;
            miesta_raptorov.pop_back();
            pocet_okolitych_ciest[x][y]=100;
            K.push_back({{-1, 100}, {x,y}});
            vykonaj(Prikaz::buraj(x,y));
        }
        else if(K.size() == 0) {
            break;
        }
        //sem pridu raptori
        if(pocet_raptorov > 0) {
            if(!vykonaj(Prikaz::buduj(K[0].second.first, K[0].second.second, LASER_RAPTOR)))
                break;
            pocet_okolitych_ciest[K[0].second.first][K[0].second.second] = -2;
            miesta_raptorov.push_back({K[0].second.first, K[0].second.second});
            pocet_raptorov--;
            continue;
        }
        //koniec raptorov
        bojovnik = poradie_obrana[ktora_veza%4];
        if(!vykonaj(Prikaz::buduj(K[0].second.first, K[0].second.second, bojovnik)))
            break;
	pocet_druhu++;
	if (pocet_druhu == pocet_obrana[ktora_veza%4]) {
	    ktora_veza++;
	    pocet_druhu = 0;
	}
        pocet_okolitych_ciest[K[0].second.first][K[0].second.second] = -2;
        veze++;
    }
  }
  if(stav.cas % 150 == 0 || (stav.cas > 500 && stav.hraci[0].utocnici.size() == 0)) {
      mozem_lab = true;
  }
  /*int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
  vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab));
  vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho));
  naKoho = (naKoho + 1) % mapa.pocetHracov;
  if(naKoho == 0) naKoho++;
  if(!vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho)))
      vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab));
  else
      vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, rand()%VEZA_POCET_TYPOV));
  */
  // hmm este nejaku obranu by to chcelo...
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

