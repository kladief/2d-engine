#ifndef ENGINE_H
#define ENGINE_H
#define DEFAULT_TX (wchar_t*)L"defaultTX.bmp"
#include "window-engine/Pixel.h"
#include "helpType.h"
#include <windows.h>
#include <vector>
#include <map>
#include <iostream>
#include <math.h>

#include <wincodec.h>
#include <WTypes.h>
#include <windowsx.h>
#include <commdlg.h>
#include <Objbase.h>
#include <stdio.h>

#include <functional>
#define COLLISION_LEFT 1
#define COLLISION_RIGHT 2
#define COLLISION_TOP 3
#define COLLISION_BOTTOM 4
#define ISMOVE(a,b) (a.X==b.X && a.Y==b.Y) ? false:true
#define toCOORD(a) (COORD){(SHORT)a.x,(SHORT)a.y}
#define OBJ_PLAYER 1

class Scripts{
public:
    class _script{
    private:
        std::function<bool()> func;
        std::function<void(std::wstring)> Delete;
        std::wstring name;
    public:
        _script(){}
        _script(std::function<bool()> FUNC): func(FUNC){}
        void _setDeleteFunc(std::function<void(std::wstring)> _Delete,std::wstring name){
            Delete=_Delete;
            this->name=name;
        }
        bool check();
    };
private:
    void DeleteScript(std::wstring);
    std::map<std::wstring,_script> scripts;
    std::vector<_script*> activeScript;
public:
    void create(std::wstring, std::function<bool()> );
    void create(std::wstring, _script );
    _script* getScript(std::wstring name){
        return &(scripts[name]);
    }
    void setScript(_script* scr){
        activeScript.push_back(scr);
    }
    std::vector<_script*> _getScripts();
};

class Engine{
public:
    struct Object{
    private:
        Engine::Object* collision=nullptr;
        float rotate=0;
        objectCoord coord;
        HBITMAP* tx;
        RECT txBox;
        RECT collisionBox;
        COORD txCenter={-1,-1};
        int lvl=0;
        float _getAngle(double sin,double cos);
        float scale=0;
        class Animation{
        private:
            std::vector<HBITMAP*> textures;
            HBITMAP** tx;
            int txNum=0;
            int tickNum=0;
            int tick=1;
        public:
            void setFrame(HBITMAP* tx){
                textures.push_back(tx);
            }
            void _getTxSizeTickNum(int* TxSize,int *txNum){
                int size=textures.size();
                *TxSize=size;
                *txNum=this->txNum;
            }
            void restart();
            bool interrupts=false;
            Animation(HBITMAP** TX,std::vector<HBITMAP*> TEXURES):tx(TX),textures(TEXURES){}
            void setTickCount(int tick){this->tick=tick;}
            void next();
        };
        Animation* animation=nullptr;
        std::map<std::wstring,Animation*> animations;
    public:
        int collisionSide=0; 
        RECT getCollisionBox(){return (RECT){collisionBox.left+coord.x,collisionBox.top+coord.y,collisionBox.right+coord.x,collisionBox.bottom+coord.y};} 
        void setCollisionBox(RECT collisionBox){this->collisionBox=collisionBox;} 
        void setCollision(Object* collision){this->collision=collision;}
        Object* getCollision(){return collision;}
        bool hard=false;
        void setTxCenter(COORD txCenter){this->txCenter=txCenter;}
        COORD getTxCenter(){return (scale) ? (COORD){(SHORT)(txCenter.X*scale),(SHORT)(txCenter.Y*scale)}:txCenter;}
        float getRotate(){return rotate;}
        void setRotate(float rotate){this->rotate=rotate;}
        void setRotate(double sin,double cos){this->rotate=_getAngle(sin,cos);}
        Scripts::_script CreateAnimation(std::wstring ,std::vector<HBITMAP*>);
        Animation* getAnimation(std::wstring name=L" ");
        bool setAnimation(Animation* );
        void LoadHBM(HBITMAP*,RECT rect={0,0,0,0});
        objectCoord getCoord(){return coord;}
        void setCoord(objectCoord COORD){coord=COORD;}
        HBITMAP* getTx(){return tx;}
        void setTx(HBITMAP* TX){tx=TX;}
        Object(bool HARD=false, objectCoord COORDs={0,0},COORD TXCENTER={-1,-1}):hard(HARD),coord(COORDs),txCenter(TXCENTER){}
        void setRect(RECT rect){this->txBox=rect;}
        void setScale(float scale){this->scale=scale;}
        RECT* setRect(){return &(this->txBox);}
        RECT getRect();
        void setLvL(int lvl){this->lvl=lvl;}
        int getLvL(){return lvl;}
    };
private:
    Pixel pix;
    Object* player;
    std::map<int,std::vector<Object*>*> objects;
    std::map<wchar_t*, HBITMAP*> textures;
    std::vector<char> keyboardInput;
    bool keyRepeat=false;
    HBITMAP loadImage(wchar_t*);
    mouseButton mouseButtons;
    std::vector<HBITMAP*> bakeTextureVector;
    Object* collisionCheck(Object*,std::vector<Engine::Object*>*);
public:
    std::vector<HBITMAP*> bakeTexture(std::vector<HBITMAP*>,std::vector<HBITMAP*>,COORD center={0,0});

    mouseButton getMouseButtons(){return mouseButtons;}
    std::vector<char> getKeyboardInput(){
        return keyboardInput;
    }
    COORD getMousePos();
    HBITMAP* loadTexture(wchar_t*,RECT rect={-1,-1,-1,-1});
    Engine(HINSTANCE);
    void setObject(Object*,int,int objType=0);
    bool render(Scripts* script=nullptr);
    COORD playerMove(COORD coord={-1,-1});
    ~Engine();
};

#endif