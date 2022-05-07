#ifndef ENGINE_H
#define ENGINE_H
#define DEFAULT_TX (wchar_t*)L"defaultTX.bmp"
#include "window-engine/Pixel.h"
#include "helpType.h"
#include <windows.h>
#include <vector>
#include <map>
#include <iostream>

#include <wincodec.h>
#include <WTypes.h>
#include <windowsx.h>
#include <commdlg.h>
#include <Objbase.h>
#include <stdio.h>

#define ISMOVE(a,b) (a.X==b.X && a.Y==b.Y) ? false:true
#define toCOORD(a) (COORD){(SHORT)a.x,(SHORT)a.y}
#define OBJ_PLAYER 1
class Engine{
public:
    struct Object{
    private:
        objectCoord coord;
        HBITMAP* tx;
        bool hard;
        bool entity;
        RECT txBox;
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
            Animation(HBITMAP** TX,std::vector<HBITMAP*> TEXURES):tx(TX),textures(TEXURES){}
            void setTickCount(int tick){this->tick=tick;}
            void next();
        };
        Animation* animation=nullptr;
        std::map<std::wstring,Animation*> animations;
    public:
        void CreateAnimation(std::wstring name,std::vector<HBITMAP*> hBms){
            animations[name]=new Animation(&tx,hBms);
        }
        Animation* getAnimation(std::wstring name=L" "){
            if(name!=L" ")
                return animations[name];
            return animation;
        }
        void setAnimation(Animation* animation){
            this->animation=animation;
        }
        void LoadHBM(HBITMAP*,RECT rect={0,0,0,0});
        objectCoord getCoord(){return coord;}
        void setCoord(objectCoord COORD){coord=COORD;}
        HBITMAP* getTx(){return tx;}
        void setTx(HBITMAP* TX){tx=TX;}
        Object(bool HARD=false,objectCoord COORD={0,0}):hard(HARD),coord(COORD){}
        void setRect(RECT rect){this->txBox=rect;}
        RECT* setRect(){return &(this->txBox);}
        RECT getRect(){return txBox;}
    };
private:
    Pixel pix;
    Object* player;
    std::map<int,std::vector<Object*>*> objects;
    std::map<wchar_t*, HBITMAP*> textures;
    std::vector<char> keyboardInput;
    bool keyRepeat=false;
    HBITMAP loadImage(wchar_t*);
public:
    std::vector<char> getKeyboardInput(){
        return keyboardInput;
    }
    HBITMAP* loadTexture(wchar_t*,RECT rect={-1,-1,-1,-1});
    Engine(HINSTANCE);
    void setObject(Object*,int,int objType=0);
    bool render();
    COORD playerMove(COORD coord={-1,-1});
    ~Engine();
};

#endif