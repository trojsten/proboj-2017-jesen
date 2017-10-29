# proboj-2017-jar - Mad Scientists

## Co treba na rozbehanie proboja?

Nasledujúci návod slúži na spustenie hlavného kompu, ak chcete len kódiť 
klienta, prečítajte si dokumentáciu.

Odporucame rozbehavat proboj na linuxe. Na windowse je to bolestive
a nie zdokumentovane, ale ak to date tak vas to urcite posilni...


Na web je potrebny python2, a kadejake balicky v nom
(flask, markdown) a na grafy treba gnuplot a python3
ostatné scripty používajú bash.

## Ako rozbehat proboj?

O tom, ako rozbehat server a observer je napisane v subore
`dokumentacia.md`. (Pristupny aj cez web, ked je rozbehany webserver.)
Dalej popiseme, ako rozbehat webserver (potrebny na submitovanie).

Spustit webserver si viete pomocou bash-scriptu `./runweb` ktory
spustite z korena proboja. (Teda v `./proboj-2017-jar/` spustite
`./runweb`.)

Ked vam uz bezi web, tak ho viete navstivit na `0.0.0.0:5000`

Ak sa chcete zaregistrovat (co je nutne na to, aby ste mohli
submitovat botov), vojdite do priecinku `registracia` a spustite
bash-script `./register`. Dalej postupujte podla jeho pokynov.

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

## O vyhodnoteni

Vyhodnotenie prebehlo dvoma spôsobmy. Štandardne (každý proti každému) a turnaj 1v1 (double elimination).
Turnaj sa hral na mapách kebak, logo a isle, každý duel na všetkých troch.
![](proboj-2017-jar-turnaj.png)

Zápasi z finále turnaja sú v priečinku záznamy.
Výsledky štandardného finále (10% priebežné body + vyhodnotenie):



## Nejake obrazky na zaver

![](proboja.png)

![](probojb.png)

![](probojc.png)

![](probojd.png)
