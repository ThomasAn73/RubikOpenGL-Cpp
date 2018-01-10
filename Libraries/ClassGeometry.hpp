/*2015 Thomas Androxman
-------------------------------------------------------------------------------------------
This library handles:
-2D, 3D, 4D tuples with full vector math support and operator overloading
-Matrix objects and algebra
-Geometry object which acts like a canvas for vertices in space and acts. It registers geometric shapes
-The geometry object is purely theoretical/mathematical (no openGL specifics are used)
-------------------------------------------------------------------------------------------
*/

#pragma once

#include <cmath>    //trig functions
#include <iostream> //cout
#include <iomanip>  //control cout output
#include <string>   //use of strings
#include <vector>   //vector arrays
#include <array>

#define pi 3.1415926535897932384626433832795
#define degToRad 0.01745329251994329576923690768489
#define RadToDeg 57.295779513082320876798154814105
#define epsilon 0.0001

using namespace std;

//***GLOBAL Declarations************************************************************************************
//**********************************************************************************************************
struct TypeXY
{
    double x,y;

    //Struct Constructor Handles aggregate list initialization like: TypeXY p1={1.4,1.0};
    TypeXY(double X=0.0, double Y=0.0) {x=X; y=Y;}

    //Calculations
    bool   IsCollinear     (TypeXY a, TypeXY b)const       {return ((b-a).angleTO(*this-a)<epsilon)? true:false;}
    bool   isOrtho         (TypeXY v)const                 {return (*this).DotProduct(v)<epsilon ? true:false;}
    bool   isValid         () const                        {return (isfinite(x)&&isfinite(y));}
    double length          () const                        {return sqrt(x*x + y*y);}
    double angleXY         () const                        {return atan2(y,x);} //angle in radians (atan2 handles correct quadrants)}
    double angleTO         (TypeXY v) const                {return acos(DotProduct(v)/(length()*v.length()));}
    double DotProduct      (TypeXY v) const                {return v.x*x + v.y*y;}
    TypeXY AsOrthoTo       (TypeXY v) const                {return (*this) - (*this).ProjectONTO(v);}
    TypeXY Normalized      () const                        {return (*this)/length();}
    TypeXY scale           (double X,double Y) const       {return TypeXY{x*X,y*Y};}
    TypeXY ofLength        (double newLen)                 {return (*this)*(newLen/length());}
    TypeXY Rotate          (double angle) const            {return TypeXY {cos(angle)*x-sin(angle)*y,sin(angle)*x+cos(angle)*y};}
    TypeXY ProjectONTO     (TypeXY v) const                {return v*(v.DotProduct(*this)/v.DotProduct(v));}
    TypeXY ReflectABOUT    (TypeXY a, TypeXY b) const      {TypeXY p1 = (*this).ProjectONTO(b-a);
                                                            return p1+p1-(*this);}
    //Misc Methods
    void   Print           (string label="")               {cout<<label<<"("<<x<<", "<<y<<")"<<endl;}

    //Operators
    TypeXY operator+       (const TypeXY& ThatPoint) const {return TypeXY{x+ThatPoint.x,y+ThatPoint.y};}
    TypeXY operator-       (const TypeXY& ThatPoint) const {return TypeXY{x-ThatPoint.x,y-ThatPoint.y};}
    TypeXY operator/       (const double& scalar) const    {return TypeXY{x/scalar,y/scalar};}
    TypeXY operator*       (const double& scalar) const    {return TypeXY{x*scalar,y*scalar};}
    bool   operator==      (const TypeXY& ThatPoint) const {return (x==ThatPoint.x&&y==ThatPoint.y);}
    bool   operator==      (const double& value)  const    {return (x==value&&y==0.0);}
    bool   operator!=      (const TypeXY& ThatPoint) const {return !(x==ThatPoint.x&&y==ThatPoint.y);}
    bool   operator!=      (const double& value) const     {return !(x==value&&y==0.0);}
    void   operator=       (const TypeXY& ThatPoint)       {x=ThatPoint.x; y=ThatPoint.y;}
    void   operator=       (const double& value)           {x=value; y=0.0;} //Same as setting a vector of length 'value' on the X-axis
    double operator[]      (const unsigned short idx) const{return ((idx==0)? x: (idx==1)? y: NAN);}

};
//----------------------------------------------------------------------------------------------------------
struct TypeXYZ
{
    double x,y,z;

    //Struct Constructor Handles aggregate list initialization like: TypeXYZ p1={1.4,1.0,1.0};
    TypeXYZ(double X=0.0, double Y=0.0, double Z=0.0) {x=X; y=Y; z=Z;}

    //Calculations
    bool          IsCoplanar (TypeXYZ a, TypeXYZ b, TypeXYZ c)const
                                                                 {TypeXYZ normal=(b-a).CrossProduct(c-a);
                                                                  double lineTest=normal.x*(x-a.x)+ normal.y*(y-a.y)+ normal.z*(z-a.z);
                                                                  return (lineTest<=epsilon)? true:false;}
    bool    IsCollinear (TypeXYZ a, TypeXYZ b)const              {return ((b-a).angleTO(*this-a)<epsilon)? true:false;}
    bool    isOrtho     (TypeXYZ v)const                         {return (*this).DotProduct(v)<epsilon ? true:false;}
    bool    isValid     () const                                 {return (isfinite(x)&&isfinite(y)&&isfinite(z));}
    double  length      () const                                 {return sqrt(x*x + y*y + z*z);}
    double  angleXY     () const                                 {return atan2(y,x);} //angle in radians (atan2 handles correct quadrants)}
    double  angleYZ     () const                                 {return atan2(z,y);}
    double  angleXZ     () const                                 {return atan2(z,x);}
    double  angleTO     (TypeXYZ v) const                        {return acos(DotProduct(v)/(length()*v.length()));}
    double  DotProduct  (TypeXYZ v) const                        {return v.x*x + v.y*y + v.z*z;}
    TypeXYZ CrossProduct(TypeXYZ v) const                        {return TypeXYZ{y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x};}
    TypeXYZ MutualOrtho (TypeXYZ v) const                        {return (*this).CrossProduct(v);}
    TypeXYZ AsOrthoTo   (TypeXYZ v) const                        {return (*this) - (*this).ProjectONTO(v);}
    TypeXYZ Normalized  () const                                 {return (*this)/length();}
    TypeXYZ scale       (double X,double Y,double Z) const       {return TypeXYZ{x*X,y*Y,z*Z};}
    TypeXYZ ofLength    (double newLen)                          {return (*this)*(newLen/length());}
    TypeXYZ RotAboutX   (double angle) const                     {return TypeXYZ {x,cos(angle)*y-sin(angle)*z,sin(angle)*y+cos(angle)*z};}
    TypeXYZ RotAboutY   (double angle) const                     {return TypeXYZ {cos(angle)*z-sin(angle)*x,y,sin(angle)*z+cos(angle)*x};}
    TypeXYZ RotAboutZ   (double angle) const                     {return TypeXYZ {cos(angle)*x-sin(angle)*y,sin(angle)*x+cos(angle)*y,z};}
    TypeXYZ RotAboutAxis(TypeXYZ axis, double angle) const       {TypeXYZ RelOrigin = (*this).ProjectONTO(axis);
                                                                  TypeXYZ relX = (*this) - RelOrigin;
                                                                  TypeXYZ relY = axis.CrossProduct(*this).ofLength(1.0);
                                                                          relY = relY * (sin(angle)*relX.length());
                                                                          relX = relX *cos(angle);
                                                                          return relX + relY + RelOrigin;}
    TypeXYZ ProjectONTO (TypeXYZ v) const                        {return v*(v.DotProduct(*this)/v.DotProduct(v));}
    TypeXYZ ProjectONTO (TypeXYZ a, TypeXYZ b, TypeXYZ c) const  {TypeXYZ PRab=(*this - a).ProjectONTO(b-a);
                                                                  TypeXYZ PRac=(*this - a).ProjectONTO(c-a);
                                                                  return (PRab+PRac)+a;}
    TypeXYZ ReflectABOUT(TypeXYZ a, TypeXYZ b, TypeXYZ c) const  {TypeXYZ p1 = (*this).ProjectONTO(a,b,c);
                                                                  return p1+p1-(*this);}
    //Misc Methods
    void    Print       (string label="")                        {cout<<label<<"("<<x<<", "<<y<<", "<<z<<")"<<endl;}

    //Operators
    TypeXYZ operator+   (const TypeXYZ& ThatPoint) const         {return TypeXYZ{x+ThatPoint.x,y+ThatPoint.y,z+ThatPoint.z};}
    TypeXYZ operator-   (const TypeXYZ& ThatPoint) const         {return TypeXYZ{x-ThatPoint.x,y-ThatPoint.y,z-ThatPoint.z};}
    TypeXYZ operator/   (const double& scalar) const             {return TypeXYZ{x/scalar,y/scalar,z/scalar};}
    TypeXYZ operator*   (const double& scalar) const             {return TypeXYZ{x*scalar,y*scalar,z*scalar};}
    bool    operator==  (const TypeXYZ& ThatPoint) const         {return (x==ThatPoint.x&&y==ThatPoint.y&&z==ThatPoint.z);}
    bool    operator==  (const double& value)  const             {return (x==value&&y==0.0&&z==0.0);}
    bool    operator!=  (const TypeXYZ& ThatPoint) const         {return !(x==ThatPoint.x&&y==ThatPoint.y&&z==ThatPoint.z);}
    bool    operator!=  (const double& value) const              {return !(x==value&&y==0.0&&z==0.0);}
    void    operator=   (const TypeXYZ& ThatPoint)               {x=ThatPoint.x; y=ThatPoint.y; z=ThatPoint.z;}
    void    operator=   (const double& value)                    {x=value; y=0.0; z=0.0;} //Same as setting a vector of length 'value' on the X-axis
    double  operator[]  (const unsigned short idx) const         {return ((idx==0)? x: (idx==1)? y: (idx==2)? z:NAN);}

    //Type conversions to return simple arrays; to cooperate with functions expecting simple arrays[3]. Deal with this memory leak.
    //operator      double*     () const                                 {double* result=new double[3] {x,y,z}; return result;}
    //operator      float*      () const                                 {float* result=new float[3] {(float)x,(float)y,(float)z}; return result;}
    //operator      int*        () const                                 {int* result=new int[3] {(int)x,(int)y,(int)z}; return result;}
};
//----------------------------------------------------------------------------------------------------------

struct TypeSpherical //depracated (typeCartesian computes this on the fly)
{
    double radius,xyAngle,zAngle; //From X axis, rotate by xyAngle and come down from z axis by zAngle.

    //Struct Constructor
    TypeSpherical(double m=0.0, double xy=0.0, double z=0.0) {radius=m; xyAngle=xy; zAngle=z;}
};
//----------------------------------------------------------------------------------------------------------

struct TypeXYZw
{
    double a,b,c,d;

    //Struct Constructor
    TypeXYZw (double A=0.0, double B=0.0, double C=0.0, double D=1.0) {a=A; b=B; c=C; d=D;}
    TypeXYZw (const TypeXY&   A, double C=0.0, double D=1.0)          {a=A.x; b=A.y; c=C; d=D;} //also doubles as an operator=
    TypeXYZw (const TypeXYZ&  A, double D=1.0)             {a=A.x; b=A.y; c=A.z; d=D;}          //also doubles as an operator=
    TypeXYZw (const TypeXYZw& A)                           {a=A.a; b=A.b; c=A.c; d=A.d;}

    bool     isValid     () const                          {return (isfinite(a)&&isfinite(b)&&isfinite(c)&&isfinite(d));}
    double   Length      () const                          {return sqrt(a*a + b*b + c*c + d*d);}
    double   DotProduct  (TypeXYZw v) const                {return v.a*a + v.b*b + v.c*c + v.d*d;}
    TypeXYZw ProjectONTO (TypeXYZw u) const                {return u*(u.DotProduct(*this)/u.DotProduct(u));}
    TypeXYZw Normalized  () const                          {return (*this)/Length();}

    void     Print       (string label="")                 {cout<<label<<"("<<a<<", "<<b<<", "<<c<<", "<<d<<")"<<endl;}

    TypeXYZw operator+   (const TypeXYZw& ThatTuple) const {return TypeXYZw{a+ThatTuple.a,b+ThatTuple.b,c+ThatTuple.c,d+ThatTuple.d};}
    TypeXYZw operator-   (const TypeXYZw& ThatTuple) const {return TypeXYZw{a-ThatTuple.a,b-ThatTuple.b,c-ThatTuple.c,d-ThatTuple.d};}
    TypeXYZw operator/   (const double& scalar) const      {return TypeXYZw{a/scalar,b/scalar,c/scalar,d/scalar};}
    TypeXYZw operator*   (const double& scalar) const      {return TypeXYZw{a*scalar,b*scalar,c*scalar,d*scalar};}

    bool     operator== (const TypeXYZw& Other) const      {return  (a==Other.a && b==Other.b && c==Other.c && d==Other.d);}
    bool     operator!= (const TypeXYZw& Other) const      {return !(*this==Other);}
    bool     operator== (const double& value) const        {return  (a==value && b==value && c==value && d==value);}
    bool     operator!= (const double& value) const        {return !(*this==value);}
    double   operator[] (const unsigned short idx) const   {return ((idx==0)? a: (idx==1)? b: (idx==2)? c: (idx==3)? d:NAN);}
};
//----------------------------------------------------------------------------------------------------------

struct TypeVector3D
{
    //Data
    TypeXYZ Direction;
    TypeXYZ Origin;

    TypeXYZ Tip       () const                                {return Origin+Direction;}

    //Operator overloading methods -> for vector arithmitic
    TypeVector3D operator+  (const TypeVector3D& ThatVec) const     {return TypeVector3D {Direction+ThatVec.Direction,Origin};}
    TypeVector3D operator-  (const TypeVector3D& ThatVec) const     {return TypeVector3D {Direction-ThatVec.Direction,Origin};}
    TypeVector3D operator/  (const double& scalar) const            {return TypeVector3D {Direction/scalar,Origin};} //Divide by zero creates NaN
    TypeVector3D operator*  (const double& scalar) const            {return TypeVector3D {Direction*scalar,Origin};}
    bool         operator== (TypeVector3D& ThatVec) const           {return (Direction==ThatVec.Direction)? true:false;}
    bool         operator!= (TypeVector3D& ThatVec) const           {return (Direction!=ThatVec.Direction)? true:false;}
};
//----------------------------------------------------------------------------------------------------------

inline double RadFromDeg (double Deg)   {return atan(1) * Deg/45.0; } //PI = atan(1)*4, also 360deg = 2pi rads
inline double DegFromRad (double Rad)   {return 45.0 * Rad / atan(1); } //PI = atan(1)*4, also 360deg = 2pi rads
inline TypeSpherical SphericalFromCartesian (TypeXYZ ThisPoint) {return TypeSpherical {ThisPoint.length(),ThisPoint.angleXY(),ThisPoint.angleTO(TypeXYZ {0,0,1})};}
inline TypeXYZ CartesianFromSpherical (TypeSpherical ThisPoint)
{
    double xyShadow=sin(ThisPoint.zAngle)*ThisPoint.radius;
    return TypeXYZ {cos(ThisPoint.xyAngle)*xyShadow,sin(ThisPoint.xyAngle)*xyShadow,cos(ThisPoint.zAngle)*ThisPoint.radius};
}
//----------------------------------------------------------------------------------------------------------

struct TypeTmatrix
{
    //Data
    double data[4][4];                   //Follows convention -> [Row][Col]
    //Struct constructor
    TypeTmatrix             ();
    TypeTmatrix             (float (&A)[4][4]);
    TypeTmatrix             (float (&A)[16]);

    //Calculations
    void SetIdentity        ();
    void SetZero            ();
    void SetRow             (unsigned short Row, TypeXYZw a);
    void SetRow             (unsigned short Row, double a) {SetRow   (Row,{a,a,a,a});}
    void SetColumn          (unsigned short Col, TypeXYZw a);
    void SetColumn          (unsigned short Col, double a) {SetColumn(Col,{a,a,a,a});}
    void SetInvalid         () {for (int i=0;i<16;i++) data[i/4][i%4]=NAN;}
    void OrthogonalizeCols  (unsigned short thisMany, bool normalize=false); //assumes vectors are stored vertically as in math texts
    void ScaleBy            (double scaleX,double scaleY,double scaleZ);
    void ScaleBy            (double scale);
    void ScaleByPointDrag   (TypeXYZ pivot, TypeXYZ fromA, TypeXYZ toB,bool isUniform=true);
    void ScaleBox           (TypeXYZ boxCornerA, TypeXYZ boxCornerB, TypeXYZ pivotA, TypeXYZ stretchB);
    void TranslateBy        (double x, double y, double z);
    void TranslateBy        (TypeXYZ delta);
    void RotAboutX          (double angle, bool inDegrees=true);
    void RotAboutY          (double angle, bool inDegrees=true);
    void RotAboutZ          (double angle, bool inDegrees=true);
    void Print              (string Comment="") const;

    bool IsValid            () const {for (int i=0;i<16;i++) {if(!isfinite(data[i/4][i%4])) return false;} return true;}

    TypeXYZw GetRow         (unsigned short Row) const    {if (Row>3) Row=3; return TypeXYZw{data[Row][0],data[Row][1],data[Row][2],data[Row][3]};}
    TypeXYZw GetColumn      (unsigned short Col) const    {if (Col>3) Col=3; return TypeXYZw{data[0][Col],data[1][Col],data[2][Col],data[3][Col]};}

    TypeTmatrix   Transpose () const;
    TypeTmatrix   Inverse   () const;

    //Operators
    TypeTmatrix   operator* (const TypeTmatrix& B) const;
    TypeXYZ       operator* (const TypeXYZ& v) const;
};
//----------------------------------------------------------------------------------------------------------

struct TypeKinematics
{
    double        TimeStamp=0.0;
    TypeXYZ       TranslationVel;     //Applied at the center of mass (centroid)
    TypeXYZw      RotatationVel;      //{axis,mag} in rads/sec applied about the center of mass (centroid)
    TypeTmatrix   Tmatrix;            //The current spatial state
};

struct TypePhysics
{
    double mass         =1.0;  //in Kg
    double density      =1.0;
    double elastiscity  =1.0;  //from 0-1 for collision energy transfer
    double friction     =0.0;  //from 0-1
};
//----------------------------------------------------------------------------------------------------------

struct TypeSurface
{
    string SurfaceName;
    vector<unsigned>         Mesh;      //indexes to vertex coordinates forming triangles/or quads depending on implementation
    vector<vector<unsigned>> Edges;     //Stores surface edge polylines (each mesh can have many openings).

    void ComputeEdges (bool IsItTriangleMesh=true); //TODO: compute surface edges
};

struct TypeBspline
{
    short degree;
    vector<unsigned>         ControlPoints;
};

struct TypeGeometryRegistry
{
    vector<TypeXYZ>          Coordinates; //holds a pile of point coordinates in space. [0] is the centroid.
    vector<unsigned>         Points;      //All the stray points are registered here
    vector<vector<unsigned>> Lines;       //Standalone straight lines are registered here
    vector<vector<unsigned>> Polylines;   //Polylines are registered here (each polyline is an array of indexes that form it)
    vector<TypeBspline>      Bsplines;    //Bezier curves are registered here
    vector<TypeSurface>      Surfaces;    //Surfaces are registered here (each surface is an array of indexes to form triangles)
};
//----------------------------------------------------------------------------------------------------------


//***CLASS Declaration**************************************************************************************
//**********************************************************************************************************
//Provides core functionality for geometric object data
//This class serves as a base class to build on.
class TypeGeometry
{
private:
    //Private data
    TypeGeometryRegistry ContentTable;      //Registry of geometric features of the present object
    TypeXYZ              BoundingBox[2];    //geometry bounding box defined by two corners diagonally

    //Private Methods
    void UpdateCentroid();                  //a simple average for now (this can get too fancy quick)
    void ComputeCentroid ();
    void InitializeBoundingBox ();
    void UpdateBoundingBox (TypeXYZ NewPoint);
    void ComputeBoundingBox ();

protected:
    //Protected Methods

public:
    //Constructor
    TypeGeometry();

    //Destructor
    ~TypeGeometry();

    //MISC methods
    void BakeTmatrixIntoGeometry (const TypeTmatrix& Tmatrix); //Apply a Tmatrix to all points for real (their original positioning is lost)

    void GenerateGrid (double UnitLength, unsigned UnitCountX, unsigned subdivions=10);


    //SET methods
    void SetCentroid     (TypeXYZ a);
    void SetCoordByIndex (unsigned int thisPoint, double x, double y, double z,bool updateCentroid=false, bool updateBoundingBox=false);
    void SetCoordByIndex (unsigned int thisPoint, TypeXYZ newXYZ,bool updateCentroid=false, bool updateBoundingBox=false);
    void SetGeometryData (const TypeGeometryRegistry& source);  //Sets the entire geometry table
    void AddCoord        (TypeXYZ a) {ContentTable.Coordinates.push_back(a); UpdateCentroid(); UpdateBoundingBox(a);}
    void AddPoint        (TypeXYZ thisPoint);
    void AddPoint        (double x, double y, double z) {AddPoint(TypeXYZ{x,y,z});}
    void AddLine         (TypeXYZ a, TypeXYZ b);
    void AddPolyline     (short LastStrayPoints);
    void AddNgon         (unsigned short sidesCount, TypeXYZ center, TypeXYZ RadiusPoint, TypeXYZ normal, bool inscribed=true);
    void AddTriangleSrf  (TypeXYZ a, TypeXYZ b, TypeXYZ c);
    void AddRectangleSrf (TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight);
    void AddDiskAprox    ();
    void AddSphereAprox  ();
    void AddCylinderAprox();
    void AddPlaneSubdivided ();
    void RegisterSurface (const TypeSurface& ThisSurfaceIdxArr);

    //void DeleteAllPoints ();

    //GET methods
    unsigned int   GetNumberOfCoords    () const {return ContentTable.Coordinates.size() - 1;} //excluding the centroid
    unsigned int   GetNumberOfPoints    () const {return ContentTable.Points.size();}
    unsigned short GetNumberOfLines     () const {return ContentTable.Lines.size();}
    unsigned short GetNumberOfPolylines () const {return ContentTable.Polylines.size();}
    unsigned short GetNumberOfSurfaces  () const {return ContentTable.Surfaces.size();}

    long long      GetSurfaceIndex      (const string& SurfaceName) const;

    TypeXYZ  GetCentroid          () const {return GetPointByIndex(0);}
    TypeXYZ  GetPointByIndex      (unsigned int i) const;

    const TypeGeometryRegistry& GetGeometryTable () {return ContentTable;}
};
//----------------------------------------------------------------------------------------------------------

struct TypeGroup    //Used as a grouping mechanism for geometry
{
    vector<TypeGeometry>    hasOBJ;     //Other objects at this level
    vector<TypeGroup>       hasGRP;     //Other groups (creating hierarchies)
    TypeTmatrix             Tmatrix;    //Transformations for the entire group
};
//----------------------------------------------------------------------------------------------------------

//The rest of this
#include "ClassGeometry.cpp"
