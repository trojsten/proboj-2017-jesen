//interface pre komunikáciu s klientami (universal proboj magic)
#include <cstdio>
#include <sstream>
using namespace std;

#include "marshal.h"
#include "common.h"
// vid streamtest.cpp
static int safepeek(istream& in) {
  if (in.eof()) return EOF;
  if (in.fail()) return EOF;
  return in.peek();
}

// ak na vstupe najde sentinel, tak ho nacita a vrati true, inak nenacita
// nic (okrem whitespacu) a vrati false. sentinel moze byt znak alebo EOF.
// fcia by nikdy nemala zmenit failbit.
bool skusNacitatSentinel(istream& in, int sentinel) {
  while (1) {   // precitame whitespace
    int c = safepeek(in);
    if (!(c >= 0 && c <= ' ')) break;
    in.ignore(1);
  }
  if (safepeek(in) == sentinel) {
    if (sentinel != EOF) in.ignore(1);   // precitame sentinel
    return true;
  }
  else return false;   // ak sme nenasli sentinel, precitame len whitespace
}


template<> void uloz<int>(ostream& buf, const int& in) {
    buf << in << '\n';
}

template<> void nacitaj<int>(istream& buf, int& out) {
    buf >> out;
}

template<> void uloz<bool>(ostream& buf, const bool& in) {
    buf << in << '\n';
}

template<> void nacitaj<bool>(istream& buf, bool& out) {
    buf >> out;
}

// uloz() pre enumy z common.h
#define reflectenum(T) template<> void uloz<T> (ostream& out, const T& in) { uloz(out,  (const int &)in);}
#include "common.h"
#undef reflectenum

// nacitaj() pre enumy z common.h
#define reflectenum(T) template<> void nacitaj<T> (istream& in, T& out) { nacitaj(in, (int &)out); }
#include "common.h"
#undef reflectenum

// uloz() pre struktury z common.h
#define reflection(T) template<> void uloz<T> (ostream& out, const T& in) {
#define member(x) uloz(out, in.x);
#define end() }
#include "common.h"
#undef reflection
#undef member
#undef end

// nacitaj() pre struktury z common.h
#define reflection(T) template<> void nacitaj<T> (istream& in, T& out) {
#define member(x) nacitaj(in, out.x);
#define end() }
#include "common.h"
#undef reflection
#undef member
#undef end
