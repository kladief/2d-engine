#include "Engine.h"


float Engine::Object::_getAngle(double sin,double cos){// получаем угол из sin и cos
    int ang=acos(cos)*180/M_PI;
    if(sin<0 && cos<0){
        ang=180+abs(asin(sin)*180/M_PI);
    }
    else if(cos>0 && sin<0){
        ang=270+asin(cos)*180/M_PI;
    }
    return (float)ang/(float)360;
}
std::vector<Scripts::_script*> Scripts::_getScripts(){// возвращаем вектор со всеми скриптами
    return activeScript;
}
COORD Engine::getMousePos(){// позиция мыши в окне
    POINT pos;
    GetCursorPos(&pos);
    RECT wndRect;
    GetWindowRect(pix.getWnd(),&wndRect);
    if(pos.x<wndRect.left || pos.y<wndRect.top || pos.x>wndRect.right || pos.y>wndRect.bottom){
        return {0,0};
    }
    return (COORD){(SHORT)(pos.x-wndRect.left) , (SHORT)(pos.y-wndRect.top)};
}
bool Scripts::_script::check(){
    if(func()){
        Delete(name);
        return true;
    }
    return false;
}
void Scripts::create(std::wstring name, std::function<bool()> func){// создаем новый скрипт
    _script scr(func);
    scr._setDeleteFunc([this](std::wstring name){
        DeleteScript(name);
    },name);
    scripts[name]=scr;
}
void Scripts::create(std::wstring name, Scripts::_script script){
    script._setDeleteFunc([this](std::wstring name){
        DeleteScript(name);
    },name);
    scripts[name]=script;
}
void Scripts::DeleteScript(std::wstring name){
    for(int i=0;i<activeScript.size();i++){
        if(activeScript[i]==&(scripts[name])){
            activeScript.erase((activeScript.begin()+i));
        }
    }
}
void Engine::Object::LoadHBM(HBITMAP* Tx,RECT rect){// загружаем текстуру и ограничивающий прямоугольник в обьект
    tx=0;
    if(Tx)
        tx=Tx;
    txBox=rect;
}
HBITMAP* Engine::loadTexture(wchar_t* txName,RECT rect){ // загружаем текстуру из файла
    if(textures.find(txName)==textures.end()){
        HBITMAP* tx=new HBITMAP;
        *tx=loadImage(txName);
        textures[txName]=tx;
        if(!*tx){
            if(textures.find(DEFAULT_TX)==textures.end()){
                HBITMAP* tx=new HBITMAP;
                *tx=loadImage(DEFAULT_TX);
                textures[DEFAULT_TX]=tx;
            }
            textures[txName]=textures[DEFAULT_TX];
        }
    }
    if(rect.bottom==-1 && rect.left==-1 && rect.right==-1 && rect.top==-1)
        return textures[txName];
    HDC hDCWnd=GetDC(0);
    HDC hDCMain=CreateCompatibleDC(hDCWnd);
    HDC hDCTemp=CreateCompatibleDC(hDCMain);
    HBITMAP* txNew=new HBITMAP;
    *txNew=CreateCompatibleBitmap(hDCWnd,rect.right,rect.bottom);
    SelectObject(hDCTemp,*txNew);
    SelectObject(hDCMain,*(textures[txName]));
    BitBlt(hDCTemp,0,0,rect.right,rect.bottom,hDCMain,rect.left,rect.top,SRCCOPY);
    DeleteDC(hDCTemp);
    DeleteDC(hDCMain);
    DeleteDC(hDCWnd);
    return txNew;
}
void Engine::setObject(Engine::Object* obj,int lvl,int objType){
    obj->setLvL(lvl);
    if(objects.find(lvl)!=objects.end()){
        objects[lvl]->push_back(obj);
    }
    else{
        objects[lvl]=new std::vector<Object*>;
        objects[lvl]->push_back(obj);
    }
    if(objType==OBJ_PLAYER)
        player=obj;
}
Engine::~Engine(){
    for(auto ObjVector=objects.begin(); ObjVector!=objects.end() ;ObjVector++){
        delete (ObjVector->second);
    }
    for(auto bakeVector=bakeTextureVector.begin(); bakeVector!=bakeTextureVector.end() ;bakeVector++){
        if(**bakeVector)
            delete *bakeVector;
    }
}
bool Engine::render(Scripts* script){ // основная функция, вней :происходит рендер, обработка сообщений окна, обработка скриптов
    for(;;){
        if(!pix.process()){
            return false;
        }
        switch(pix.getMsg().message){
            case 0:
                goto Exit;
                break;
            case WM_KEYDOWN:{
                DWORD dwScanCode = ( pix.getMsg().lParam >> 16 ) & 0xFF;
                char lpKeyState[256];
                ZeroMemory(lpKeyState,256);
                char input[2];
                int symNum=ToAsciiEx(pix.getMsg().wParam,dwScanCode,(BYTE*)lpKeyState,(WORD*)input,0,GetKeyboardLayout(0));
                if(symNum==1){
                    if(!keyRepeat){
                        bool insert=true;
                        for(auto sym=keyboardInput.begin(); sym!=keyboardInput.end() ;sym++){
                            if(*sym==input[0]){
                                insert=false;
                            }
                        }
                        if(insert)
                            keyboardInput.push_back(input[0]);
                    }
                    else
                        keyboardInput.push_back(input[0]);
                }
                break;
            }
            case WM_KEYUP:{
                DWORD dwScanCode = ( pix.getMsg().lParam >> 16 ) & 0xFF;
                char lpKeyState[256];
                ZeroMemory(lpKeyState,256);
                char input[2];
                int symNum=ToAsciiEx(pix.getMsg().wParam,dwScanCode,(BYTE*)lpKeyState,(WORD*)input,0,GetKeyboardLayout(0));
                keyboardInput.insert(keyboardInput.begin(), 0);
                if(symNum==1){
                    for(auto sym=keyboardInput.begin(); sym!=keyboardInput.end() ;sym++){
                        if(*sym==input[0]){
                            keyboardInput.erase(sym);
                            sym=keyboardInput.begin();
                        }
                    }
                }
                keyboardInput.erase(keyboardInput.begin());
                break;
            }
            case WM_LBUTTONDOWN:
                mouseButtons.leftButton=true;
                break;
            case WM_RBUTTONDOWN:
                mouseButtons.rightButton=true;
                break;
            case WM_LBUTTONUP:
                mouseButtons.leftButton=false;
                break;
            case WM_RBUTTONUP:
                mouseButtons.rightButton=false;
                break;
            
        }
    }
    Exit:

    for(int i=0;;i++){
        std::vector<Scripts::_script *> scripts = script->_getScripts();
        for(auto scr=scripts.begin();scr < scripts.end();scr++){
            (*scr)->check();
        }
        std::cout<<"rotate="<<player->getRotate()<<std::endl;
        if(objects.find(i) != objects.end()){
            for(auto obj=objects[i]->begin(); obj<objects[i]->end() ;obj++){
                if((*obj)->getAnimation()){
                    (*obj)->getAnimation()->next();
                }
                pix.printBitMap(*(*obj)->getTx(),toCOORD((*obj)->getCoord()),(*obj)->getRect(),true,(*obj)->getRotate(),(*obj)->getTxCenter());
            }
        }
        else{
            break;
        }
    }
    pix.printBitMap();
    return true;
}
Engine::Object* Engine::collisionCheck(Engine::Object* obj_toCheck,std::vector<Engine::Object*>* objects){
    RECT obj_toCheck_Box=obj_toCheck->getCollisionBox();
    obj_toCheck->collisionSide=0;
    COORD points[4];
    points[0]={(SHORT)obj_toCheck_Box.left,(SHORT)obj_toCheck_Box.top};
    points[1]={(SHORT)obj_toCheck_Box.right,(SHORT)obj_toCheck_Box.top};
    points[2]={(SHORT)obj_toCheck_Box.right,(SHORT)obj_toCheck_Box.bottom};
    points[3]={(SHORT)obj_toCheck_Box.left,(SHORT)obj_toCheck_Box.bottom};
    for(auto obj=objects->begin(); obj!=objects->end() ;obj++){
        RECT obj_Box=(*obj)->getCollisionBox();
        bool p1=(points[0].X>obj_Box.left && points[0].X<obj_Box.right && points[0].Y>obj_Box.top && points[0].Y<obj_Box.bottom);
        bool p2=(points[1].X>obj_Box.left && points[1].X<obj_Box.right && points[1].Y>obj_Box.top && points[1].Y<obj_Box.bottom);
        bool p3=(points[2].X>obj_Box.left && points[2].X<obj_Box.right && points[2].Y>obj_Box.top && points[2].Y<obj_Box.bottom);
        bool p4=(points[3].X>obj_Box.left && points[3].X<obj_Box.right && points[3].Y>obj_Box.top && points[3].Y<obj_Box.bottom);
        if(p1||p2||p3||p4){
            obj_toCheck->collisionSide=0;
            if(p1 && p2)
                obj_toCheck->collisionSide=COLLISION_TOP;
            if(p2 && p3)
                obj_toCheck->collisionSide=COLLISION_RIGHT;
            if(p3 && p4)
                obj_toCheck->collisionSide=COLLISION_BOTTOM;
            if(p4 && p1)
                obj_toCheck->collisionSide=COLLISION_LEFT;

            if(!obj_toCheck->collisionSide){
                if(p1){
                    COORD p_normalize={(SHORT)( obj_Box.right-obj_toCheck_Box.left),(SHORT)( obj_Box.bottom-obj_toCheck_Box.top)};
                    if(p_normalize.X>p_normalize.Y){
                        obj_toCheck->collisionSide=COLLISION_TOP;
                    }
                    else{
                        obj_toCheck->collisionSide=COLLISION_LEFT;
                    }
                }
                if(p2){
                    COORD p_normalize={(SHORT)( obj_toCheck_Box.right-obj_Box.left),(SHORT)( obj_Box.bottom-obj_toCheck_Box.top)};
                    if(p_normalize.X>p_normalize.Y){
                        obj_toCheck->collisionSide=COLLISION_TOP;
                    }
                    else{
                        obj_toCheck->collisionSide=COLLISION_RIGHT;
                    }
                }
                if(p3){
                    COORD p_normalize={(SHORT)( obj_toCheck_Box.right-obj_Box.left),(SHORT)( obj_toCheck_Box.bottom-obj_Box.top)};
                    if(p_normalize.X>p_normalize.Y){
                        obj_toCheck->collisionSide=COLLISION_BOTTOM;
                    }
                    else{
                        obj_toCheck->collisionSide=COLLISION_RIGHT;
                    }
                }
                if(p4){
                    COORD p_normalize={(SHORT)( obj_Box.right-obj_toCheck_Box.left),(SHORT)( obj_toCheck_Box.bottom-obj_Box.top)};
                    if(p_normalize.X>p_normalize.Y){
                        obj_toCheck->collisionSide=COLLISION_BOTTOM;
                    }
                    else{
                        obj_toCheck->collisionSide=COLLISION_LEFT;
                    }
                }
            }
            return *obj;
        }
    }
    return nullptr;
}
COORD Engine::playerMove(COORD coord){
    objectCoord oldCoord=player->getCoord();
    if(coord.X==-1 && coord.Y==-1){
        for(auto sym=keyboardInput.begin(); sym<keyboardInput.end() ;sym++){
            std::cout<<*sym<<std::endl;
            objectCoord Coord=player->getCoord();
            switch(*sym){
                case 'a':{
                    if(Coord.x-1>=0)
                        player->setCoord({Coord.x-1,Coord.y});
                    break;
                }
                case 'd':{
                    player->setCoord({Coord.x+1,Coord.y});
                    break;
                }
                case 'w':{
                    if(Coord.y-1>=0)
                    player->setCoord({Coord.x,Coord.y-1});
                    break;
                }
                case 's':{
                    player->setCoord({Coord.x,Coord.y+1});
                    break;
                }
            }
        }
    }
    else{
        player->setCoord({coord.X,coord.Y});
    }
    Engine::Object *obj=collisionCheck(player,objects[player->getLvL()]);
    if(obj){
        objectCoord newCoord=player->getCoord();
        if(player->collisionSide==COLLISION_LEFT)
            player->setCoord({obj->getCollisionBox().right - (player->getCollisionBox().left - player->getCoord().x),newCoord.y});
        if(player->collisionSide==COLLISION_RIGHT)
            player->setCoord({obj->getCoord().x-(player->getCollisionBox().right-player->getCoord().x),newCoord.y});
        if(player->collisionSide==COLLISION_TOP)
            player->setCoord({newCoord.x,obj->getCollisionBox().bottom -(player->getCollisionBox().top - player->getCoord().y)});
        if(player->collisionSide==COLLISION_BOTTOM)
            player->setCoord({newCoord.x,obj->getCollisionBox().top - (player->getCollisionBox().bottom - player->getCoord().y)});


    }
    {
        RECT playerBox=player->getCollisionBox();
        HDC hdcMain=GetDC(pix.getWnd());
        MoveToEx(hdcMain,playerBox.left,playerBox.top,nullptr);
        LineTo(hdcMain,playerBox.right,playerBox.top);
        LineTo(hdcMain,playerBox.right,playerBox.bottom);
        LineTo(hdcMain,playerBox.left,playerBox.bottom);
        LineTo(hdcMain,playerBox.left,playerBox.top);
        DeleteDC(hdcMain);
    }   
    std::cout<<"\n\n\n\n\n\n\n\n\n\n\n\n\n";
    return toCOORD(player->getCoord());
}
Engine::Engine(HINSTANCE hInst){
    pix.beginPaint(hInst);
}
void Engine::Object::Animation::next(){// загружаем в указатель на текстуру обьекта следующий кадр анимации
    tickNum++;
    if(tickNum==tick){
        if(textures.size()==txNum){
            txNum=0;
        }
        tickNum=0;
        *tx=textures[txNum];
        txNum++;
    }
}

Scripts::_script Engine::Object::CreateAnimation(std::wstring name,std::vector<HBITMAP*> hBms){
    animations[name]=new Animation(&tx,hBms);
    Engine::Object::Animation* animatonPtr=animations[name];
    HBITMAP* Tx=tx;
    Scripts::_script script([this,animatonPtr,Tx]()->bool{
        int txSize=0;
        int txNum=0;
        animatonPtr->_getTxSizeTickNum(&txSize,&txNum);
        if(txSize==txNum){
            (this->animation)->restart();
            this->animation=nullptr;
            tx=Tx;
            return true;
        }
        return false;
    });
    return script;
}
Engine::Object::Animation* Engine::Object::getAnimation(std::wstring name){
    if(name!=L" ")
        return animations[name];
    return animation;
}

bool Engine::Object::setAnimation(Animation* animation){
    if((this->animation)){
        if(!(this->animation->interrupts)){
            this->animation=animation;
        }
        else
            return false;
    }
    else{
        this->animation=animation;
    }
    return true;
}

RECT Engine::Object::getRect(){
    if(!scale){
        return txBox;
    }
    BITMAP bm;
    GetObject(*tx,sizeof(bm),&bm);
    RECT rectToReturn={0,0,(SHORT)(bm.bmWidth*scale),(SHORT)(bm.bmHeight*scale)};
    return rectToReturn;
}
void Engine::Object::Animation::restart(){
    txNum=0;
    tickNum=0;
    tick=1;
}
std::vector<HBITMAP*> Engine::bakeTexture(std::vector<HBITMAP*> front,std::vector<HBITMAP*> behind,COORD center){
    std::vector<HBITMAP*> bakeTX(front.size(),new HBITMAP);
    BITMAP bm;
    GetObject(front[0],sizeof(bm),&bm);
    COORD txSize={(SHORT)bm.bmWidth,(SHORT)bm.bmHeight};
    auto pBehind=behind.begin();
    for(int i=0; i!=bakeTX.size();i++){
        HBITMAP* tx=new HBITMAP;
        *tx = pix.bakeHBITMAPs(*(front[i]),**pBehind,center);

        bakeTX[i]=tx;
        bakeTextureVector.push_back(tx);

        pBehind++;
        if(pBehind==behind.end())
            pBehind=behind.begin();
    }
    return bakeTX;
}
HBITMAP Engine::loadImage(wchar_t* wcharFilename)// загружаем текстуру из файла
{
	IWICBitmapDecoder *decoder = NULL;
    IWICImagingFactory* factory;
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, reinterpret_cast<void **>(&factory));
	HRESULT result = factory->CreateDecoderFromFilename(wcharFilename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder);

	if (FAILED(result))
	{
		return NULL;
	}

	IWICBitmapFrameDecode *frame = NULL;
	if (FAILED(decoder->GetFrame(0, &frame)))
	{
        return NULL;
	}

	IWICBitmapSource *bitmapSource = NULL;
	if (FAILED(WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &bitmapSource)))
	{
		return NULL;
	}

	if (FAILED(WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, frame, &bitmapSource)))
	{
		return NULL;
	}

	UINT width = 0;
	UINT height = 0;
	if (FAILED(bitmapSource->GetSize(&width, &height)) || width == 0 || height == 0)
	{
		return NULL;
	}

	std::vector<BYTE> buffer(width * height * 4);
	if (FAILED(bitmapSource->CopyPixels(NULL, width * 4, buffer.size(), &buffer[0])))
	{
		return NULL;
	}

	bitmapSource->Release();

	HBITMAP bitmap = CreateBitmap(width, height, 1, 32, &buffer[0]);
	frame->Release();

	decoder->Release();

	return bitmap;
}
