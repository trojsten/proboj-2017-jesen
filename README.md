# proboj-2017-jesen Tower defense-offense

## Co treba na rozbehanie proboja?

Nasledujúci návod slúži na spustenie hlavného kompu, ak chcete len kódiť 
klienta, prečítajte si dokumentáciu.

Odporucame rozbehavat proboj na linuxe. Na windowse je to bolestive
a nie zdokumentovane, ale ak to date tak vas to urcite posilni...


Na web je potrebny python2, a kadejake balicky v nom
(flask, markdown) ostatné scripty používajú bash.
Observer je napísaný v javascripte, takže vám stačí ľubovolný prehliadač a
nejaký python ktorý bude servovať súbory pre neho.

## Ako rozbehat proboj?

O tom, ako rozbehat server a observer je napisane v subore
`dokumentacia.md`. (Pristupny aj cez web, ked je rozbehany webserver.)
Dalej popiseme, ako rozbehat webserver (potrebny na submitovanie).

Spustit webserver si viete pomocou bash-scriptu `./runweb` ktory
spustite z korena proboja. (Teda v `./proboj-2017-jesen/` spustite
`./runweb`.)

Ked vam uz bezi web, tak ho viete navstivit na `localhost:5000`

Ak sa chcete zaregistrovat (co je nutne na to, aby ste mohli
submitovat botov), vojdite do priecinku `registracia` a spustite
bash-script `./register`. Dalej postupujte podla jeho pokynov.
Bude pýtať login a názov. Login je na prihlasovanie na stránku, 
ale používa sa aj na iných miestach, mal by byť alfanumerický.

## Ako urobit, aby sa zapasy sami generovali a zobrazovali?

Vojdete do priecinka `hlavnykomp` a v nom sa nachadzaju skripty

*   `pustaj-plamen`, ktory automaticky skompiluje vase submity, ked
ich odoslete cez webserver. Takisto do terminalu vypisuje pripadne
chybove hlasky pri kompilovani.

*   `pustaj-server` --- automaticke generovanie zapasov.

*   `pustaj-observer` --- automaticky zobrazuje zapasy.

## A co, ked nieco nefunguje?

Tak to treba opravit. Konkretne pokyny bohuzial neexistuju,
takze si asi trochu poplacete, ale co vas nezabije, to vas posilni.
Ak si nebudete vedieť poradiť, napíšte mi.

## O vyhodnoteni

Vyhodnotenie sa počítalo dvoma spôsobmi. Hlavná časť vyhodnotenia sú zápasy
náhodne generovaných osmíc na náhodnej mape. Menšia časť bodov je tvorená 
zápasmi 1v1 v podobe každý s každým, ktoré sa počítali na viacerych mapách.
Tito zápasy boli väčšinou vyrovnané, niektorý klienti na niektorých mapách mali 
vyditeľnú výhodu.

Priebežné body mali váhu 1/3, body zo zápasov 1v1 na 5 mapách (sčítané) mali 
váhu 8/3 a body z vyhodnotenia 5/7. Jednotlivé body sú v nejakom formáte v 
tabuľke `vyhodnotenie.ods`.

Vygenerovalo sa pri tom niekolko GB záznamov, finálna výsledkovka vyzerala takto:

1. baja	994537
2. nieco	971885
3. Nulano	962203
4. Meky Zbirka rules the world	944846
5. pecene jednorozce	939185
6. U Šel Not Pas	935756
7. Magický Puding	935186
8. paulinia	930787
9. Za nepovinný proboj!!!	928956
10. DJ Anko Bin Jadin	886336
11. misko	868673
12. PFU	841020
13. dominik	831040
14. MArShmalLOv	828922
15. KEbab	826124
16. Timka a Viki sa tesia z proboja	732848
17. *****Ponorka	695564

Zloženie jednotlivých týmov:
baja (Baška, Aja)
nieco ()
Nulano (Ondro)
Meky Zbirka rules the world ()
pecene jednorozce ()
U Šel Not Pas ()
Magický Puding ()
paulinia (Paulínka)
Za nepovinný proboj!!! (Jitka, Gabika, (Prefix))
DJ Anko Bin Jadin (Jano, Medveď)
misko (Mišo S.)
PFU ()
dominik (Jožo Č., Jaro P.)
MArShmalLOv ()
KEbab ()
Timka a Viki sa tesia z proboja (Timka, Viki, (Samo))
*****Ponorka ()


## Nejake obrazky na zaver

![](proboja.png)

![](probojb.png)

![](probojc.png)

![](probojd.png)
