#include "ClassGLobject.hpp"

//***GLobject CLASS implementation****************************************************************************
//************************************************************************************************************

//---PRIVATE functions----------------------------------------------------------------------------------------
void TypeGLObject::InitLocals ()
{
    isMaster       = true;
    isVisible      = true;
    isWireframe    = false;
    showSrfEdges   = false;
    defaultColor   = {0.3,0.3,0.3,1.0};
    SrfEdgeColor   = {0.5,0.5,0.5,1.0};
    WireframeColor = {0.9,0.6,0.0,1.0};
    LineWidth      = 1.0;
    EdgeLineWidth  = 1.0;

    ParentObject   = nullptr;
    ParentGroup    = nullptr;

    ExcludeFromLights = false;
    ObjectName     = "";

    ParticlesArr.resize(1);
}

void TypeGLObject::AllocateMemory (bool newGeom, bool newVcolors, bool newNorms, bool newUVs, bool newTextures)
{
    if (newGeom==true)
    {
        Geometry        =new TypeGeometry;                   //new geometry table
        GeomBufferIDs   =new vector<TypeGLbufferID >(6);     // 0-Coords, 1-PointEL, 2-LineEL, 3-PolylineEL, 4-SurfEL, 5-surfEdgEL
    }

    if (newVcolors ==true) {VertexColors    =new TypeAttribute<TypeXYZw       >; VertexColors   ->IsMaster=false;}
    if (newNorms   ==true) {VertexNormals   =new TypeAttribute<TypeXYZ        >; VertexNormals  ->IsMaster=false;}
    if (newUVs     ==true) {TextureCoord    =new TypeAttribute<TypeXY         >; TextureCoord   ->IsMaster=false;}
    if (newTextures==true) {LinkImageToSurf =new TypeAttribute<TypeGLtexture* >; LinkImageToSurf->IsMaster=false;}
}

void TypeGLObject::DeallocateMemory ()
{
    if (isMaster==true)
    {
        delete Geometry;
        delete GeomBufferIDs;
        delete VertexNormals;
        delete TextureCoord;
        delete VertexColors;
        delete LinkImageToSurf;
    }
    else
    {   //on the children's attributes the IsMaster==false is if an object is an instance (borrows coordinate data) but has it's own separate attributes
        if (VertexNormals  ->IsMaster==false) {delete VertexNormals;}
        if (TextureCoord   ->IsMaster==false) {delete TextureCoord;}
        if (VertexColors   ->IsMaster==false) {delete VertexColors;}
        if (LinkImageToSurf->IsMaster==false) {delete LinkImageToSurf;}

        //De-register from parent
        unsigned siblingCount=ParentObject->Children.size();
        for (unsigned i=0;i<siblingCount;i++)
        {
            if(ParentObject->Children[i]==this) ParentObject->Children.erase(ParentObject->Children.begin()+i);
        }
    }
}

void TypeGLObject::CopyLocalsByValue (const TypeGLObject& Original)
{
    isVisible        = Original.isVisible;
    isWireframe      = Original.isWireframe;
    showSrfEdges     = Original.showSrfEdges;
    ParentGroup      = Original.ParentGroup;
    defaultColor     = Original.defaultColor;
    SrfEdgeColor     = Original.SrfEdgeColor;
    WireframeColor   = Original.WireframeColor;
    LineWidth        = Original.LineWidth;
    EdgeLineWidth    = Original.EdgeLineWidth;
    ParticlesArr     = Original.ParticlesArr;
    ExcludeFromLights= Original.ExcludeFromLights;
    ObjectName       = Original.ObjectName;
}

void TypeGLObject::CopyByValue (const TypeGLObject& Original)
{   //Making an exact copy (all data one for one)
    CopyLocalsByValue(Original);
    isMaster       =  Original.isMaster;
    ParentObject   =  Original.ParentObject;
    Children       =  Original.Children;
   *Geometry       = *Original.Geometry;       //Copies all vector values one by one
   *GeomBufferIDs  = *Original.GeomBufferIDs;  //Copies all vector values one by one
   *VertexColors   = *Original.VertexColors;   //Copies all vector values one by one
   *VertexNormals  = *Original.VertexNormals;  //Copies all vector values one by one
   *TextureCoord   = *Original.TextureCoord;   //Copies all vector values one by one
   *LinkImageToSurf= *Original.LinkImageToSurf;//Copies all vector values one by one
}

//Copy a master object to form an instance object
void TypeGLObject::CopyAsInstance (TypeGLObject& Original)
{
    CopyLocalsByValue(Original);
    isMaster       = false;
    ParentObject   = &Original;               //The original object becomes the parent
    Geometry       = Original.Geometry;       //Copies the pointer which means this instance uses the master's vertex geometry
    GeomBufferIDs  = Original.GeomBufferIDs;  //Copies the pointer
    VertexColors   = Original.VertexColors;   //Copies the pointer
    VertexNormals  = Original.VertexNormals;  //Copies the pointer
    TextureCoord   = Original.TextureCoord;   //Copies the pointer
    LinkImageToSurf= Original.LinkImageToSurf;//Copies the pointer
}

//Copy an instance object as another instance object
void TypeGLObject::CopyInstance (const TypeGLObject& Original)
{
    CopyLocalsByValue(Original);
    isMaster       = Original.isMaster;       //This boolean needs to become obsolete (we can check ParentObject pointer for the same info)
    ParentObject   = Original.ParentObject;   //Same parent object as the original instance
    Geometry       = Original.Geometry;       //Copies the pointer which means this instance uses the master's vertex geometry
    GeomBufferIDs  = Original.GeomBufferIDs;  //Copies the pointer
    if (Original.VertexColors->IsMaster    ==true) {VertexColors    =Original.VertexColors;   } else {*VertexColors    = *Original.VertexColors;}
    if (Original.VertexNormals->IsMaster   ==true) {VertexNormals   =Original.VertexNormals;  } else {*VertexNormals   = *Original.VertexNormals;}
    if (Original.TextureCoord->IsMaster    ==true) {TextureCoord    =Original.TextureCoord;   } else {*TextureCoord    = *Original.TextureCoord;}
    if (Original.LinkImageToSurf->IsMaster ==true) {LinkImageToSurf =Original.LinkImageToSurf;} else {*LinkImageToSurf = *Original.LinkImageToSurf;}
}

//TypeAttribute is a tuple4. floatDepth tells how much of the tuple4 to use into the new array
template<class Any>
void TypeGLObject::VectorArrToGLfloat (vector<GLfloat>& destination, const vector<Any>& source, unsigned short floatDepth)
{
    if (floatDepth>4) floatDepth=4;
    unsigned length = source.size();

    destination.resize(length*floatDepth);
    for (unsigned i=0;i<floatDepth;i++) {destination[i]=NAN;}   //Zeroth element is not used
    for (unsigned i=1;i<length;i++)
    {
        TypeXYZw oneTuple4 = source[i];  //Copy value into a large tuple to avoid referencing surprises later
        if (floatDepth>0) destination[i*floatDepth + 0]=oneTuple4[0];
        if (floatDepth>1) destination[i*floatDepth + 1]=oneTuple4[1];
        if (floatDepth>2) destination[i*floatDepth + 2]=oneTuple4[2];
        if (floatDepth>3) destination[i*floatDepth + 3]=oneTuple4[3];
    }
}

void TypeGLObject::Pack2DIndexArrayTo1D(const vector<vector<unsigned>>& source, vector<GLuint>& destination, bool UseNullEnd)
{
    unsigned TotalRows=source.size();

    //Populate the destination vector (should I pre-allocate vector size instead of using push_back?)
    for (unsigned i=0;i<TotalRows;i++)
    {
        unsigned RowLength=source[i].size();
        for (unsigned j=0;j<RowLength;j++) {destination.push_back(source[i][j]);}
        if (UseNullEnd==true) destination.push_back(0);
    }
}

void TypeGLObject::PackSurfacesArrayTo1D  (const vector<TypeSurface>& source, vector<GLuint>& destination)
{
    unsigned TotalSurafes=source.size();

    //Populate the destination vectors
    for (unsigned i=0;i<TotalSurafes;i++)
    {
        unsigned MeshSize=source[i].Mesh.size();
        for (unsigned j=0;j<MeshSize;j++) {destination.push_back(source[i].Mesh[j]);}
    }
}

void TypeGLObject::PackSrfEdgesArrayTo1D  (const vector<TypeSurface>& source, vector<GLuint>& destination)
{
    unsigned TotalSurafes=source.size();

    //Populate the destination vectors
    for (unsigned i=0;i<TotalSurafes;i++)
    {
        unsigned EdgeCount=source[i].Edges.size();
        for (unsigned k=0;k<EdgeCount;k++)
        {
            unsigned EdgeLength=source[i].Edges[k].size();
            for (unsigned l=0;l<EdgeLength;l++) {destination.push_back(source[i].Edges[k][l]);}
            destination.push_back(0); //signifies termination of the edge polyline
        }
    }
}

void TypeGLObject::ComputeImageList()
{
    ImageList.clear();
    unsigned Len=LinkImageToSurf->data.size();
    for (unsigned i=0;i<Len;i++)
    {
        if (LinkImageToSurf->data[i]==nullptr) continue;
        //Poor: can this nested loop **lookup** be made more efficient ?
        if (GetImageListIdx(LinkImageToSurf->data[i])<0) ImageList.push_back(LinkImageToSurf->data[i]);
    }
}

//ImageList is local (uses different indices than GLscene textures)
long long TypeGLObject::GetImageListIdx(const TypeGLtexture* thisImage)
{
    long long found=-1; if (thisImage==nullptr) return found;
    unsigned RegLen=ImageList.size();
    for (unsigned j=0;j<RegLen;j++) {if (ImageList[j]==thisImage) found=j;}
    return found;
}

//This is the texture unit 'selector' array sent to the GL buffers
//It is used to manipulate the fragment shader texture unit selection on per vertex basis
void TypeGLObject::FillTexUnitToVertexArr (vector<GLint>& destination, const vector<TypeGLtexture*>& source)
{
    unsigned Len=Geometry->GetNumberOfCoords()+1; //including the centroid
    destination.resize(Len,-1); //Fill it with the 'none' value
    ComputeImageList();     //Figure out which textures are being used by this object

    //For each surface
    //unsigned surfCount=LinkImageToSurf->data.size();
    unsigned surfCount=LinkImageToSurf->data.size();
    for (unsigned i=0;i<surfCount;i++)
    {
        int texUnit=GetImageListIdx(LinkImageToSurf->data[i]);

        //For each index in the surface index array (there is index overlap between adjacent triangles, so vertices are being updated at least twice)
        unsigned surfVertCount=Geometry->GetGeometryTable().Surfaces[i].Mesh.size();
        for (unsigned j=0;j<surfVertCount;j++)
        {
            unsigned vertIdx=Geometry->GetGeometryTable().Surfaces[i].Mesh[j];
            destination[vertIdx]=texUnit;
        }
    }
}

void TypeGLObject::GenerateGLbuffers()
{
    if (Geometry->GetNumberOfCoords()==0) return; //Nothing to draw
    const TypeGeometryRegistry GeomTable = Geometry->GetGeometryTable(); //get a reference to the geometry data object itself as read-only

    //Create a bunch of buffers and keep their handleIDs
    //Vertex coordinates buffer (entire coordinates array is loaded here for this GLobject)
    if ( (*GeomBufferIDs)[0].isActive==false )
    {
        vector<GLfloat> VertexCoordArr;
        //CoordsArrToFloat3(VertexCoordArr, GeomTable.Coordinates);
        VectorArrToGLfloat (VertexCoordArr,GeomTable.Coordinates,3);
        (*GeomBufferIDs)[0].isActive=true;
        (*GeomBufferIDs)[0].Name="Vertex Coordinates";
        (*GeomBufferIDs)[0].ElementCount=VertexCoordArr.size();
        glGenBuffers (1, &((*GeomBufferIDs)[0].ID));            //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ARRAY_BUFFER,(*GeomBufferIDs)[0].ID);  //connect the object to the GL_ARRAY_BUFFER docking point
        glBufferData (GL_ARRAY_BUFFER,(VertexCoordArr.size())*sizeof(GLfloat),&VertexCoordArr[0],GL_STATIC_DRAW); //Send data through the GL_ARRAY_BUFFER docking point.
    }

    //START of elements buffers---------------------------------------------------------------------------------------
    //Elements buffer for Points
    if ((*GeomBufferIDs)[1].isActive==false && Geometry->GetNumberOfPoints()>0) //if the buffer is not active but there are data
    {
        (*GeomBufferIDs)[1].isActive=true;
        (*GeomBufferIDs)[1].Name="Points element buffer";
        (*GeomBufferIDs)[1].ElementCount=GeomTable.Points.size();
        //point index array is already a one dimension vector of ints
        glGenBuffers (1, &((*GeomBufferIDs)[1].ID));                   //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[1].ID); //connect the object to the GL_ELEMENT_ARRAY_BUFFER docking point
        glBufferData (GL_ELEMENT_ARRAY_BUFFER,GeomTable.Points.size()*sizeof(unsigned int),&GeomTable.Points[0],GL_STATIC_DRAW); //Send data through the GL_ELEMENT_ARRAY_BUFFER docking point.
    }

    //Elements buffer for Lines
    if ((*GeomBufferIDs)[2].isActive==false && Geometry->GetNumberOfLines()>0) //if the buffer is not active but there are data
    {
        vector<GLuint> LineIndexArr;
        Pack2DIndexArrayTo1D(GeomTable.Lines,LineIndexArr);

        (*GeomBufferIDs)[2].isActive=true;
        (*GeomBufferIDs)[2].Name="Lines Element buffer";
        (*GeomBufferIDs)[2].ElementCount=LineIndexArr.size();
        glGenBuffers (1, &((*GeomBufferIDs)[2].ID));                   //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[2].ID); //connect the object to the GL_ELEMENT_ARRAY_BUFFER docking point
        glBufferData (GL_ELEMENT_ARRAY_BUFFER,LineIndexArr.size()*sizeof(GLuint),&LineIndexArr[0],GL_STATIC_DRAW); //Send data through the GL_ELEMENT_ARRAY_BUFFER docking point.
    }

    //Elements buffer for PolyLines
    if ((*GeomBufferIDs)[3].isActive==false && Geometry->GetNumberOfPolylines()>0) //if the buffer is not active but there are data
    {
        vector<GLuint> PolylineIndexArr;
        Pack2DIndexArrayTo1D(GeomTable.Polylines,PolylineIndexArr,true);
        (*GeomBufferIDs)[3].isActive=true;
        (*GeomBufferIDs)[3].Name="Polyline element buffer";
        (*GeomBufferIDs)[3].ElementCount=PolylineIndexArr.size();
        glGenBuffers (1, &((*GeomBufferIDs)[3].ID));                   //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[3].ID); //connect the object to the GL_ELEMENT_ARRAY_BUFFER docking point
        glBufferData (GL_ELEMENT_ARRAY_BUFFER,PolylineIndexArr.size()*sizeof(GLuint),&PolylineIndexArr[0],GL_STATIC_DRAW); //Send data through the GL_ELEMENT_ARRAY_BUFFER docking point.
    }

    //Elements buffer for Surfaces
    if ((*GeomBufferIDs)[4].isActive==false && Geometry->GetNumberOfSurfaces()>0) //if the buffer is not active but there are data
    {
        vector<GLuint> SurfaceIndexArr;
        PackSurfacesArrayTo1D(GeomTable.Surfaces,SurfaceIndexArr);
        (*GeomBufferIDs)[4].isActive=true;
        (*GeomBufferIDs)[4].Name="Surfaces element buffer";
        (*GeomBufferIDs)[4].ElementCount=SurfaceIndexArr.size();
        glGenBuffers (1, &((*GeomBufferIDs)[4].ID));                   //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[4].ID); //connect the object to the GL_ELEMENT_ARRAY_BUFFER docking point
        glBufferData (GL_ELEMENT_ARRAY_BUFFER,SurfaceIndexArr.size()*sizeof(GLuint),&SurfaceIndexArr[0],GL_STATIC_DRAW); //Send data through the GL_ELEMENT_ARRAY_BUFFER docking point.
    }

    //Elements buffer for Surface edges
    if ((*GeomBufferIDs)[5].isActive==false && showSrfEdges==true && Geometry->GetNumberOfSurfaces()>0)
    {
        vector<GLuint> SurfaceEdgesIndexArr;
        PackSrfEdgesArrayTo1D(GeomTable.Surfaces,SurfaceEdgesIndexArr);
        (*GeomBufferIDs)[5].isActive=true;
        (*GeomBufferIDs)[5].Name="Surface edges element buffer";
        (*GeomBufferIDs)[5].ElementCount=SurfaceEdgesIndexArr.size();
        glGenBuffers (1, &((*GeomBufferIDs)[5].ID));                   //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[5].ID); //connect the object to the GL_ELEMENT_ARRAY_BUFFER docking point
        glBufferData (GL_ELEMENT_ARRAY_BUFFER,SurfaceEdgesIndexArr.size()*sizeof(GLuint),&SurfaceEdgesIndexArr[0],GL_STATIC_DRAW); //Send data through the GL_ELEMENT_ARRAY_BUFFER docking point.
    }
    //END of elements buffers----------------------------------------------------------------------------------------

    //Colors buffer (if we have one)
    //Colors don't have their own index buffer. GL uses the vertex index buffer for colors and normals too
    if (VertexColors->GLbuffer.isActive==false && VertexColors->data.size()>0) //if the buffer is not active but there are data
    {
        vector<GLfloat> VertexColorArr;
        VectorArrToGLfloat(VertexColorArr, VertexColors->data); //the shader expects vec4 for color
        VertexColors->GLbuffer.isActive=true;
        VertexColors->GLbuffer.Name="Vertex colors buffer";
        VertexColors->GLbuffer.ElementCount=VertexColorArr.size();
        glGenBuffers (1,&VertexColors->GLbuffer.ID);              //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ARRAY_BUFFER,VertexColors->GLbuffer.ID); //connect the object to the GL_ARRAY_BUFFER docking point
        glBufferData (GL_ARRAY_BUFFER,VertexColorArr.size()*sizeof(GLfloat),&VertexColorArr[0],GL_STATIC_DRAW); //Send data through the GL_ARRAY_BUFFER docking point.
    }

    //Normals buffer (if we have one)
    if (VertexNormals->GLbuffer.isActive==false && VertexNormals->data.size()>0) //if the buffer is not active but there are data
    {
        vector<GLfloat> VertexNormalsArr;
        VectorArrToGLfloat(VertexNormalsArr, VertexNormals->data,3); //making an array of 3 floats
        VertexNormals->GLbuffer.isActive=true;
        VertexNormals->GLbuffer.Name="Vertex normals buffer";
        VertexNormals->GLbuffer.ElementCount=VertexNormalsArr.size();
        glGenBuffers (1,&VertexNormals->GLbuffer.ID);              //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ARRAY_BUFFER,VertexNormals->GLbuffer.ID); //connect the object to the GL_ARRAY_BUFFER docking point
        glBufferData (GL_ARRAY_BUFFER,VertexNormalsArr.size()*sizeof(GLfloat),&VertexNormalsArr[0],GL_STATIC_DRAW); //Send data through the GL_ARRAY_BUFFER docking point.
    }

    //Texture coordinates buffer (if we have one). Texture pixel arrays are stored up at the 'scene' object.
    if (TextureCoord->GLbuffer.isActive==false && TextureCoord->data.size()>0) //if the buffer is not active but there are data
    {
        vector<GLfloat> TextureCoordsArr;
        VectorArrToGLfloat(TextureCoordsArr, TextureCoord->data,2); //making an array of 2 floats
        TextureCoord->GLbuffer.isActive=true;
        TextureCoord->GLbuffer.Name="Texture coordinates buffer";
        TextureCoord->GLbuffer.ElementCount=TextureCoordsArr.size();
        glGenBuffers (1,&TextureCoord->GLbuffer.ID);              //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ARRAY_BUFFER,TextureCoord->GLbuffer.ID); //connect the object to the GL_ARRAY_BUFFER docking point
        glBufferData (GL_ARRAY_BUFFER,TextureCoordsArr.size()*sizeof(GLfloat),&TextureCoordsArr[0],GL_STATIC_DRAW); //Send data through the GL_ARRAY_BUFFER docking point.
    }

    //Vertex-to-textureUnit relationship buffer
    if (LinkImageToSurf->GLbuffer.isActive==false && LinkImageToSurf->data.size()>0) //if the buffer is not active but there are data
    {
        //create an array of texture buffer IDs to pass to GL
        vector<GLint> TexUnitToVertexArr;    //same size as coordinates array linking each vertex to a texture ID
        FillTexUnitToVertexArr (TexUnitToVertexArr, LinkImageToSurf->data);
        LinkImageToSurf->GLbuffer.isActive=true;
        LinkImageToSurf->GLbuffer.Name="Material to surface relationship array buffer";
        LinkImageToSurf->GLbuffer.ElementCount=TexUnitToVertexArr.size();
        glGenBuffers (1,&LinkImageToSurf->GLbuffer.ID);              //instantiate ONE buffer object and return its handle/ID
        glBindBuffer (GL_ARRAY_BUFFER,LinkImageToSurf->GLbuffer.ID); //connect the object to the GL_ARRAY_BUFFER docking point
        glBufferData (GL_ARRAY_BUFFER,TexUnitToVertexArr.size()*sizeof(GLint),&TexUnitToVertexArr[0],GL_STATIC_DRAW); //Send data through the GL_ARRAY_BUFFER docking point.
    }
}

//This method makes sure the attributes array (if started) is as large as the specified size
template<class Any>
void TypeGLObject::InflateAttribArray (TypeAttribute<Any>* thisAttribute, Any defaultVal, unsigned toThisSize, bool forced)
{
    unsigned CurSize=thisAttribute->data.size();
    if ((CurSize>0 || forced==true) && CurSize<toThisSize)
    {
        thisAttribute->data.resize(toThisSize, defaultVal); //inflate using defaultVal
        thisAttribute->GLbuffer.Reset();
    }
}

void TypeGLObject::EnsureArraysSync ()
{
    unsigned VertCount=Geometry->GetNumberOfCoords()+1;
    InflateAttribArray(VertexColors,defaultColor,VertCount);
    InflateAttribArray(VertexNormals,{NAN,NAN,NAN},VertCount);
    InflateAttribArray(TextureCoord,{NAN,NAN},VertCount);

    TypeGLtexture* defaultTex=nullptr;
    InflateAttribArray(LinkImageToSurf,defaultTex,Geometry->GetNumberOfSurfaces());
}

//Adds values at the end of the attribute arrays
template<class Any>
void TypeGLObject::UpdateAttribArray (TypeAttribute<Any>* thisAttribute, unsigned arrSize, unsigned short soMany, const Any& newValue, const Any& fillValue, bool useNewValue)
{
    if (useNewValue==true)
    {
        InflateAttribArray(thisAttribute,fillValue,arrSize,true); //Explicitly use default on all previous features
        unsigned LastIdx=thisAttribute->data.size()-1;
        if (LastIdx+1<soMany) soMany=LastIdx+1;
        for (unsigned short i=0;i<soMany;i++) thisAttribute->data[LastIdx-i]=newValue; //Use color on new features
    }
    else InflateAttribArray(thisAttribute,fillValue,arrSize);     //If already started continue it.
}

// PUBLIC FUNCTIONS-------------------------------------------------------------------------------------------
//---CONSTRUCTORS---------------------------------------------------------------------------------------------

//Default constructor (always makes master objects)
TypeGLObject::TypeGLObject()
{
    InitLocals();
    AllocateMemory(true,true,true,true,true);
}

//Copy constructor (passing by value as function parameter, or assigning to a new object)
TypeGLObject::TypeGLObject(const TypeGLObject& Original)
{
    if (Original.isMaster==true)
    {   //Copy a master as master
        unsigned ChildrenCount=Original.Children.size();
        if (ChildrenCount>0)
        {
            cout<<"ERROR (Copy constructor): The master object <"<<Original.ObjectName;
            cout<<"> is being referenced by <"<<ChildrenCount;
            cout<<"> child objects and therefore is not trivially copyable. Since it is unknown whether the copy or the original will be later deleted, copying is undefined;";
            cout<<"(Object was either passed by value in a function,";
            cout<<" or a copy was attempted behind the scenes by a C++ Vector object, or similar).";
            exit(1);
        }
        //Assign new memory and copy all contents one by one
        AllocateMemory(true,true,true,true,true);
        CopyByValue(Original);
    }
    else
    {   //Copy a child as child
        bool newColors  = Original.VertexColors->IsMaster    ==true? false:true;
        bool newNormals = Original.VertexNormals->IsMaster   ==true? false:true;
        bool newUVs     = Original.TextureCoord->IsMaster    ==true? false:true;
        bool newTextures= Original.LinkImageToSurf->IsMaster ==true? false:true;
        AllocateMemory(false,newColors,newNormals,newUVs,newTextures);
        CopyInstance(Original);
    }
}

//Instance constructor
TypeGLObject::TypeGLObject(TypeGLObject* master, bool newColors, bool newNormals, bool newUVs, bool newTextures)
{
    if (master!=nullptr)
    {   //Create a child instance
        CopyAsInstance(*master);
        AllocateMemory(false,newColors,newNormals,newUVs,newTextures);

        //Register this child
        master->Children.push_back(this);
    }
    else
    {   //Create a master
        InitLocals();
        AllocateMemory(true,true,true,true,true);
    }
}

//Import constructor
TypeGLObject::TypeGLObject(TypeOBJfile& sourceFile, unsigned ObjIdx, TypeLinkedList<TypeGLmaterial>& MatList)
{
    //Constructing a master object
    InitLocals();
    AllocateMemory(true,true,true,true,true);

    const TypeOBJData& sourceObj=sourceFile.GetObjects()[ObjIdx];
    ObjectName=sourceObj.Name;

    //Geometry data
    Geometry->SetGeometryData(sourceObj.CoreArr);  //Entire geometry table is copied by value

    //Normals
    VertexNormals->data=sourceObj.VnArr;           //Copy normals array

    //UVs
    TextureCoord->data=sourceObj.VtArr;            //Copy UVs array

    //TODO: If only one material and no textures, just use default color (no color array)
    //TODO: If all materials have textures (no color array)

    unsigned TexturedSrfCount=sourceFile.GetCountOfTexturedSrf(ObjIdx); //How many textures this object is using
    if (TexturedSrfCount>0) {TypeGLtexture* defTex=nullptr; InflateAttribArray(LinkImageToSurf,defTex,Geometry->GetNumberOfSurfaces(),true);}

    //Materials (surface)
    unsigned matSurfCount=sourceObj.MatForSurface.size(); //assume matForSurface and surfaces arrays are always synchronized during file import
    //TODO what if matSurfCount==0 ?

    //This is a constructor: The GLobject is EMPTY (there may be other objects in the scene, but this one is empty)
    for (unsigned surfIdx=0;surfIdx<matSurfCount;surfIdx++) //Inside the source object
    {
        //Get the material name from the source
        unsigned FileMatIdx=sourceObj.MatForSurface[surfIdx];
        string   MatName=sourceFile.GetMaterials()[FileMatIdx].Name;

        //Find the material in the scene object
        TypeGLmaterial* OneMaterial=nullptr;
        unsigned Len=MatList.ListSize();
        for (unsigned i=0;i<Len;i++) {if (MatList[i]->Data.MatData.Name==MatName) {OneMaterial= &(MatList[i]->Data); break;}}

        if (OneMaterial==nullptr) LinkImageToSurf->data[surfIdx]=nullptr;
        else if (OneMaterial->TexData!=nullptr)
        {
            LinkImageToSurf->data[surfIdx]=OneMaterial->TexData;
        }
        else
        {
            SetColorForSurf(surfIdx,{OneMaterial->MatData.Kd,OneMaterial->MatData.d});
        }

    }
    //TODO compute ColorArray for lines, polylines, etc

}

//Pre-existing objects assignment (this is tricky)
void TypeGLObject::operator= (const TypeGLObject& Other)
{
    cout<<"Object to object assignment is not yet supported"<<endl;
    //Copy master - master
    //What happens to all instance objects linking to this ? (their private arrays need to be scraped)
    //As a matter of fact all linking instance objects are now useless
    //Suggest to destroy all instanced objects; there needs to be a method to do so

    //Copy instance - instance
    //An instance could itself be master to another instance; all its children need to do apoptosis


}

//---MISC methods---------------------------------------------------------------------------------------------
void TypeGLObject::Draw (const TypeTmatrix& ProjViewMatrix, const TypeGlShaderVar& ShaderVar)
{
    if (isVisible==false) return; //Do not draw it

    EnsureArraysSync();  //In case geometry was added by any of the instanced objects (thus the attribute arrays might be out of sync with the coords array)
    GenerateGLbuffers(); //Make sure GL buffers are up and running

    if ((*GeomBufferIDs)[0].isActive==false) return; //Nothing to do if the are no vertex coordinates to begin with

    //The model matrix is now being factored in (since this is per object)
    TypeTmatrix ModelMatrix = GetCompleteTmatrix();
    TypeTmatrix PVMmatrix = ProjViewMatrix * ModelMatrix;
    //In the next two lines we are leaving ROW-major space and entering column-major space for the shader -->
    //convert Tmatrix data from double to GLfloat and transpose before passing them
    GLfloat floatPVMmatrix[16]; for (int i=0;i<16;i++) {floatPVMmatrix[i]=(GLfloat) PVMmatrix.data[i%4][i/4];}
    glUniformMatrix4fv (ShaderVar.MatrixVarID,1,GL_FALSE,floatPVMmatrix); //GL_FALSE because it got transposed above

    //TODO Check to make sure if this is enough or if it needs the inverse of the model matrix
    GLfloat floatModelmatrix[16]; for (int i=0;i<16;i++) {floatModelmatrix[i]=(GLfloat) ModelMatrix.data[i%4][i/4];}
    glUniformMatrix4fv (ShaderVar.NormalsMatrixVarID,1,GL_FALSE,floatModelmatrix); //GL_FALSE because it got transposed above

    //Depending on how many of the element buffers are active
    //There will be as many draw calls. Ideally each object has only one type of geometry

    /*INFO: Difference between glEnableVertexAttribArray and glEnableVertexArrayAttrib
    a) The glEnableVertexAttribArray uses currently bound vertex array object for the operation
    b) The glEnableVertexArrayAttrib updates state of the vertex array object with ID vaobj.
    c) The attrib pointer 'index' will get its attribute data from whatever buffer object is currently bound to GL_ARRAY_BUFFER*/

    //Info2: the centroid does not get drawn because the element arrays do not reference it. It does not participate in building object features

    //Vertex Coordinates
    glBindBuffer (GL_ARRAY_BUFFER,(*GeomBufferIDs)[0].ID);  //connect the coords buffer object to the GL_ARRAY_BUFFER docking point
    glEnableVertexAttribArray(0);                           //Enable coord data streaming from the currently bound buffer object
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);       //Read the data in float triplets (x,y,z)
    //the above will be used similarly for normals and UVs

    //Vertex Color
    if ( VertexColors->GLbuffer.isActive==true )
    {
        glBindBuffer (GL_ARRAY_BUFFER,VertexColors->GLbuffer.ID);  //connect the color buffer object to the GL_ARRAY_BUFFER docking point
        glEnableVertexAttribArray(1);                              //Enable color data streaming from the currently bound buffer object
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);          //Read the data in float quads (a,b,c,d)
    }

    //Vertex Normals
    if (VertexNormals->GLbuffer.isActive==true)
    {
        glBindBuffer (GL_ARRAY_BUFFER,VertexNormals->GLbuffer.ID);
        glEnableVertexAttribArray(2);                              //Vertex shader location 2 variable
        glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
    }

    //Vertex UV coordinates.
    if (TextureCoord->GLbuffer.isActive==true)
    {
        glBindBuffer (GL_ARRAY_BUFFER,TextureCoord->GLbuffer.ID);
        glEnableVertexAttribArray(3);                              //Vertex shader location 3 variable
        glVertexAttribPointer (3,2,GL_FLOAT,GL_FALSE,0,0);
    }

    //Vertex-Texture-unit selector array.
    if (LinkImageToSurf->GLbuffer.isActive==true)
    {
        glBindBuffer (GL_ARRAY_BUFFER,LinkImageToSurf->GLbuffer.ID);
        glEnableVertexAttribArray(4);                              //Vertex shader location 4 variable
        glVertexAttribIPointer (4,1,GL_INT,0,0);                   //Notice this is the 'I' version (important)

        glUniform1i (ShaderVar.UseTexturesVarID,true);

        //When there is a selector array there must be textures. So, ...
        //Setup the texture units
        unsigned texCount=ImageList.size();
        for (unsigned i=0;i<texCount;i++)
        {
            if (i>5) break; //Will not handle more than six for now;
            glActiveTexture (GL_TEXTURE0+i);
            glBindTexture(GL_TEXTURE_2D,ImageList[i]->GLbuffer.ID);
            glUniform1i (ShaderVar.TexUnitVarID[i],/*GL_TEXTURE0*/ i); //ID of the uniform, the index of the uniform
            //Using array TexUnitVarID[i] because GL/EL mentions that the IDs of uniform arrays may not be sequential so A[0]+1 may not be A[1]
        }


    } else {glUniform1i (ShaderVar.UseTexturesVarID,false);}

    //This object, depending on the geometry it contains, might make several draw calls
    //Each draw call can have it's own default color sent to the uniform variable
    //Now pass the default color values to the shader variable
    TypeXYZw defColor=-1;
    if ( GetIsDefaultColorUsed() ==true ) {defColor=defaultColor;}
    glUniform4f (ShaderVar.DefaultColorVarID,(float)defColor.a,(float)defColor.b,(float)defColor.c,(float)defColor.d);

    //Drawing ELEMENT arrays
    if ( (*GeomBufferIDs)[1].isActive==true ) //Points index array
    {
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[1].ID);
        glDrawElements(GL_POINTS,(*GeomBufferIDs)[1].ElementCount,GL_UNSIGNED_INT,nullptr);
    }
    if ( (*GeomBufferIDs)[2].isActive==true ) //Lines index array
    {
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[2].ID);
        glDrawElements(GL_LINES,(*GeomBufferIDs)[2].ElementCount,GL_UNSIGNED_INT,nullptr);
    }
    if ( (*GeomBufferIDs)[3].isActive==true ) //Polylines index array
    {
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[3].ID);
        glDrawElements(GL_LINE_STRIP,(*GeomBufferIDs)[3].ElementCount,GL_UNSIGNED_INT,nullptr);
    }
    if ( (*GeomBufferIDs)[4].isActive==true ) //Surface (triangles) index array
    {
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[4].ID);
        if (isWireframe==true)
        {
            defColor=WireframeColor;
            glUniform4f (ShaderVar.DefaultColorVarID,(float)defColor.a,(float)defColor.b,(float)defColor.c,(float)defColor.d);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glDrawElements(GL_TRIANGLES,(*GeomBufferIDs)[4].ElementCount,GL_UNSIGNED_INT,nullptr);
    }
    if ( (*GeomBufferIDs)[5].isActive==true && showSrfEdges==true) //Surface Edges index array
    {
        defColor=SrfEdgeColor;
        glUniform4f (ShaderVar.DefaultColorVarID,(float)defColor.a,(float)defColor.b,(float)defColor.c,(float)defColor.d);

        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER,(*GeomBufferIDs)[5].ID);
        glDrawElements(GL_LINE_STRIP,(*GeomBufferIDs)[5].ElementCount,GL_UNSIGNED_INT,nullptr);
    }

    //TODO: handle glDrawElementsInstanced (in case the particles array has more than one particles)

}

//---SET methods ---------------------------------------------------------------------------------------------
void TypeGLObject::SetWireframe (bool isShown, TypeXYZw color)
{
    isWireframe=isShown;
    if (isShown==true) showSrfEdges=false;  //These two are mutually exclusive
    if (color.isValid()==true) WireframeColor=color;
}

void TypeGLObject::SetSurfaceEdges(bool isShown, TypeXYZw color, float edgeWidth)
{
    showSrfEdges=isShown;
    if (isShown==true) isWireframe=false;  //These two are mutually exclusive
    if (color.isValid()==true) SrfEdgeColor=color;

}

void TypeGLObject::SetColorForSurf(unsigned SurfIndex, TypeXYZw NewColor)
{
    unsigned SurfCount=Geometry->GetGeometryTable().Surfaces.size(); if (SurfIndex>SurfCount-1) return;
    unsigned FullSize=Geometry->GetNumberOfCoords()+1;

    if (VertexColors->data.size()<FullSize) InflateAttribArray(VertexColors,defaultColor,FullSize,true);
    else VertexColors->GLbuffer.Reset(); //With new color the current buffer is obsolete

    const TypeSurface& ThisSurface=Geometry->GetGeometryTable().Surfaces[SurfIndex];
    unsigned meshSize=ThisSurface.Mesh.size();
    for (unsigned i=0;i<meshSize;i++)
    {
        unsigned VertIdx=ThisSurface.Mesh[i];
        VertexColors->data[VertIdx]=NewColor;
    }
}

//Apply an image projected as a plane whose size and orientation defined by three points
 void TypeGLObject::SetImageForSurf(unsigned SurfIndex, TypeGLtexture* Texture, TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight, bool maintainAspect)
 {
    unsigned SurfCount=Geometry->GetNumberOfSurfaces();
    if (Texture==nullptr || SurfIndex>=SurfCount) return;

    double aspectRatio=(double)Texture->ImageFile.GetProperties().height/(double)Texture->ImageFile.GetProperties().width;

    //Link the texture to the surface index
    TypeGLtexture* defaultTex=nullptr;
    InflateAttribArray(LinkImageToSurf,defaultTex,SurfCount,true); //first make sure the texture-to-surface array is inflated
    LinkImageToSurf->data[SurfIndex]=Texture; //now it is safe to index into the surface registration array

    //Set up surface UV coordinates (using planar projection)
    //UVs convention LowerLeft=(0,0), UpperRight=(1,1)
    TypeTmatrix Tmatrix=GetCompleteTmatrix(0);
    TypeXY defaultUV={0,0};
    InflateAttribArray(TextureCoord,defaultUV,Geometry->GetNumberOfCoords()+1,true);
    const TypeGeometryRegistry GeometryTable=Geometry->GetGeometryTable();
    unsigned meshSize=GeometryTable.Surfaces[SurfIndex].Mesh.size();
    TypeXYZ TextureZeroBase = BaseRight-BaseLeft;
    TypeXYZ TextureZeroSide = TopLeft-BaseLeft;
    if (maintainAspect==true)
    {
        TextureZeroSide = (TopLeft-BaseLeft).AsOrthoTo (TextureZeroBase);
        TextureZeroSide = TextureZeroSide.ofLength(TextureZeroBase.length()*aspectRatio);
    }

    for (unsigned i=0;i<meshSize;i++)
    {
        unsigned vertIndex=GeometryTable.Surfaces[SurfIndex].Mesh[i];
        TypeXYZ OneVertex = Tmatrix*GeometryTable.Coordinates[vertIndex] - BaseLeft;
        TypeXYZ PRside=OneVertex.ProjectONTO(TextureZeroSide); int signSide = abs(PRside.angleTO(TextureZeroSide))<epsilon? 1:-1;
        TypeXYZ PRbase=OneVertex.ProjectONTO(TextureZeroBase); int signBase = abs(PRbase.angleTO(TextureZeroBase))<epsilon? 1:-1;
        TypeXY  texUV ={signBase*PRbase.length()/TextureZeroBase.length(), signSide*PRside.length()/TextureZeroSide.length()};
        TextureCoord->data[vertIndex]=texUV;
    }
 }

void TypeGLObject::AddGrid(double UnitLength, unsigned UnitCountX, unsigned subdivions, TypeXYZw color)
{
    unsigned density   = UnitCountX*subdivions*2.0;
    double   LineLength= UnitLength*UnitCountX*2.0;
    double   step      = UnitLength/(double) subdivions;
    TypeXYZ  Start     = {-LineLength/2.0,-LineLength/2.0,0.0};
    TypeXYZw finalColor;

    for (unsigned i=0;i<=density;i++)
    {
        //Create a Y line
        Geometry->AddLine({Start.x+(i*step),Start.y,0},{Start.x+(i*step),Start.y+LineLength,0});
        //Create an X line
        Geometry->AddLine({Start.x,Start.y+(i*step),0},{Start.x+LineLength,Start.y+(i*step),0});

        //Handle major vs minor grid-line color
        if (color.isValid()==false) {finalColor=defaultColor;} else {finalColor=color;}
        if (i%subdivions>0) {finalColor = {(1.0+4*finalColor.a)/5.0, (1.0+4*finalColor.b)/5.0, (1.0+4*finalColor.c)/5.0,1.0};} //switch to minor grid line color
        //Since the grid has two colors we need to add them per-vertex at all times.
        //So, finalColor is never none at this point and this grid object will always have a colors array.
        UpdateAttribArray(VertexColors,Geometry->GetNumberOfCoords()+1,4,finalColor,defaultColor,true); //We just added four points earlier, so we will also add four color values now
    }
}

void TypeGLObject::AddLine (TypeXYZ a, TypeXYZ b, TypeXYZw color)
{
    Geometry->AddLine(a,b);                //add geometry
    UpdateAttribArray(VertexColors,Geometry->GetNumberOfCoords()+1,2,color,defaultColor,color.isValid()?true:false);
}

void TypeGLObject::AddNgon (unsigned short sidesCount, TypeXYZ center, TypeXYZ RadiusPoint, TypeXYZ normal,bool inscribed, TypeXYZw color)
{
    Geometry->AddNgon(sidesCount,center,RadiusPoint,normal,inscribed);
    UpdateAttribArray(VertexColors,Geometry->GetNumberOfCoords()+1,sidesCount,color,defaultColor,color.isValid()?true:false);
}

void TypeGLObject::AddTriangleSrf (TypeXYZ a, TypeXYZ b, TypeXYZ c, TypeXYZw color)
{
    Geometry->AddTriangleSrf(a,b,c); //add geometry
    UpdateAttribArray(VertexColors,Geometry->GetNumberOfCoords()+1,3,color,defaultColor,color.isValid()?true:false);
}

void TypeGLObject::AddRectangleSrf(TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight, TypeXYZw color)
{
    Geometry->AddRectangleSrf(TopLeft,BaseLeft,BaseRight);  //add geometry
    UpdateAttribArray(VertexColors,Geometry->GetNumberOfCoords()+1,4,color,defaultColor,color.isValid()?true:false);
}

//Import
bool TypeGLObject::ImportGeometry (const TypeOBJData& source)
{
    cout<<"Adding geometry on top of existing has not been implemented yet"<<endl; return false;
}

//---GET methods ---------------------------------------------------------------------------------------------
TypeTmatrix TypeGLObject::GetCompleteTmatrix (unsigned particleIdx) const
{
    //Roll in all the Tmatrices for this object and all the groups that affect it
    TypeTmatrix result=ParticlesArr[particleIdx].Tmatrix;
    TypeGLGroup* level=ParentGroup;
    while (level!=nullptr)
    {
        result = level->Tmatrix * result;
        level = level->GetParent();
    }

    return result;
}

//------------------------------------------------------------------------------------------------------------

//***TypeGLtexture implementation*****************************************************************************
//************************************************************************************************************
void TypeGLtexture::LoadTexture (string FileName)
{
    unsigned Len=FileName.size();
    if (Len<5) {cout<<"Filename too short (not likely any supported image type)"; return;}

    string extension=FileName.substr(Len-3,3);
    for (unsigned i=0;i<3;i++) {if ((int)extension[i]>90) extension[i]-=32;}
    if (extension!="BMP") {cout<<"Only *.BMP images are currently supported"; return;}

    ImageFile.LoadBMPimage(FileName);
    GenerateGLbuffer();
}

void TypeGLtexture::GenerateGLbuffer   ()
    {
        if (GLbuffer.isActive==true || ImageFile.GetPixelArray().size()==0) return;
        GLbuffer.isActive=true;
        GLbuffer.isTexture=true;
        GLbuffer.Name=ImageFile.GetProperties().FileName;
        GLbuffer.ElementCount=ImageFile.GetPixelArray().size();

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);       //Important if your texture size is not power of two
        glPixelStorei(GL_PACK_ALIGNMENT, 1);         //Important if your texture size is not power of two

        glGenTextures (1,&GLbuffer.ID);              //instantiate ONE buffer object and return its handle/ID
        glBindTexture (GL_TEXTURE_2D,GLbuffer.ID);   //connect the object to the GL_TEXTURE_2D docking point
        glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,ImageFile.GetProperties().width, ImageFile.GetProperties().height,0,GL_RGBA,GL_UNSIGNED_BYTE,&(ImageFile.GetPixelArray()[0]));
        //The image will begin loading at coordinate 0,0
        //2nd param, 0 used to load your own mipmaps instead of letting GL do them
        //3rd param, GL_RGB to be used to store the image internally,
        //6th param, 0 - this should always be zero as per specification
        //7th param, GL_RGB is how the original image is to be presented

        //Idea: This may need to go on a separate method with other texture parameters
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  //Essential: without this the texture will be black
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  //Essential: without this the texture will be black
    }
//------------------------------------------------------------------------------------------------------------

//***TypeGLscene implementation*******************************************************************************
//************************************************************************************************************

//Constructor
TypeGLscene::TypeGLscene()
{
    TypeGLmaterial SceneDefault("Scene Default");
    Materials.PushBack(SceneDefault);
    AmbientColor={1,1,1,1}; //pure white

    InstalShaders ();

    //Scene-wide enable flags
    glEnable(GL_DEPTH_TEST);
}

bool TypeGLscene::ImportOBJfile (string ThisFile)
{
    TypeOBJfile OBJfile; //This data is local and will disappear when this function returns
    if (!OBJfile.LoadDataFrom(ThisFile)) return false;

    //Transfer textures into the scene (adding to what is already there)
    unsigned TexCount=OBJfile.GetTextures().size();
    for (unsigned i=0;i<TexCount;i++)
    {
        TypeGLtexture* LookupTex=GetTexture(OBJfile.GetTextures()[i].GetFileName());
        if (LookupTex!=nullptr) {continue;} //texture is already there

        TypeGLtexture OneTexture;
        Textures.PushBack(OneTexture);
        Textures.Back()->Data.ImageFile=OBJfile.GetTextures()[i];
        Textures.Back()->Data.GenerateGLbuffer();
    }

    //Transfer materials into the scene (adding to what is already there)
    unsigned MatCount=OBJfile.GetMaterials().size();
    for (unsigned i=0;i<MatCount;i++)
    {
        TypeGLmaterial* LookupMat=GetMaterial(OBJfile.GetMaterials()[i].Name);
        if (LookupMat!=nullptr) continue; //Material already exists
        //FIX: If a textured material exists from a previous OBJ load and is replaced by an untextured one will cause a crash
        //TODO: Generate material unique name

        TypeGLmaterial OneMaterial;
        Materials.PushBack(OneMaterial);
        Materials.Back()->Data.MatData=OBJfile.GetMaterials()[i];
        Materials.Back()->Data.TexData=GetTexture(OBJfile.GetMaterials()[i].TextureFile);
    }

    //Transfer objects, materials, and textures
    unsigned ObjectCount=OBJfile.GetObjects().size();
    for (unsigned thisOne=0;thisOne<ObjectCount;thisOne++)      //Take every object in the obj file
        {ObjectList.emplace_back(OBJfile, thisOne, Materials);} //invoke import constructor

    return true; //OBJfile data is now destroyed (make sure you got what you needed)
}


//Private methods---------------------------------------------------------------------------------------------

//Poor: These sequential lookups have poor performance
TypeGLmaterial* TypeGLscene::GetMaterial (const string& lookupName)
{
    unsigned Len=Materials.ListSize();
    for (unsigned i=0;i<Len;i++) {if (Materials[i]->Data.MatData.Name==lookupName) return &(Materials[i]->Data);}
    return nullptr;
}

//Poor: These sequential lookups have poor performance
TypeGLtexture*  TypeGLscene::GetTexture (const string& lookupFileName)
{
    unsigned Len=Textures.ListSize();
    for (unsigned i=0;i<Len;i++) {if (Textures[i]->Data.ImageFile.GetFileName()==lookupFileName) return &(Textures[i]->Data);}
    return nullptr;
}

string TypeGLscene::LoadGLshader (string ThisFile)
{
    ifstream fromFile;
    fromFile.open(ThisFile);
    if (fromFile.good()==false)
    {
        cout<<"Could not find file: "<<ThisFile<<endl;
        exit(1);
    }

    string  FileContents, oneLine;

    while (fromFile.good()==true)
    {
        getline(fromFile,oneLine);
        FileContents += (oneLine + "\n");
    }
    fromFile.close();
    //FileContents=FileContents.c_str();
    return FileContents;
}

bool TypeGLscene::ShaderCompileStatus (GLint ShaderID, string filename)
{
    bool result=true;
    GLint CompileStatus;
    glGetShaderiv(ShaderID,GL_COMPILE_STATUS,&CompileStatus);

    if (CompileStatus==GL_FALSE)
    {
        result=false;

        cout<<"Compile Error in: "<<filename<<endl;
        GLint LogLen;
        glGetShaderiv(ShaderID,GL_INFO_LOG_LENGTH,&LogLen);

        GLchar* ErrorMessage = new GLchar [LogLen]; GLsizei returnedLogLen;
        glGetShaderInfoLog(ShaderID,LogLen,&returnedLogLen,ErrorMessage);

        cout<<ErrorMessage<<endl;
        delete [] ErrorMessage;
    }
    return result;
}

bool TypeGLscene::ShaderLinkingStatus(GLint ProgramID)
{
    bool result=true;
    GLint LinkingStatus;
    glGetProgramiv(ProgramID,GL_LINK_STATUS,&LinkingStatus);

    if (LinkingStatus==GL_FALSE)
    {
        result=false;

        cout<<"Shader linking Error"<<endl;
        GLint LogLen;
        glGetProgramiv(ProgramID,GL_INFO_LOG_LENGTH,&LogLen);

        GLchar* ErrorMessage = new GLchar [LogLen]; GLsizei returnedLogLen;
        glGetProgramInfoLog(ProgramID,LogLen,&returnedLogLen,ErrorMessage);

        cout<<ErrorMessage<<endl;
        delete [] ErrorMessage;
    }
    return result;
}

void TypeGLscene::InstalShaders ()
{
    string file, fileContent;
    GLchar* ShaderChapters[1];

    //Vertex Shader. Load and compile.
    file="ProgVertexShader.glsl";
    fileContent=LoadGLshader(file);
    ShaderChapters[0]=&fileContent[0];
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShaderID,1,ShaderChapters,0);
    glCompileShader(VertexShaderID); if (ShaderCompileStatus(VertexShaderID, file)==false) return;

    //Fragment shader. Load and compile.
    file="ProgFragmentShader.glsl";
    fileContent=LoadGLshader(file);
    ShaderChapters[0]=&fileContent[0];
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShaderID,1,ShaderChapters,0);
    glCompileShader(FragmentShaderID); if(ShaderCompileStatus(FragmentShaderID, file)==false) return;

    //Make Shader program
    ShaderVar.ProgramID = glCreateProgram();
    glAttachShader (ShaderVar.ProgramID,VertexShaderID);
    glAttachShader (ShaderVar.ProgramID,FragmentShaderID);
    glLinkProgram  (ShaderVar.ProgramID); if (ShaderLinkingStatus(ShaderVar.ProgramID)==false) return;

    glUseProgram (ShaderVar.ProgramID);

    GetUniformVarIDs(); //Get the uniform variable handles
}

void TypeGLscene::GetUniformVarIDs ()
{

    ShaderVar.MatrixVarID        =glGetUniformLocation(ShaderVar.ProgramID,"ProjViewModelMatrix");
    ShaderVar.DefaultColorVarID  =glGetUniformLocation(ShaderVar.ProgramID,"DefaultColor");
    ShaderVar.UseTexturesVarID   =glGetUniformLocation(ShaderVar.ProgramID,"useTextures");
    ShaderVar.NormalsMatrixVarID =glGetUniformLocation(ShaderVar.ProgramID,"NormalsMatrix");
    ShaderVar.AmbientColorVarID  =glGetUniformLocation(ShaderVar.ProgramID,"AmbientColor");
    ShaderVar.LightColorVarID    =glGetUniformLocation(ShaderVar.ProgramID,"LightColor");
    ShaderVar.LightPositionVarID =glGetUniformLocation(ShaderVar.ProgramID,"LightPosition");
    ShaderVar.LightIntensityVarID=glGetUniformLocation(ShaderVar.ProgramID,"LightIntensity");

    for(int i=0;i<5;i++)       //Each of the six texture unit variable IDs (for larger integers use stringstream)
    {ShaderVar.TexUnitVarID[i] =glGetUniformLocation(ShaderVar.ProgramID,(string("TextureUnit[")+char(48+i)+"]").c_str());}

    //cout<<"TexUnitVarID"<<ShaderVar.TexUnitVarID<<endl;
    //cout<<"UseTexturesVarID"<<ShaderVar.UseTexturesVarID<<endl;

    //GLint iUnits; glGetIntegerv(GL_MAX_TEXTURE_UNITS, &iUnits); cout<<"Support "<<iUnits<<" tex units"<<endl;
    //INFO: for the ID of non-uniform variables use glGetAttribLocation
}

//Public methods
void TypeGLscene::Draw ()
{

    //Sent light information to the shader
    float    ShaderLightIntensity;
    TypeXYZ  LightPos = Light.Position;
    if (Light.isRelToCam==true)
    {
       //LightPos = LightPos + ActiveCamera->GetEyePos();
       TypeXYZ CamZ=ActiveCamera->GetEyePos() - ActiveCamera->GetTarget();
       TypeXYZ LightPosZ = CamZ.ofLength(Light.Position.z);
       TypeXYZ LightPosY = ActiveCamera->GetUpDir().AsOrthoTo(CamZ).ofLength(Light.Position.y);
       TypeXYZ LightPosX = CamZ.CrossProduct(ActiveCamera->GetUpDir()).ofLength(Light.Position.x);
       LightPos = LightPosX + LightPosY + LightPosZ + ActiveCamera->GetEyePos();
    }

    glUniform4f (ShaderVar.AmbientColorVarID,(float)AmbientColor.a,(float)AmbientColor.b,(float)AmbientColor.c,(float)AmbientColor.d);
    glUniform4f (ShaderVar.LightColorVarID,(float)Light.Color.a,(float)Light.Color.b,(float)Light.Color.c,(float)Light.Color.d);
    glUniform3f (ShaderVar.LightPositionVarID,(float)LightPos.x,(float)LightPos.y,(float)LightPos.z);

    //Compute the matrix
    //This is the same for the whole scene, so it's only computed once up here.
    TypeTmatrix ProjViewMatrix = ActiveCamera->ProjectionMatrix * ActiveCamera->ViewMatrix ;

    unsigned ObjCount=ObjectList.size();
    for (unsigned i=0;i<ObjCount;i++)
    {
        //Send appropriate light intensity for this object
        if (Light.isON==false || ObjectList[i].ExcludeFromLights==true) {ShaderLightIntensity=-1;}else{ShaderLightIntensity=Light.Intensity;}
        glUniform1f (ShaderVar.LightIntensityVarID,ShaderLightIntensity);

        //Draw the object
        ObjectList[i].Draw(ProjViewMatrix,ShaderVar); //ProjView is still in row-major order
    }
}

long long TypeGLscene::GetObjectIndex (string LookupByName)
{
    unsigned scenePopulation = ObjectList.size();
    if (scenePopulation==0 || LookupByName=="") return -1;

    for (unsigned i=0;i<scenePopulation;i++) {if (ObjectList[i].ObjectName==LookupByName) return i;}

    return -1;
}

//***Class GLwindow implementation****************************************************************************
//************************************************************************************************************

//Constructor
TypeGLwindow::TypeGLwindow(int& argc, char* argv[], string Title, short width, short height, short xPos, short yPos)
{
    glutInit(&argc,argv);

    glutInitContextVersion(3.3, 0);
    //The next three go together
    //glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); //This is an unnecessary feature. Avoid enabling it.
    //glutInitContextFlags(GLUT_DEBUG);
    //glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
    glutInitContextProfile ( GLUT_CORE_PROFILE );
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitWindowSize(width,height);
    glutInitWindowPosition(xPos,yPos);
    windowHandle=glutCreateWindow(Title.c_str());

    if(windowHandle < 1) {cout<<"ERROR: Could not create a new rendering window."<<endl; exit(1);}
    else {cout<<"INFO: OpenGL Version:"<<glGetString(GL_VERSION)<<endl<<"FreeGLUT version:"<<glutGet(GLUT_VERSION)<<endl;}

    GLenum GlewInitResult;
    glewExperimental = GL_TRUE; //otherwise glGenVertexArrays will cause a hang
    GlewInitResult = glewInit(); //glew adds openGL 4.0 functions

    if (GLEW_OK != GlewInitResult) {cout<<"Error:"<<glewGetErrorString(GlewInitResult)<<endl; exit(EXIT_FAILURE);}

    glClearColor(1,1,1,0);  //Set the initial erasing color
    glColor3f(0,0,0);       //Set the initial fill color
}
