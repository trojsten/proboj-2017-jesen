#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

using namespace std;

#include "common.h"
#include "marshal.h"
#include "update.h"

static int DX[] = { 0, 1, 0, -1 };
static int DY[] = { -1, 0, 1, 0 };

Mapa mapa;
Stav stav;   // vzdy som hrac cislo 0
vector<Prikaz> prikazy;
int naKoho;
int q = 0;

struct Point{
	int x;
	int y;
	int near = 0;
};

vector <Point> obranne;
vector <Point> utocne;
vector <Point> obr;
vector <int> distt;
vector <int> dist;

bool postavil_som = false;
int typ_veze;
int stavaj_na;
int pocet_postavenych = 0;

int akoBudemUtocit;


// pomocna funkcia. volajte zo zistiTah(), napr.:
// if (!vykonaj(Prikaz::stavaj(10, 10, VEZA_LASER))) { /* zeby malo dreva? */ }
bool vykonaj(const Prikaz& p) {
	bool uspech = vykonajPrikaz(mapa, stav, 0, p);
	if (uspech) prikazy.push_back(p);   // potom to posleme serveru
	return uspech;
}

static void Bfs(const Mapa& mapa, vector<int>& dist, int typ) {
  dist.clear();
  dist.resize(mapa.w * mapa.h, mapa.w * mapa.h * 2);
  queue<int> Q;
  for (int y = 0; y < mapa.h; y++){
        for (int x = 0; x < mapa.w; x++) {
            if (mapa.zisti(x, y) == typ) {
                Q.push(y * mapa.w + x);
                dist[y * mapa.w + x] = 0;
            }
        }
	}
	while (!Q.empty()) {
		int p = Q.front(); Q.pop();
		int y = p / mapa.w, x = p % mapa.w;
		for (int d = 0; d < 4; d++) {
			int ny = y + DY[d], nx = x + DX[d];
			int np = ny * mapa.w + nx, nd = dist[p] + 1;
			if (mapa.zisti(nx, ny) == POZEMOK && typ == CIEL) obr.push_back({nx,ny});
			if (!mapa.priechodne(nx, ny)) continue;
			if (dist[np] <= nd) continue;
			if (typ == SPAWN && distt[ny*mapa.w + nx] > distt[y*mapa.w + x]) continue;
			dist[np] = nd;
			Q.push(np);
		}
	}
}

// main() zavola tuto funkciu, ked nacita mapu
void inicializuj() {
	srand(time(NULL) * getpid());
	Bfs(mapa, distt, CIEL);
	Bfs(mapa, dist, SPAWN);
	for(int i = 0; i < mapa.w; i++){
		for(int j = 0; j < mapa.h; j++){
			cerr << mapa.zisti(i,j) << '\t';
		}
		cerr << '\n';
	}
	cerr << '\n';
	for(int i = 0; i < mapa.w; i++){
		for(int j = 0; j < mapa.h; j++){
			cerr << dist[j*mapa.w + i] << '\t';
		}
		cerr << '\n';
	}
	cerr << '\n';
	for(int i = 0; i < mapa.w; i++){
		for(int j = 0; j < mapa.h; j++){
			cerr << distt[j*mapa.w + i] << '\t';
		}
		cerr << '\n';
	}
	cerr << '\n';
	for(int i = 0; i < mapa.w; i++){
		for(int j = 0; j < mapa.h; j++){
			if(mapa.zisti(i, j) == POZEMOK){
				if(i == 0 && j == 0) {
					if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(i == mapa.w-1 && j == 0) {
					if(dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(i == 0 && j == mapa.h-1) {
					if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(i == mapa.w-1 && j == mapa.h-1) {
					if(dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(i == 0){
					if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(i == mapa.w-1){
					if(dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(j == 0){
					if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2 ){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(j == mapa.h-1){
					if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
						obranne.push_back({i,j});
					} else{
						utocne.push_back({i,j});
					}
				} else if(dist[j*mapa.w + i + 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - 1] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i + mapa.w] != mapa.w * mapa.h * 2 || dist[j*mapa.w + i - mapa.w] != mapa.w * mapa.h * 2){
					obranne.push_back({i,j});
				} else{
					utocne.push_back({i,j});
				}
			}
		}
	}
	typ_veze = rand()%VEZA_POCET_BOJOVYCH;
	stavaj_na = rand()%obranne.size();
	pocet_postavenych = 0;
	postavil_som = false;
	int obrane = false;
	for(int i = 0; i < mapa.w; i++){
		for(int j = 0; j < mapa.h; j++){
			obrane = false;
			if(mapa.zisti(i,j) == POZEMOK) {
				for(int k = 0; k < obranne.size(); k++) {
					if(obranne[k].x == i && obranne[k].y == j){
						cerr << 'o' << '\t';
						obrane = true;
						break;
					}
				}
				if(!obrane) cerr << 'u' << '\t';
			} else cerr << 'x' << '\t';
		}
		cerr << '\n';
	}
}


// main() zavola tuto funkciu, ked chce vediet, ake prikazy chceme vykonat
void zistiTah() {
	/*int mojLab = VEZA_LAB_PRVY + akoBudemUtocit;
	if(rand()%4)
		vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, mojLab));
	else
		vykonaj(Prikaz::buduj(rand() % mapa.w, rand() % mapa.h, rand()%VEZA_LAB_PRVY));
	int naKoho = 1 + rand() % (mapa.pocetHracov-1);
	vykonaj(Prikaz::utoc(akoBudemUtocit, naKoho));*/
	// hmm este nejaku obranu by to chcelo...

	if(stav.cas % 100 == 0) naKoho = 1 + rand() % (mapa.pocetHracov-1);
	if(postavil_som){
		pocet_postavenych++;
		if(typ_veze < VEZA_POCET_BOJOVYCH || utocne.empty()) obranne.erase(obranne.begin()+stavaj_na);
		else utocne.erase(utocne.begin()+stavaj_na);
		if(pocet_postavenych < 5){
			typ_veze = 4;
		} else if(pocet_postavenych == 5){
			typ_veze = 5;
		} else if(stav.cas < 700){
			typ_veze = rand()%VEZA_POCET_BOJOVYCH;
		} else {
			typ_veze = rand()%VEZA_POCET_TYPOV;
		}
		if(pocet_postavenych < 5) {
			stavaj_na = stavaj_na = rand()%obranne.size();
			int bolo_to = -1;
			for(; bolo_to == -1 && q < obr.size(); q++){
				for(int k = 0; k < obranne.size(); k++) {
					if(obranne[k].x == obr[q].x && obranne[k].y == obr[q].y){
						stavaj_na = k;
						bolo_to = k;
						break;
					}
				}
			}
		} else if(typ_veze < VEZA_POCET_BOJOVYCH || utocne.empty())stavaj_na = rand()%obranne.size();
		else stavaj_na = rand()%utocne.size();
	}

	if(typ_veze < VEZA_POCET_BOJOVYCH || utocne.empty()) postavil_som = vykonaj(Prikaz::buduj(obranne[stavaj_na].x, obranne[stavaj_na].y, typ_veze));
	else postavil_som = vykonaj(Prikaz::buduj(utocne[stavaj_na].x, utocne[stavaj_na].y, typ_veze));
	for(int i = 0;  i < 4; i++) if(vykonaj(Prikaz::utoc(i, naKoho))) naKoho = (naKoho+1)%mapa.pocetHracov;
	if(naKoho == 0) naKoho ++;
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

