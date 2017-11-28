// very important, much wow, such don't touch this
#define protected public
#define iii int

#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef long double ld;
typedef complex<ll> point;
typedef pair<ll, ll> pll;
typedef vector<ll> vll;
typedef vector<vll> vvll;
typedef vector<bool> vb;

const ll INF=LONG_LONG_MAX;
ll MOD=1000000009;

#define FOR(i, low, high) for(ll i = (low); i < (ll)(high); ++i)
#define MP(x, y) (make_pair((x), (y)))

template<class T> ostream& operator<<(ostream &out, const vector<T> &v) {
    for(unsigned long long i = 0; i < v.size(); ++i)
        out << v[i] << (i<v.size()-1 ? " " : "");
    return out;
}

template<class T1, class T2> ostream& operator<<(ostream &out, const pair<T1, T2> &v) {
    out << "(" << v.first << ", " << v.second << ")";
    return out;
}

ll sign(ll x) {
    return (x > 0) - (x < 0);
}

vector<point> dirs={{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
bool bound(point p, ll r, ll c) {
	return p.real() >= 0 && p.real() < r && p.imag() >= 0 && p.imag() < c;
}

iii main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    srand(time(NULL));

}
