#include "C:\Users\Thomas\AnTh\A0_Appl\S00_Programming\C++Misc\Thomas C++ Libraries\ClassGLobject.hpp"

//All global variables here
struct TypeGlobal
{
    TypeGLwindow* W1;
    TypeGLscene* S1;

    bool   IsWireFrame   =false;
    bool   IsVisible     =true;
    bool   Explode       =false;
    bool   InMotion      =false;
    int    rotationPlane =0;
    float  rotDeltaR     =0.4;
    float  rotCummulative=0.0;
    //TODO: Cube-planes rotations and mouse hot-select
    //TODO: Cube sticker textures
    //TODO: Text overlay

    //mouse
    TypeXYZ MouseFirstPick;
    bool IsLeftButtonPressed=false;
    bool IsRightButtonPressed=false;
    //keyboard
    int ModKeyPressed=0;
}globalV;


//Function declarations-----------------------------------------------------------
void DoDrawThis();
void DoDraw2DOverlay();
//void DoConstruct(int GLshape, TypeGeometry& ThisShape);
void DoInitObjects();
void DoInitLight();
void DoInitCamera();
void DoInitPopupMenu ();
void SetupInitialValues();
void DoWhileWaiting();
void DoCubeAppearance();
void DoWhenKeyPress(unsigned char ThisKey, int x, int y);
void DoWhenMousePress(int ThisButton, int IsPressed, int x, int y);
void DoWhenMouseDrag(int x, int y);
void DoWhenSpecialKeyPress (int ThisKey, int x, int y);
void DoConfineCursor(int& x, int& y);
void DoResize(int NewWidth, int NewHeight);
void DoWithPopupChoice (int value);
void DoShowGrid ();
void DoResetCube ();
void PrintText(std::string ThisText);
void DoRotate (TypeGLObject& CurrOBJ);
bool DoExpandContract (TypeGLObject& CurrOBJ, double currTime);
TypeXYZ DoConvertFromWindowToModel (int WinX, int WinY);

//Main function-------------------------------------------------------------------
int main (int argc, char* argv[])
{
    TypeGLwindow* window1=new TypeGLwindow(argc,argv,"My Window",1000,600); globalV.W1=window1;
    TypeGLscene* scene1=new TypeGLscene; globalV.S1=scene1;

    SetupInitialValues();

    glutMainLoop();

    delete window1;
    delete scene1;
    return 0;
}

//Helper functions------------------------------------------------------------------
void SetupInitialValues()
{
    DoInitPopupMenu();
    DoInitLight();
    DoInitCamera();
    DoInitObjects();


    //Register callback functions
    glutIdleFunc(0);          //Consumes CPU (30% or more even while doing nothing)
    glutDisplayFunc(DoDrawThis);
    glutReshapeFunc(DoResize);
    glutMouseFunc(DoWhenMousePress);
    glutMotionFunc(DoWhenMouseDrag);
    glutKeyboardFunc (DoWhenKeyPress);
    glutSpecialFunc(DoWhenSpecialKeyPress);
}

void DoInitLight()
{
    globalV.S1->Light.isON=true;
    globalV.S1->Light.isRelToCam=true;
    globalV.S1->Light.Position={3,5,0};
    globalV.S1->Light.Intensity=130.0;

    globalV.S1->AmbientColor={0.1,0.1,0.1,1.0};
}

void DoInitCamera()
{
    globalV.S1->CameraList.emplace_back(); //add a camera
    globalV.S1->ActiveCamera=&(globalV.S1->CameraList[0]);
}

void DoInitObjects()
{
    globalV.S1->ObjectList.emplace_back();
    TypeGLObject& Grid=globalV.S1->ObjectList[0];
    Grid.AddGrid(1,5,10,{0.4,0.4,0.4});
    Grid.ExcludeFromLights=true;

    globalV.S1->ImportOBJfile("test.obj");
    unsigned objectCount=globalV.S1->ObjectList.size();
    globalV.S1->ObjectList.reserve(objectCount+11+7+5+1);

    long long skydomeIdx = globalV.S1->GetObjectIndex("SkyDome");
    if (skydomeIdx>-1) {globalV.S1->ObjectList[skydomeIdx].ExcludeFromLights=true;}

    TypeXYZw RedSticker   = {213.0,28.0,28.0,255.0}; RedSticker = RedSticker/255.0;
    TypeXYZw GreenSticker = {62.0,165.0,33.0,255.0}; GreenSticker = GreenSticker/255.0;
    TypeXYZw WhiteSticker = {255.0,255.0,255.0,255.0}; WhiteSticker = WhiteSticker/255.0;
    TypeXYZw YellowSticker= {240.0,235.0,59.0,255.0}; YellowSticker = YellowSticker/255.0;
    TypeXYZw OrangeSticker= {243.0,152.0,20.0,255.0}; OrangeSticker = OrangeSticker/255.0;
    TypeXYZw BlueSticker  = {42.0,60.0,184.0,255.0}; BlueSticker = BlueSticker/255.0;

    unsigned StartTexIdx = globalV.S1->Textures.ListSize();
    TypeGLtexture OneTexture;
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/Spok.bmp");      //0-Red
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/butterfly.bmp"); //1-Green
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/Anubis.bmp");    //2-White
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/Butterfly2.bmp");//3-Yellow
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/V.bmp");         //4-Orange
    globalV.S1->Textures.PushBack(OneTexture);
    globalV.S1->Textures.Back()->Data.LoadTexture("Stickers/Lion.bmp");      //5-Blue
    TypeXYZ TopLeft    ={-1.5,-1.5,+1.5};
    TypeXYZ BottomLeft ={-1.5,-1.5,-1.5};
    TypeXYZ BottomRight={+1.5,-1.5,-1.5};

    long long SurfaceIdx;
    long long MiddlePieceIdx = globalV.S1->GetObjectIndex("MiddlePiece");
    if (MiddlePieceIdx>-1)
    {   //Create middle piece instances

        TypeGLObject& MiddlePieceMaster=globalV.S1->ObjectList[MiddlePieceIdx];
        SurfaceIdx=MiddlePieceMaster.GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) MiddlePieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx=MiddlePieceMaster.GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) MiddlePieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_FrontTop";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(180,true);
        globalV.S1->ObjectList.back().ObjectName="MP_FrontRight";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_FrontBottom";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BackLeft";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BackRight";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90*degToRad),BottomLeft.RotAboutZ(-90*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BackBottom";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BackTop";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_TopLeft";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_TopRight";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BottomLeft";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&MiddlePieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(90,true);
        globalV.S1->ObjectList.back().ObjectName="MP_BottomRight";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);
    }

    long long CornerPieceIdx = globalV.S1->GetObjectIndex("CornerPiece");
    if (CornerPieceIdx>-1)
    {
        TypeGLObject& CornerPieceMaster=globalV.S1->ObjectList[CornerPieceIdx];
        SurfaceIdx=CornerPieceMaster.GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) CornerPieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx=CornerPieceMaster.GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) CornerPieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);
        SurfaceIdx=CornerPieceMaster.GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) CornerPieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ObjectName="CP_FrontTR";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(180,true);
        globalV.S1->ObjectList.back().ObjectName="CP_FrontRB";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ObjectName="CP_FrontLB";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ObjectName="CP_BackTR";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);
        globalV.S1->ObjectList.back().ObjectName="CP_BackTL";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ObjectName="CP_BackRB";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CornerPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-180,true);
        globalV.S1->ObjectList.back().ObjectName="CP_BackLB";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active2");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active3");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);
    }

    long long CenterPieceIdx = globalV.S1->GetObjectIndex("CenterPiece");
    if (CenterPieceIdx>-1)
    {
        TypeGLObject& CenterPieceMaster=globalV.S1->ObjectList[CenterPieceIdx];
        SurfaceIdx = CenterPieceMaster.GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) CenterPieceMaster.SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+0]->Data),TopLeft,BottomLeft,BottomRight,true);

        globalV.S1->ObjectList.emplace_back(&CenterPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ObjectName="CNP_Left";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+1]->Data),TopLeft.RotAboutZ(-90.0*degToRad),BottomLeft.RotAboutZ(-90.0*degToRad),BottomRight.RotAboutZ(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CenterPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);
        globalV.S1->ObjectList.back().ObjectName="CNP_Back";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+4]->Data),TopLeft.RotAboutZ(-180*degToRad),BottomLeft.RotAboutZ(-180*degToRad),BottomRight.RotAboutZ(-180.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CenterPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(90,true);
        globalV.S1->ObjectList.back().ObjectName="CNP_Right";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+3]->Data),TopLeft.RotAboutZ(90*degToRad),BottomLeft.RotAboutZ(90*degToRad),BottomRight.RotAboutZ(90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CenterPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(90,true);
        globalV.S1->ObjectList.back().ObjectName="CNP_Top";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+2]->Data),TopLeft.RotAboutX(-90.0*degToRad),BottomLeft.RotAboutX(-90.0*degToRad),BottomRight.RotAboutX(-90.0*degToRad),true);

        globalV.S1->ObjectList.emplace_back(&CenterPieceMaster,false,false,true,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);
        globalV.S1->ObjectList.back().ParticlesArr[0].Tmatrix.RotAboutY(-90,true);
        globalV.S1->ObjectList.back().ObjectName="CNP_Bottom";
        SurfaceIdx = globalV.S1->ObjectList.back().GetSurfaceIndex("Active1");
        if (SurfaceIdx>-1) globalV.S1->ObjectList.back().SetImageForSurf(SurfaceIdx,&(globalV.S1->Textures[StartTexIdx+5]->Data),TopLeft.RotAboutX(90*degToRad),BottomLeft.RotAboutX(90*degToRad),BottomRight.RotAboutX(90.0*degToRad),true);
    }
}

void DoResetCube ()
{
    long long PartIdx;
    PartIdx = globalV.S1->GetObjectIndex("MiddlePiece");
    if (PartIdx>-1) globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity();
    PartIdx = globalV.S1->GetObjectIndex("MP_FrontTop");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_FrontRight");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(180,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_FrontBottom");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BackLeft");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BackRight");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BackBottom");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-180,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BackTop");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-180,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_TopLeft");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_TopRight");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BottomLeft");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("MP_BottomRight");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(90,true);}

    PartIdx = globalV.S1->GetObjectIndex("CornerPiece");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity();}
    PartIdx = globalV.S1->GetObjectIndex("CP_FrontTR");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_FrontRB");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(180,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_FrontLB");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_BackTR");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_BackTL");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_BackRB");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CP_BackLB");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-180,true);}

    PartIdx = globalV.S1->GetObjectIndex("CenterPiece");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity();}
    PartIdx = globalV.S1->GetObjectIndex("CNP_Left");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CNP_Back");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-180,true);}
    PartIdx = globalV.S1->GetObjectIndex("CNP_Right");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CNP_Top");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(90,true);}
    PartIdx = globalV.S1->GetObjectIndex("CNP_Bottom");
    if (PartIdx>-1) {globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.SetIdentity(); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutZ(-90,true); globalV.S1->ObjectList[PartIdx].ParticlesArr[0].Tmatrix.RotAboutY(-90,true);}

    glutPostRedisplay();
}

void DoInitPopupMenu ()
{
    glutCreateMenu(DoWithPopupChoice);
    glutAddMenuEntry("Quit",0);
    //glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void DoWithPopupChoice (int value)
{
    switch (value)
    {
    case 0:
        exit(0);
    }

}

void DoWhenKeyPress(unsigned char ThisKey, int x, int y)
{

    switch (ThisKey)
    {
    case 'g': case 'G':
        globalV.S1->ObjectList[0].SetIsVisible(!globalV.S1->ObjectList[0].GetIsVisible());
        glutPostRedisplay();
        break;
    case 'w': case 'W':
        globalV.IsWireFrame=!globalV.IsWireFrame;
        globalV.IsVisible=true;
        DoCubeAppearance();
        break;
    case 'h': case 'H':
        if (globalV.InMotion==true) break;

        globalV.IsVisible=!globalV.IsVisible;
        DoCubeAppearance();
        break;
    case 'e': case 'E':
        if (globalV.InMotion==true) break;

        globalV.InMotion=true;
        globalV.IsVisible=true;
        globalV.Explode=!globalV.Explode;
        glutIdleFunc(DoWhileWaiting);
        break;
    case 'r':case 'R':
        if (globalV.InMotion==true) break;
        DoResetCube();
        break;
    case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        if (globalV.InMotion==true) break;

        globalV.InMotion=true;
        globalV.IsVisible=true;
        DoCubeAppearance();
        globalV.rotationPlane=ThisKey-'0';
        glutIdleFunc(DoWhileWaiting);
        break;
    case 'q': case 'Q':
        exit(0);
    //default:
    }
}

void DoWhenSpecialKeyPress (int ThisKey, int x, int y)
{
    cout<<"PRESSed "<<ThisKey<<endl;
}

void DoWhenMouseDrag(int x, int y)
{
    DoConfineCursor(x,y);

    TypeXYZ newPoint = DoConvertFromWindowToModel (x, y);
    TypeXYZ delta = newPoint-globalV.MouseFirstPick;
    if (globalV.IsRightButtonPressed==true && globalV.S1->ActiveCamera->GetIsOrtho()==true && (globalV.ModKeyPressed==0 || globalV.ModKeyPressed==GLUT_ACTIVE_SHIFT))
    { //PAN
        delta = delta*(-1); //camera target needs to move in the opposite direction the mouse is moving
        globalV.S1->ActiveCamera->Move(delta,true);
        globalV.MouseFirstPick=newPoint; //important so that delta doesn't compound
    }
    else if (globalV.IsRightButtonPressed==true && globalV.ModKeyPressed==GLUT_ACTIVE_CTRL)
    {//ZOOM
        globalV.S1->ActiveCamera->Zoom(delta.y*15.0);
        globalV.MouseFirstPick=newPoint; //important so that delta doesn't compound
    }
    else if (globalV.IsRightButtonPressed==true && globalV.S1->ActiveCamera->GetIsOrtho()==false && globalV.ModKeyPressed==0)
    {//ROTATE view
        globalV.S1->ActiveCamera->RotateX(-delta.y*30.0);
        globalV.S1->ActiveCamera->RotateZ(-delta.x*30.0);
        globalV.MouseFirstPick=newPoint; //important so that delta doesn't compound
    }

    glutPostRedisplay();
}

void DoConfineCursor(int& x, int& y)
{
    if (x<0)
    {
        x+=globalV.W1->GetWidth();
        globalV.MouseFirstPick.x+=globalV.S1->ActiveCamera->GetClipWidth();
        glutWarpPointer(x,y);
    }
    else if (x>globalV.W1->GetWidth())
    {
        x-=globalV.W1->GetWidth();
        globalV.MouseFirstPick.x-=globalV.S1->ActiveCamera->GetClipWidth();
        glutWarpPointer(x,y);
    }
    if (y<0)
    {
        y+=globalV.W1->GetHeight();
        globalV.MouseFirstPick.y-=globalV.S1->ActiveCamera->GetClipHeight();
        glutWarpPointer(x,y);
    }
    else if (y>globalV.W1->GetHeight())
    {
        y-=globalV.W1->GetHeight();
        globalV.MouseFirstPick.y+=globalV.S1->ActiveCamera->GetClipHeight();
        glutWarpPointer(x,y);
    }

}

void DoWhenMousePress(int ThisButton, int IsPressed, int x, int y)
{
    globalV.ModKeyPressed=glutGetModifiers();

    if (ThisButton==GLUT_LEFT_BUTTON && IsPressed==GLUT_DOWN)
    {
        globalV.IsLeftButtonPressed=true;
        globalV.IsRightButtonPressed=false;
    }
    else if (ThisButton==GLUT_RIGHT_BUTTON && IsPressed==GLUT_DOWN)
    {
        globalV.IsLeftButtonPressed=false;
        globalV.IsRightButtonPressed=true;
    }

    globalV.MouseFirstPick = DoConvertFromWindowToModel (x, y);
}

void DoWhileWaiting()
{
    double currTime=glutGet(GLUT_ELAPSED_TIME);         //This is in milliseconds
    unsigned ObjCount=globalV.S1->ObjectList.size();
    unsigned finished=0;
    for (unsigned i=0;i<ObjCount;i++)
    {
        TypeGLObject& CurrOBJ=globalV.S1->ObjectList[i];
        string prefix=CurrOBJ.ObjectName.substr(0,2);
        if ( !(prefix=="MP" || prefix=="CP" || prefix=="CN" || prefix=="Co" || prefix=="Mi" || prefix=="Ce" || prefix=="Pl" || prefix=="Me")) continue;

        if (globalV.rotationPlane>0) DoRotate(CurrOBJ);
        else if (prefix!="Pl" && prefix!="Me" && DoExpandContract (CurrOBJ,currTime)==false) {finished++;}

    }

    if (globalV.rotationPlane>0) globalV.rotCummulative+=globalV.rotDeltaR;

    if (finished==26 || globalV.rotCummulative>=90) //There are 26 moving elements in the cube
    {
        glutIdleFunc(0);
        globalV.InMotion=false;
        globalV.rotationPlane=0;
        globalV.rotCummulative=0;
    }
    glutPostRedisplay();
}

void DoRotate (TypeGLObject& CurrOBJ)
{
    TypeKinematics& CurrOBJkinem=CurrOBJ.ParticlesArr[0];
    TypeXYZ         CurrOBJCentr=CurrOBJ.GetCompleteTmatrix(0)*CurrOBJ.GetCentroid();
    if ( (globalV.rotationPlane==1 && CurrOBJCentr.x<-0.5) || (globalV.rotationPlane==2 && CurrOBJCentr.x<0.5 && CurrOBJCentr.x>-0.5) || (globalV.rotationPlane==3 && CurrOBJCentr.x>0.5) )
    {
        CurrOBJkinem.Tmatrix.RotAboutX(-globalV.rotDeltaR);
    }
    else if ( (globalV.rotationPlane==4 && CurrOBJCentr.z>0.5) || (globalV.rotationPlane==5 && CurrOBJCentr.z<0.5 && CurrOBJCentr.z>-0.5) || (globalV.rotationPlane==6 && CurrOBJCentr.z<-0.5) )
    {
        CurrOBJkinem.Tmatrix.RotAboutZ(-globalV.rotDeltaR);
    }
    else if ( (globalV.rotationPlane==7 && CurrOBJCentr.y<-0.5) || (globalV.rotationPlane==8 && CurrOBJCentr.y<0.5 && CurrOBJCentr.y>-0.5) || (globalV.rotationPlane==9 && CurrOBJCentr.y>0.5) )
    {
        CurrOBJkinem.Tmatrix.RotAboutY(globalV.rotDeltaR);
    }

}

bool DoExpandContract(TypeGLObject& CurrOBJ, double currTime)
{
    TypeKinematics& CurrOBJkinem=CurrOBJ.ParticlesArr[0];
    TypeXYZ  defaultPos = CurrOBJ.GetCentroid();
    TypeXYZ  currPos    = CurrOBJkinem.Tmatrix*defaultPos;
    double   displacement=currPos.length()-defaultPos.length();


    if ((globalV.Explode==true && displacement>=1.0) || (globalV.Explode==false && displacement<epsilon))
    {   //The expand/contract motion is finished for this object
        CurrOBJkinem.TranslationVel=0;
        CurrOBJkinem.TimeStamp=0;
        return false;
    }
    if (globalV.Explode==true && displacement<1.0 && CurrOBJkinem.TranslationVel.length()==0)
    {   //Set explode velocity
        CurrOBJkinem.TranslationVel=(CurrOBJ.ParticlesArr[0].Tmatrix * CurrOBJ.GetCentroid()).ofLength(0.002);
    }
    else if (globalV.Explode==false && displacement>epsilon && CurrOBJkinem.TranslationVel.length()==0)
    {   //Set implode velocity
        CurrOBJkinem.TranslationVel=(CurrOBJ.ParticlesArr[0].Tmatrix * CurrOBJ.GetCentroid()).ofLength(-0.002);
    }
    if (CurrOBJkinem.TimeStamp==0) {CurrOBJkinem.TimeStamp=currTime; return true;}
    double deltaT=(currTime-CurrOBJkinem.TimeStamp)/1000;
    TypeXYZ deltaS=CurrOBJkinem.TranslationVel * deltaT;
    CurrOBJkinem.Tmatrix.TranslateBy(deltaS);

    return true;
}

void DoResize(int NewWidth, int NewHeight)
{
    //Store the new window size
    globalV.W1->SetWidth(NewWidth);
    globalV.W1->SetHeight(NewHeight);

    globalV.S1->ActiveCamera->SetAspect((double)NewWidth/(double)NewHeight);

    glViewport(0,0,NewWidth,NewHeight);
}

void PrintText(std::string ThisText)
{
    glColor3f(0,0,0);
    //glWindowPos2i(10,10);
    for (unsigned i=0; i<ThisText.length();i++) {glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,ThisText[i]);}
}

TypeXYZ DoConvertFromWindowToModel (int x,int y)
{
    //window origin is top left. Model origin is in the middle
    //find WinX relative to total width and correlate it to the clipping plane
    double clipWidth=globalV.S1->ActiveCamera->GetClipWidth();
    double clipHeight=globalV.S1->ActiveCamera->GetClipHeight();
    double clipLeft=globalV.S1->ActiveCamera->GetClipLeft();
    double clipTop=globalV.S1->ActiveCamera->GetClipTop();
    //remember the clipVolume might be off-center
    TypeXYZ result;
    result.x=clipWidth * ((double)x / (double)globalV.W1->GetWidth()) + clipLeft;
    result.y=-clipHeight * ((double)y / (double)globalV.W1->GetHeight()) + clipTop;
    result.z=0.0;

    return result;
}

void DoCubeAppearance ()
{
    TypeXYZw WireframeColor = {0.5,0.5,0.5,1.0};
    unsigned ObjCount=globalV.S1->ObjectList.size();
    for (unsigned i=0;i<ObjCount;i++)
    {
        const string& ObjName=globalV.S1->ObjectList[i].ObjectName;
        string prefix=ObjName.substr(0,2);
        if (prefix=="MP" || prefix=="CP" || prefix=="CN" || prefix=="Co" || prefix=="Mi" || prefix=="Ce")
        {
            globalV.S1->ObjectList[i].SetWireframe(globalV.IsWireFrame,WireframeColor);
            globalV.S1->ObjectList[i].SetIsVisible(globalV.IsVisible);
        }
    }
    glutPostRedisplay();
}

void DoDrawThis()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear the color buffer (part of the frame buffer)

    globalV.S1->Draw();
    glutSwapBuffers();
}
