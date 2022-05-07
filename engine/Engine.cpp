#include "Engine.h"

void Engine::Object::LoadHBM(HBITMAP* Tx,RECT rect){
    tx=0;
    if(Tx)
        tx=Tx;
    txBox=rect;
}
HBITMAP* Engine::loadTexture(wchar_t* txName,RECT rect){
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
}
bool Engine::render(){
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
        }
    }
    Exit:

    for(int i=0;;i++){
        if(objects.find(i) != objects.end()){
            for(auto obj=objects[i]->begin(); obj<objects[i]->end() ;obj++){
                if((*obj)->getAnimation()){
                    (*obj)->getAnimation()->next();
                }
                pix.printBitMap(*(*obj)->getTx(),toCOORD((*obj)->getCoord()),(*obj)->getRect());
            }
        }
        else{
            break;
        }
    }
    pix.printBitMap();
    return true;
}
COORD Engine::playerMove(COORD coord){
    if(coord.X==-1 && coord.Y==-1){
        for(auto sym=keyboardInput.begin(); sym<keyboardInput.end() ;sym++){
            std::cout<<*sym<<std::endl;
            switch(*sym){
                case 'a':{
                    objectCoord Coord=player->getCoord();
                    if(Coord.x-1>=0)
                        player->setCoord({Coord.x-1,Coord.y});
                    break;
                }
                case 'd':{
                    objectCoord Coord=player->getCoord();
                    player->setCoord({Coord.x+1,Coord.y});
                    break;
                }
                case 'w':{
                    objectCoord Coord=player->getCoord();
                    if(Coord.y-1>=0)
                    player->setCoord({Coord.x,Coord.y-1});
                    break;
                }
                case 's':{
                    objectCoord Coord=player->getCoord();
                    player->setCoord({Coord.x,Coord.y+1});
                    break;
                }
            }
        }
    }
    else{
        player->setCoord({coord.X,coord.Y});
    }
    std::cout<<"\n\n\n\n\n\n\n\n\n\n\n\n\n";
    return toCOORD(player->getCoord());
}
Engine::Engine(HINSTANCE hInst){
    pix.beginPaint(hInst);
}
void Engine::Object::Animation::next(){
    tickNum++;
    if(tickNum==tick){
        tickNum=0;
        *tx=textures[txNum];
        txNum++;
        if(textures.size()==txNum)
            txNum=0;
    }
}

HBITMAP Engine::loadImage(wchar_t* wcharFilename)
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