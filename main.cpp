#include "engine/Engine.h"
#include <Windows.h>
#include <fcntl.h>
#include <string.h>
#define KNIFE knife

#define LSTR(x) L ## x
#define STRWIDE(x) LSTR(# x)
#define STRWIDE2(x) STRWIDE(x)
#define LowTab(x) _x
#define MOVE(a,b) sprites/a/move/survivor-move_\a\_\b.png
#define GETFILE(x,y,z) STRWIDE2(x(y,z))
BOOL CreateConsole(void)
{
    FreeConsole(); //на всякий случай
    if ( AllocConsole() )
    {
        int hCrt = _open_osfhandle((intptr_t)
        GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
        *stdout = *(::_fdopen(hCrt, "w"));
        ::setvbuf(stdout, NULL, _IONBF, 0);
        *stderr = *(::_fdopen(hCrt, "w"));
        ::setvbuf(stderr, NULL, _IONBF, 0);
        return TRUE;
    }
    return FALSE;
}

int CALLBACK wWinMain(HINSTANCE hInst,HINSTANCE,PWSTR czCmdLine,int nCmdShow){
    CreateConsole();
    using namespace std;
    Engine eng(hInst);
    Engine::Object obj1(true,{10,10});
    Engine::Object player(true);
    HBITMAP* grassTx=eng.loadTexture((wchar_t*)L"grass.bmp",{0,0,96,96});
    HBITMAP* playerTx=eng.loadTexture((wchar_t*)L"sprites/knife/move/survivor-move_knife_0.png",{0,0,96,96});

    obj1.LoadHBM(grassTx);
    player.LoadHBM(playerTx);
    
    std::vector<HBITMAP*> knife(19);
    std::vector<HBITMAP*> feetWalk(19);
    for(int i=0;i<19;i++){
        std::wstring fileName=L"sprites/knife/move/survivor-move_knife_";
        fileName+=to_wstring(i);
        fileName+=L".png";
        knife[i]=eng.loadTexture((wchar_t*)(fileName.c_str()));
    }
    player.CreateAnimation(L"knife",knife);
    player.getAnimation(L"knife")->setTickCount(3);
    player.setAnimation(player.getAnimation(L"knife"));
    player.setRect({0,0,48,48});
    eng.setObject(&obj1,0);
    eng.setObject(&player,1,OBJ_PLAYER);

    for(;;){
        if(!eng.render())
            return 0;
        COORD oldCoord=toCOORD(player.getCoord());
        COORD newCoord=eng.playerMove();
        if(!ISMOVE(oldCoord,newCoord)){
            player.setAnimation(nullptr);
        }
        else{
            player.setAnimation(player.getAnimation(L"knife"));
        }
        Sleep(1);
    }
    system("pause");
    return 0;
}
