#include "update.h"
#include <iostream>
#include <algorithm>

using namespace std;



game_state update_game_state(mapa gm, game_state gs, vector<instruction> commands) {
    game_state new_gs = gs;
    new_gs.round++;
    cerr<<"update na kolo "<<new_gs.round<<endl;
    sort(commands.begin(), commands.end());
    
    
    cerr<<"hotovo"<<endl;
    return new_gs;
}
