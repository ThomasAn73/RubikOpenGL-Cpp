/*2015 Thomas Androxman
-------------------------------------------------------------------------------------------
This library handles:
-TypeGLobject - This is the most crucial since it interacts with openGL and adds openGL properties to the geometrics objects of the TypeGeometry class
-TypeGLscene  - This acts as an environment where many TypeGLobject and TypeCamera objects live (it is possible to have multiple scenes, each with its own set of objects, lights and cameras)
-TypeGLwindow - This initializes a freeGLUT window
-------------------------------------------------------------------------------------------
*/

#pragma once
#include "ClassCamera.hpp"
#include "ClassFileImport.hpp"
#include <GL/glew.h>
#include <GL/freeglut.h>

//----------------------------------------------------------------------------------------------------------
struct TypeGLbufferID
{
    string  Name        ="";
    bool    isActive    =false; //Is the buffer active ? (data already sent to the card's memory ?)
    bool    isTexture   =false; //Is this a texture buffer ?
    GLuint  ID          =0;     //The ID that GL returned to us for the buffer object it created
    GLsizei ElementCount=0;     //count of the individual array elements in the GL buffer

    //Destructor
    ~TypeGLbufferID () {Reset(); }

    //Methods
    void Reset () {if (isActive==true) {cout<<"buffer <"<<Name<<"> destroyed"<<endl; isTexture==false? glDeleteBuffers(1,&ID):glDeleteTextures(1,&ID); isActive=false; ElementCount=0; ID=0;} }
};

//----------------------------------------------------------------------------------------------------------
template<class Any>
struct TypeAttribute
{
    bool                  IsMaster=true;//This becomes false if an object is an instance (borrows coordinate data) but has it's own separate attributes
    TypeGLbufferID        GLbuffer;     //Information about the GL buffer where the data is being stored
    vector<Any>           data;         //The attribute data themselves

};
//----------------------------------------------------------------------------------------------------------

//***CLASS Declaration**************************************************************************************
//**********************************************************************************************************
class  TypeGLGroup;     //This class is further declared later
class  TypeGlShaderVar; //This class is further declared later
struct TypeGLtexture;   //This struct is further declared later
struct TypeGLmaterial;
//----------------------------------------------------------------------------------------------------------

class TypeGLObject
{
private:
    //Private data section
    //Basic local variables
    bool                     isMaster;        //is this an instance or is it borrowing geometry from a master object ?
    bool                     isVisible;       //whether this object will actually be drawn
    bool                     isWireframe;     //whether to show all surfaces as a wireframe mode
    bool                     showSrfEdges;    //whether to overlay surface edges in shaded mode

    float                    LineWidth;
    float                    EdgeLineWidth;

    //Main shared data by all instances
    TypeGeometry*            Geometry;        //Source data (all clones point to this)
    vector <TypeGLbufferID>* GeomBufferIDs;   //Source data. 0-Coords, 1-PointEL, 2-LineEL, 3-PolylineEL, 4-SurfEL, 5-SurfEdgEL

    //Selectively shared data (these can be local to the instance or shared)
    TypeAttribute<TypeXYZ        >* VertexNormals;   //Can be instanced. (Same size as coordinates array)
    TypeAttribute<TypeXY         >* TextureCoord;    //Can be instanced. (Same size as coordinates array)
    TypeAttribute<TypeXYZw       >* VertexColors;    //Can be instanced. (Same size as coordinates array)
    TypeAttribute<TypeGLtexture* >* LinkImageToSurf; //Can be instanced. (Same size as surfaces registry). Relationship array linking each surface-index to a texture-pointer.
    //This is a single object and should have an overall material feel. Per-surface texture and color is already far enough.

    //These data is local to each instance
    TypeGLObject*            ParentObject;    //Pointer to the source parent object if any;
    TypeGLGroup*             ParentGroup;     //Transformations trickle down through this pointer. (Changing the pointer, changes group affiliation)
    vector<TypeGLObject*>    Children;        //Registry of all child instances
    vector<TypeGLtexture*>   ImageList;       //A list of textures used by this object (shader texture unit uses indices from this vector)

    #define None TypeXYZw {NAN,NAN,NAN,NAN}
    TypeXYZw                 defaultColor;
    TypeXYZw                 SrfEdgeColor;
    TypeXYZw                 WireframeColor;

    //Private Methods
    void AllocateMemory         (bool newGeom, bool newVcolors, bool newNorms, bool newUVs, bool newTextures);
    void DeallocateMemory       ();
    void InitLocals             ();
    void CopyLocalsByValue      (const TypeGLObject& Original);   //Helper for copyByValue and copyAsInstance
    void CopyByValue            (const TypeGLObject& Original);   //used by copy constructor
    void CopyAsInstance         (TypeGLObject& Original);         //used to create an instance out of a master object
    void CopyInstance           (const TypeGLObject& Original);   //used to create an instance out of another instance object
    void GenerateGLbuffers      (); //Setup the data into openGL buffers
    void EnsureArraysSync       ();
    void ComputeImageList       ();
    void Pack2DIndexArrayTo1D   (const vector<vector<unsigned>>& source, vector<GLuint>& destination, bool UseNullEnd=false);
    void PackSurfacesArrayTo1D  (const vector<TypeSurface>& source, vector<GLuint>& destination);
    void PackSrfEdgesArrayTo1D  (const vector<TypeSurface>& source, vector<GLuint>& destination);
    void FillTexUnitToVertexArr (vector<GLint>& destination, const vector<TypeGLtexture*>& source);
    long long GetImageListIdx   (const TypeGLtexture*);
    template<class Any> void VectorArrToGLfloat  (vector<GLfloat>& destination, const vector<Any>& source,unsigned short floatDepth=4);
    template<class Any> void InflateAttribArray  (TypeAttribute<Any>* thisAttribute, Any defaultVal, unsigned toThisSize, bool forced=false);
    template<class Any> void UpdateAttribArray   (TypeAttribute<Any>* thisAttribute, unsigned arrSize, unsigned short soMany, const Any& newValue, const Any& fillValue, bool forceFillValue);

public:
    //Public data
    bool                   ExcludeFromLights; //Whether to compute shading for this object;
    string                 ObjectName;
    vector<TypeKinematics> ParticlesArr;

    //Constructor
    TypeGLObject(); //Default constructor; always makes master objects.
    TypeGLObject(TypeGLObject* master, bool newColors=false, bool newNormals=false, bool newUVs=false, bool newTextures=false); //instance constructor
    TypeGLObject(const TypeGLObject& Original); //copy constructor (also invoked when object is passed by value and on new assignment operations)
    TypeGLObject(TypeOBJfile& sourceFile, unsigned ObjIdx, TypeLinkedList<TypeGLmaterial>& MatList); //import constructor for OBJ file data

    //Destructor
    ~TypeGLObject() {DeallocateMemory ();}

    //MISC methods
    void Draw           (const TypeTmatrix& ProjView, const TypeGlShaderVar& ShaderVar);

    //SET methods
    void SetWireframe   (bool isShown=true, TypeXYZw color=None);
    void SetSurfaceEdges(bool isShown=true, TypeXYZw color=None, float edgeWidth=-1);
    void SetIsVisible   (bool isShown=true)                          {isVisible=isShown;}
    void SetParentGrp   (TypeGLGroup* ThisOne)                       {ParentGroup=ThisOne;}
    void SetDefaultColor(TypeXYZ ThisColor)                          {defaultColor=ThisColor;}
    void SetDefaultColor(double R, double G, double B, double A=1.0) {defaultColor={R,G,B,A};}

    void SetColorForSurf(unsigned SurfIndex, TypeXYZw NewColor);
    void SetImageForSurf(unsigned SurfIndex, TypeGLtexture* Texture, TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight, bool maintainAspect=true);

    //Add geometry
    void AddGrid        (double UnitLength, unsigned UnitCountX, unsigned subdivions=10, TypeXYZw color=None);
    void AddLine        (TypeXYZ a, TypeXYZ b, TypeXYZw color=None);
    void AddNgon        (unsigned short sidesCount, TypeXYZ center, TypeXYZ RadiusPoint, TypeXYZ normal, bool inscribed=true, TypeXYZw color=None);
    void AddTriangleSrf (TypeXYZ a, TypeXYZ b, TypeXYZ c, TypeXYZw color=None);
    void AddRectangleSrf(TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight, TypeXYZw color=None);

    bool ImportGeometry (const TypeOBJData& source); //TODO: Import on top of existing geometry

    //GET methods
    bool           GetIsMaster          () const {return ParentObject==nullptr? true:false;}
    bool           GetIsVisible         () const {return isVisible;}
    bool           GetIsWireframe       () const {return isWireframe;}
    bool           GetIsSrfEdgesShown   () const {return showSrfEdges;}
    bool           GetIsDefaultColorUsed() const {return ((VertexColors->data.size()>0) ? false:true); }
    unsigned int   GetChildrenCount     () const {return Children.size();}
    unsigned int   GetNumberOfCoords    () const {return Geometry->GetNumberOfCoords();} //excluding the centroid
    unsigned int   GetNumberOfPoints    () const {return Geometry->GetNumberOfPoints();}
    unsigned short GetNumberOfLines     () const {return Geometry->GetNumberOfLines();}
    unsigned short GetNumberOfPolylines () const {return Geometry->GetNumberOfPolylines();}
    unsigned short GetNumberOfSurfaces  () const {return Geometry->GetNumberOfSurfaces();}
    TypeXYZ        GetCentroid          () const {return Geometry->GetCentroid();}
    TypeXYZw       GetDefaultColor      () const {return defaultColor;}
    TypeXYZw       GetWireframeColor    () const {return WireframeColor;}
    TypeXYZw       GetSrfEdgeColor      () const {return SrfEdgeColor;}

    TypeTmatrix    GetCompleteTmatrix   (unsigned particleIdx=0) const;
    long long      GetSurfaceIndex      (const string& SurfaceName) const {return Geometry->GetSurfaceIndex(SurfaceName);}

    void           operator=            (const TypeGLObject& Other);
};
//----------------------------------------------------------------------------------------------------------


//***CLASS Declaration**************************************************************************************
//**********************************************************************************************************
class TypeGLGroup    //Used as a grouping mechanism for TypeGLobjects
{
private:
    TypeGLGroup*          ParentGroup;//Parent group
    vector<TypeGLObject>* ChildrenOBJ;//Children objects for this group
    vector<TypeGLGroup>*  ChildrenGRP;//Children groups  for this group

public:
    //Data
    TypeTmatrix           Tmatrix;    //Transformations  for this group

    //Misc methods
    void ExplodeGroup  ();  //All contents spill to the parent group

    //Set/Add methods
    void SetParentGrp  (TypeGLGroup* ThisOne)   {ParentGroup=ThisOne;}
    void AddOBJtoGroup (TypeGLObject* ThisOne);
    void AddGRPtoGroup (TypeGLGroup* ThisOne);

    //Get methods
    TypeGLGroup* GetParent () const     {return ParentGroup;}

};
//----------------------------------------------------------------------------------------------------------

struct TypeGlShaderVar //All the uniform shader variables being used (in one place)
{
    GLuint ProgramID;

    GLint MatrixVarID;
    GLint DefaultColorVarID;
    GLint UseTexturesVarID;
    GLint TexUnitVarID[6];
    GLint NormalsMatrixVarID;
    GLint AmbientColorVarID;
    GLint LightColorVarID;
    GLint LightPositionVarID;
    GLint LightIntensityVarID;
};
//----------------------------------------------------------------------------------------------------------
struct TypeGLtexture //Associate texture data with a GL buffer
{
    TypeGLbufferID GLbuffer;
    TypeImageFile  ImageFile;

    void LoadTexture        (string FileName);
    void GenerateGLbuffer   (); //Generate texture buffers (glGenTextures glBindTexture)
};
//----------------------------------------------------------------------------------------------------------
struct TypeGLmaterial
{
    TypeOBJMat     MatData;
    TypeGLtexture* TexData;

    TypeGLmaterial (string Name="") {MatData.Name=Name;}
};
//----------------------------------------------------------------------------------------------------------
struct TypeLight
{
    bool     isON       =false;
    bool     isRelToCam =true;
    float    Intensity  =1.0;
    TypeXYZw Color      ={1.0,1.0,1.0,1.0};
    TypeXYZ  Position   ={-1.0,-1.0,5.0};
};
//----------------------------------------------------------------------------------------------------------
struct TypeGLscene
{
    vector<TypeGLObject>  ObjectList;        //a scene contains objects
    vector<TypeGLGroup>   GroupList;         //a scene may contain groups of objects
    vector<TypeCamera>    CameraList;        //a scene has cameras (one of which is active)

    //This needs to be a linked list because, unlike C++ vectors, the pointers to linked-list the texture objects do not get destroyed each time a new one is pushed back
    TypeLinkedList<TypeGLmaterial> Materials;//collection of materials
    TypeLinkedList<TypeGLtexture > Textures; //store textures (being large files) at the scene level (send the address to each object to draw)

    TypeCamera*           ActiveCamera;      //Pointer to the currently active camera

    TypeLight             Light;             //One scene light for now (this can be an array in the future)
    TypeXYZw              AmbientColor;      //Scene wide ambient illumination;

    //Constructor
    TypeGLscene ();

    void      Draw ();
    bool      ImportOBJfile     (string ThisFile);
    long long GetObjectIndex    (string LookupByName);

private:
    //Private data
    TypeGlShaderVar ShaderVar;

    //Private methods
    TypeGLmaterial* GetMaterial(const string& lookupName);
    TypeGLtexture*  GetTexture (const string& lookupFileName);

    string LoadGLshader        (string ThisFile);
    bool   ShaderCompileStatus (GLint ShaderID, string filename);
    bool   ShaderLinkingStatus (GLint ProgramID);
    void   InstalShaders       ();
    void   GetUniformVarIDs    ();
};
//----------------------------------------------------------------------------------------------------------


//***CLASS Declaration**************************************************************************************
//**********************************************************************************************************
class TypeGLwindow
{
private:
    //Data
    short Width, Height, xPosition, yPositon;
    GLint windowHandle=0;

public:
    //Constructor
    TypeGLwindow(int& argc, char* argv[], string Title="Untitled", short width=600, short height=600, short xPos=100, short yPos=100);

    //Set methods
    void SetWidth  (short w)     {Width=w;}
    void SetHeight (short h)     {Height=h;}
    void SetPosX   (short x)     {xPosition=x;}
    void SetPosy   (short y)     {yPositon=y;}

    //Get methods
    short GetWidth  ()     {return Width;}
    short GetHeight ()     {return Height;}
    short GetPosX   ()     {return xPosition;}
    short GetPosy   ()     {return yPositon;}
};
//----------------------------------------------------------------------------------------------------------

//The reset of this
#include "ClassGLobject.cpp"
