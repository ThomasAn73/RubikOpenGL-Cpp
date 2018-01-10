/*2015 Thomas Androxman
-------------------------------------------------------------------------------------------
This library handles:
-Camera object
-------------------------------------------------------------------------------------------
*/

#pragma once
#include "ClassGeometry.hpp"

//***CLASS Declaration**************************************************************************************
//**********************************************************************************************************
class TypeCamera
{
private:
    //Private variables
    TypeXYZ EyePos;
    TypeXYZ Target;
    TypeXYZ Up;
    TypeXYZ FOV;   //field of view (in degrees) in the X,Y, diagonal directions of the camera

    bool   Ortho;        //for 2D applications
    double clipLeft;     //clipping plane measured from the camera
    double clipRight;    //there is such thing as off axis projection
    double clipTop;      //don't assume that the values will always be symmetrical
    double clipBottom;
    double nearPlane;    //defining a clipping volume
    double farPlane;
    double aspect;       //the aspect ratio of the clipping plane

    //Private Methods
    void   computeViewMatrix2      ();
    void   computeFOV              ();
    void   computeAspect           ();    //aspect ratio given the existing clip plane values
    double ComputeClipPlaneScale   (double th1, double th2, double newFOV); //given a new FOV, how much would you have to scale the clip plane dimensions

protected:
    //Protected Variable

    //Protected Methods

public:
    //Constructor
    TypeCamera();

    //Destructor
    ~TypeCamera();

    //Data
    TypeTmatrix ViewMatrix;
    TypeTmatrix ProjectionMatrix;

    //MISC methods
    void computeViewMatrix       ();          //applying this matrix to the scene will produce the camera vantage point
    void computeProjectionMatrix ();
    void Move         (TypeXYZ newTarget, bool isDelta=false);
    void Zoom         (double scale);
    void RotateZ      (double angle);              //in degrees
    void RotateX      (double angle);              //in degrees

    //SET methods
    void SetAspect    (double newRatio);      //will affect clipping values and FOV
    void SetFOVy      (double degrees);       //will affect clipping values
    void SetClipPlane (double left, double right, double bottom, double top); //will affect FOV, aspect ratio
    void SetNearPlane (double nearDist)       {if (nearDist>=farPlane) {farPlane+=1;} nearPlane=nearDist; computeFOV(); computeProjectionMatrix();}
    void SetFarPlane  (double farDist)        {(farDist<=nearPlane ? farDist=nearPlane+1: farPlane=farDist);}
    void SetOrthoMode (bool isOrtho)          {Ortho=isOrtho;}
    void SetUpDir     (TypeXYZ UpDir)         {Up=UpDir;}
    void SetCameraPosition (TypeXYZ NewPos)   {EyePos=NewPos;}
    void SetCameraTarget   (TypeXYZ NewTarget){Target=NewTarget;}
    void SetViewTop   ()                      {EyePos={0,0,5}; Target={0,0,0}; Up={0,1,0};}
    void SetViewFront ()                      {EyePos={0,-5,0}; Target={0,0,0}; Up={0,0,1};}
    void SetViewRight ()                      {EyePos={5,0,0}; Target={0,0,0}; Up={0,0,1};}
    void SetViewPersp ()                      {EyePos={-1.9619,-4.11307,2.05759}; Target={0,0,0}; Up={0,1,0};}


    //GET methods
    TypeXYZ GetEyePos        () const   {return EyePos;}
    TypeXYZ GetTarget        () const   {return Target;}
    TypeXYZ GetUpDir         () const   {return Up;}
    TypeXYZ GetFOV           () const   {return FOV;}

    double  GetClipLeft      () const   {return clipLeft;}
    double  GetClipRight     () const   {return clipRight;}
    double  GetClipBottom    () const   {return clipBottom;}
    double  GetClipTop       () const   {return clipTop;}
    double  GetClipWidth     () const   {return clipRight-clipLeft;}
    double  GetClipHeight    () const   {return clipTop-clipBottom;}
    double  GetNearPlane     () const   {return nearPlane;}
    double  GetFarPlane      () const   {return farPlane;}
    double  GetAspect        () const   {return aspect;}
    bool    GetIsOrtho       () const   {return Ortho;}

};

//The rest of this
#include "ClassCamera.cpp"
