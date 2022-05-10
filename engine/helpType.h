struct objectCoord{
    int x;
    int y;
    int z;
    objectCoord(){};
    objectCoord(int X,int Y,int Z=0) : x(X),y(Y),z(Z){};
    void operator=(objectCoord a){
        x=a.x;
        y=a.y;
        z=a.z;
    }
};
struct mouseButton{
    bool leftButton=false;
    bool rightButton=false;
};