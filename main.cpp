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
    Engine::Object obj2(true,{30,30});
    Engine::Object player(true);

    obj2.LoadHBM(eng.loadTexture((wchar_t*)L"grass.bmp",{0,0,96,96}));
    obj2.setRect({0,0,100,100});
    obj1.LoadHBM(eng.loadTexture((wchar_t*)L"test.bmp",{0,0,200,200}));
    obj1.setRect({0,0,200,200});
    
    player.LoadHBM(eng.loadTexture((wchar_t*)L"sprites/knife/move/survivor-move_knife_0.png"));
    player.setRect({0,0,96,96});
    player.setScale(0.5f);

    std::vector<HBITMAP*> knife(19);
    std::vector<HBITMAP*> knife_hit(14);
    std::vector<HBITMAP*> feetWalk(19);
    for(int i=0;i<19;i++){
        std::wstring fileName=L"sprites/feet/walk/survivor-walk_";
        fileName+=std::to_wstring(i);
        fileName+=L".png";
        feetWalk[i]=eng.loadTexture((wchar_t*)(fileName.c_str()));
    }
    for(int i=0;i<19;i++){
        std::wstring fileName=L"sprites/knife/move/survivor-move_knife_";
        fileName+=to_wstring(i);
        fileName+=L".png";
        knife[i]=eng.loadTexture((wchar_t*)(fileName.c_str()));
    }
    for(int i=0;i<14;i++){
        std::wstring fileName=L"sprites/knife/melee/survivor-meleeattack_knife_";
        fileName+=to_wstring(i);
        fileName+=L".png";
        knife_hit[i]=eng.loadTexture((wchar_t*)(fileName.c_str()));
    }
    Scripts scrs;
    player.CreateAnimation(L"knife",knife);
    player.getAnimation(L"knife")->setTickCount(3);
    player.setAnimation(player.getAnimation(L"knife"));

    scrs.create(L"knife",player.CreateAnimation(L"knife_hit",knife_hit));
    player.getAnimation(L"knife_hit")->setTickCount(2);
    player.getAnimation(L"knife_hit")->interrupts=true;

    eng.setObject(&obj1,0);
    eng.setObject(&obj2,1);
    eng.setObject(&player,1,OBJ_PLAYER);

    for(;;){
        if(!eng.render(&scrs))
            return 0;
        COORD mouseCoord=eng.getMousePos();
        std::cout<<"mouse x="<<mouseCoord.X<<std::endl<<"mouse y="<<mouseCoord.Y<<std::endl;

        COORD oldCoord=toCOORD(player.getCoord());
        COORD newCoord=eng.playerMove();

        if(!ISMOVE(oldCoord,newCoord)){
            player.setAnimation(nullptr);
        }
        else{
            player.setAnimation(player.getAnimation(L"knife"));
        }
        if(eng.getMouseButtons().leftButton){
            std::cout<<"LBOTTON"<<std::endl;
            if(player.setAnimation(player.getAnimation(L"knife_hit")))
                scrs.setScript(scrs.getScript(L"knife"));
        }
        newCoord.X+=player.getRect().right/2;
        newCoord.Y+=player.getRect().bottom/2;
        COORD rayCoord=(COORD){(SHORT)(newCoord.X-mouseCoord.X),(SHORT)(newCoord.Y-mouseCoord.Y)};
        float tan=sqrt(pow(rayCoord.X,2)+pow(rayCoord.Y,2));
        float sin=-(float)rayCoord.Y/tan;
        float cos=-(float)rayCoord.X/tan;
        std::cout<<"sin="<<sin<<std::endl<<"cos="<<cos<<std::endl;
        player.setRotate(sin,cos);
        Sleep(1);
    }
    system("pause");
    return 0;
}
