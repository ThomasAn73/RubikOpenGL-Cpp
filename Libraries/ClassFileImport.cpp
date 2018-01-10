//***Class OBJ file implementation****************************************************************************
//************************************************************************************************************
void TypeOBJMat::Print (string Comment)
{
    //Save original settings
    int old_precision = cout.precision(); ios::fmtflags old_settings  = cout.flags();

    cout<<Comment<<endl;
    cout.setf(ios::fixed, ios::floatfield); cout.precision(3);
    cout<<"Material: "<<Name<<endl;
    cout<<"Ka = ("<<setw(5)<<Ka.x<<", "<<setw(5)<<Ka.y<<", "<<setw(5)<<Ka.z<<")"<<endl;
    cout<<"Kd = ("<<setw(5)<<Kd.x<<", "<<setw(5)<<Kd.y<<", "<<setw(5)<<Kd.z<<")"<<endl;
    cout<<"Ks = ("<<setw(5)<<Ks.x<<", "<<setw(5)<<Ks.y<<", "<<setw(5)<<Ks.z<<")"<<endl;
    cout<<"Tf = ("<<setw(5)<<Tf.x<<", "<<setw(5)<<Tf.y<<", "<<setw(5)<<Tf.z<<")"<<endl;
    cout<<" d =  "<<setw(5)<<d<<endl;
    cout<<"Ns =  "<<setw(5)<<Ns<<endl;
    //restore original settings
    cout.flags(old_settings); cout.precision(old_precision);
}
//----------------------------------------------------------------------------------------------------------

//PRIVATE methods
string TypeOBJfile::CleanString (string& ThisStr,char ignore)
{   //Remove everything after the ignore char
    unsigned Len=ThisStr.size(); if (Len==0) return "";
    string result=ThisStr;
    for (unsigned i=0;i<Len;i++) if (ThisStr[i]==ignore) ThisStr=ThisStr.substr(0,i);
    return ThisStr;
}

void TypeOBJfile::TokenizeString (const string& ThisStr, vector<string>& result, char delim, int splits)
{   //split the string in words using the delimiter
    unsigned Len=ThisStr.size(); if (Len==0) return;
    unsigned i=-1, fromIdx=0;
    while (++i<Len && splits!=0)
        {if (ThisStr[i]==delim) {result.push_back(ThisStr.substr(fromIdx,i-fromIdx)); splits--; fromIdx=i+1;}}
    if(fromIdx<Len) result.push_back(ThisStr.substr(fromIdx)); else result.push_back("");
}

void TypeOBJfile::ResetData()
{
    Objects.clear();
    Materials.clear();
    Textures.clear();
    vVtVnPattern={{false,false,false}};
    LastMaterial=0;
    VertexCount=0;
}

bool TypeOBJfile::ExtractvVtVn  (const vector<string>& fromTokenList,unsigned ThisOne,array<float,3>& vVtVn)
{
    //format v/vt/vn (expect all three values to be the same)
    //Non uniform triplet indexes add unnecessary complexity, I will not even bother with it.
    vector<string> vVtVnTokens; TokenizeString(fromTokenList[ThisOne],vVtVnTokens,'/');
    unsigned vVtVnTokenCount=vVtVnTokens.size();

    vVtVn={{NAN,NAN,NAN}};
    for (unsigned j=0;j<vVtVnTokenCount;j++)
    {
        if (vVtVnTokens[j]==""){vVtVn[j]=NAN;} else {vVtVn[j]=atof(vVtVnTokens[j].c_str());}

        if (j==0) {if (!isfinite(vVtVn[0])) {cout<<"Dicoverred facet with no vertex coordinates"; return false;}}
        else if (isfinite(vVtVn[j]) && vVtVn[j]!=vVtVn[0]) {cout<<"Cannot handle face triplets with asynchronous indices"; return false;}

        if (Objects.size()==1 && Objects[0].CoreArr.Surfaces.size()==1 && ThisOne==0) {isfinite(vVtVn[j])? vVtVnPattern[j]=true:vVtVnPattern[j]=false;}
        else if ( !((isfinite(vVtVn[j]) && vVtVnPattern[j]==true) || (!isfinite(vVtVn[j]) && vVtVnPattern[j]==false)) )
            {cout<<"Discovered vertex triplets of conflicting pattern"; return false;}
    }
    for (unsigned j=vVtVnTokenCount;j<3;j++) {if (vVtVnPattern[j]==true) {cout<<"Discovered non-uniformly defined vertex triplets"<<endl; return false;}}
    return true;
}

int TypeOBJfile::GetMaterialIdx (string lookupName)
{
    unsigned Len=Materials.size();
    for (unsigned i=0;i<Len;i++) {if (Materials[i].Name==lookupName) return i;}
    return -1;
}

int  TypeOBJfile::GetTextureIdx (string lookupFileName)
{
    unsigned Len=Textures.size();
    for (unsigned i=0;i<Len;i++) {if (Textures[i].GetFileName()==lookupFileName) return i;}
    return -1;
}

void TypeOBJfile::EnsureDefaultObject()
{
    if (Objects.size()==0) {Objects.emplace_back();}
    if (Materials.size()==0) {LastMaterial=0; Materials.emplace_back("DefaultMaterial"); }
}

//This loads all the materials for all objects in the OBJ file
void TypeOBJfile::LoadMaterialsFrom (string ThisFile)
{
    ifstream fromMatFile; fromMatFile.open(ThisFile);
    if (fromMatFile.good()==false) { cout<<"Could not find file: "<<ThisFile<<endl; exit(1); }

    string  oneLine;
    vector<string> tokensArr;
    vector<string> localTokens;

    while (fromMatFile.good()==true)
    {
        getline(fromMatFile,oneLine); CleanString(oneLine);
        if(oneLine.size()==0) continue;
        TokenizeString(oneLine,tokensArr,' ',1);

        unsigned tokenCount=tokensArr.size();
        if (tokenCount!=2) continue;
        if (tokensArr[0]=="newmtl")
        {
            //Will not be checking if a material is already listed twice.
            //Everything is added as it comes in the file.
            Materials.emplace_back();
            Materials.back().Name=tokensArr[1];
        }
        else if (tokensArr[0]=="Ka")
        {
            TokenizeString(tokensArr[1],localTokens,' ');
            Materials.back().Ka.x =atof (localTokens[0].c_str());
            Materials.back().Ka.y =atof (localTokens[1].c_str());
            Materials.back().Ka.z =atof (localTokens[2].c_str());
            localTokens.clear();
        }
        else if (tokensArr[0]=="Kd")
        {
            TokenizeString(tokensArr[1],localTokens,' ');
            Materials.back().Kd.x =atof (localTokens[0].c_str());
            Materials.back().Kd.y =atof (localTokens[1].c_str());
            Materials.back().Kd.z =atof (localTokens[2].c_str());
            localTokens.clear();
        }
        else if (tokensArr[0]=="Ks")
        {
            TokenizeString(tokensArr[1],localTokens,' ');
            Materials.back().Ks.x =atof (localTokens[0].c_str());
            Materials.back().Ks.y =atof (localTokens[1].c_str());
            Materials.back().Ks.z =atof (localTokens[2].c_str());
            localTokens.clear();
        }
        else if (tokensArr[0]=="Tf")
        {
            TokenizeString(tokensArr[1],localTokens,' ');
            Materials.back().Tf.x =atof (localTokens[0].c_str());
            Materials.back().Tf.y =atof (localTokens[1].c_str());
            Materials.back().Tf.z =atof (localTokens[2].c_str());
            localTokens.clear();
        }
        else if (tokensArr[0]=="d") {Materials.back().d =atof (tokensArr[1].c_str());}
        else if (tokensArr[0]=="Ns"){Materials.back().Ns =atof (tokensArr[1].c_str());}
        else if (tokensArr[0]=="map_Kd")
        {
            //What if the texture file is already loaded
            int TexIdx=GetTextureIdx(tokensArr[1]);
            if (TexIdx<0)
            {   //Texture not found, add new one.
                TypeImageFile oneTexture;
                Textures.push_back(oneTexture);
                TexIdx=Textures.size()-1;
                Textures.back().LoadBMPimage(tokensArr[1]);
            }

            //Materials.back().TextureIdx=TexIdx;
            //Materials.back().Texture=&(Textures[TexIdx]->Data);
            Materials.back().TextureFile=Textures[TexIdx].GetFileName();

        }
        tokensArr.clear();
    }
    fromMatFile.close();
}

//PUBLIC methods--------------------------------------------------------------------------------------
//OBJ file reading happens here
bool TypeOBJfile::LoadDataFrom (string ThisFile)
{
    ResetData();
    ifstream fromMainFile; fromMainFile.open(ThisFile);
    if (fromMainFile.good()==false) { cout<<"Could not find file: "<<ThisFile<<endl; exit(1); }

    TypeSurface*   CurrentSrf     = nullptr;
    bool           StartNewFeature= false;
    string         NewFeatureName = "";
    string         oneLine;
    vector<string> tokensArr;

    while (fromMainFile.good()==true)
    {
        tokensArr.clear();
        getline(fromMainFile,oneLine); if(CleanString(oneLine,'#').size()==0) continue;
        TokenizeString(oneLine,tokensArr,' ',1);
        if (tokensArr.size()==0) continue;
        if (tokensArr.size()==1) tokensArr.push_back("");
        if (tokensArr[0]=="mtllib")
        {
            LoadMaterialsFrom(tokensArr[1]);
            continue;
        }
        if (tokensArr[0]=="g")  //groups are translated as new objects
        {
            Objects.emplace_back();
            Objects.back().Name=tokensArr[1];
            continue;
        }
        if (tokensArr[0]=="o")  //objects signify a new geometric feature (a surface, a line, etc)
        {
            StartNewFeature=true;
            NewFeatureName=tokensArr[1];
            continue;
        }
        if (tokensArr[0]=="usemtl")
        {   //Look in the materials array to find the name of the material
            int MatIdx=GetMaterialIdx(tokensArr[1]);
            if (MatIdx<0) {cout<<"Tried to assign non existent material"<<endl; return false;}
            LastMaterial=MatIdx;    //This is now the last active material
            continue;
        }
        if (tokensArr[0]=="v") //vertex coordinates
        {
            if (StartNewFeature==false) {StartNewFeature=true; NewFeatureName="";}
            EnsureDefaultObject();  //Make sure there is at least one object created already
            VertexCount++;          //counts the vertices found in the file
            //Bookmark the file vertex number at the start of this object
            if (Objects.back().CoreArr.Coordinates.size()==1) {Objects.back().FileVertIdx=VertexCount; }
            vector<string> localTokens; TokenizeString(tokensArr[1],localTokens,' ');
            TypeXYZ coords;
            coords.x=atof(localTokens[0].c_str());
            coords.y=atof(localTokens[1].c_str());
            coords.z=atof(localTokens[2].c_str());
            Objects.back().CoreArr.Coordinates.push_back(coords);
            continue;
        }

        //Anything bellow this point requires an existing object to work on
        if (Objects.size()==0)
        {
            cout<<"Corrupt file: No vertex coordinates in the proper location";
            return false;
        }

        if (tokensArr[0]=="vt")
        {
            vector<string> localTokens; TokenizeString(tokensArr[1],localTokens,' ');
            unsigned len=localTokens.size();

            TypeXY tuple2;
            if(len>0) tuple2.x=atof(localTokens[0].c_str());
            if(len>1) tuple2.y=atof(localTokens[1].c_str());
            if (Objects.back().VtArr.size()==0) {Objects.back().VtArr.push_back({NAN,NAN}); }
            Objects.back().VtArr.push_back(tuple2);
            continue;
        }
        if (tokensArr[0]=="vn")
        {
            vector<string> localTokens; TokenizeString(tokensArr[1],localTokens,' ');
            unsigned len=localTokens.size();

            TypeXYZ tuple3;
            if(len>0) tuple3.x=atof(localTokens[0].c_str());
            if(len>1) tuple3.y=atof(localTokens[1].c_str());
            if(len>2) tuple3.z=atof(localTokens[2].c_str());
            if (Objects.back().VnArr.size()==0) {Objects.back().VnArr.push_back({NAN,NAN,NAN}); }
            Objects.back().VnArr.push_back(tuple3);
            continue;
        }

        //At this point if the v vt vn arrays are non synchronous quit
        if ( (Objects.back().VtArr.size()>0 && Objects.back().CoreArr.Coordinates.size()!=Objects.back().VtArr.size()) ||
             (Objects.back().VnArr.size()>0 && Objects.back().CoreArr.Coordinates.size()!=Objects.back().VnArr.size()))
        {
            cout<<"Vertex, Texture, and Normals arrays are non synchronous.";
            return false;
        }

        //Features adding section
        if (tokensArr[0]=="f")
        {
            if (StartNewFeature==true) //Is this a new surface in the object ?
            {
                StartNewFeature=false;
                //Create a new surface index array
                Objects.back().CoreArr.Surfaces.emplace_back();
                CurrentSrf=&Objects.back().CoreArr.Surfaces.back();
                CurrentSrf->SurfaceName=NewFeatureName; NewFeatureName="";
                unsigned SurfCount = Objects.back().CoreArr.Surfaces.size();
                //link surface to material
                Objects.back().MatForSurface.push_back(LastMaterial);
                unsigned MatToSurfCount = Objects.back().MatForSurface.size();
                if (SurfCount!=MatToSurfCount) {cout<<"Alignment error (materials and coords array)"<<endl; return false;}
            }
            vector<string> localTokens; TokenizeString(tokensArr[1],localTokens,' ');
            unsigned Len=localTokens.size();
            if (Len<3 || Len>4) {cout<<"Discovered facet which is neither a triangle nor a quad"; return false;}
            for (unsigned i=0;i<Len;i++) //for each face token (always 3, or 4) extract v Vt Vn numbers.
            {
                array<float,3> vVtVn;
                if (ExtractvVtVn(localTokens, i, vVtVn)==false) return false;
                if (i==3) //If it is a quad convert it to two triangles
                {
                    unsigned LastMeshIdx=CurrentSrf->Mesh.size()-1;
                    CurrentSrf->Mesh.push_back(CurrentSrf->Mesh[LastMeshIdx-2]);
                    CurrentSrf->Mesh.push_back(CurrentSrf->Mesh[LastMeshIdx]);
                }
                CurrentSrf->Mesh.push_back(vVtVn[0]-Objects.back().FileVertIdx+1); //add this vertex Coordinate
            }
        }
    }
    fromMainFile.close();
    //PrintInventory();
    return true;
}

int TypeOBJfile:: GetCountOfTexturedSrf  (unsigned ObjectIdx)
{
    int result=0;
    unsigned Len=Objects[ObjectIdx].MatForSurface.size(); if (Len==0) return result;
    for (unsigned i=0;i<Len;i++) {if (Materials[ (Objects[ObjectIdx].MatForSurface[i]) ].TextureFile!="") result++;}
    return result;
}

void TypeOBJfile::PrintInventory()
{
    cout<<"V/Vt/Vn pattern:"<<vVtVnPattern[0]<<"/"<<vVtVnPattern[1]<<"/"<<vVtVnPattern[2]<<endl;

    unsigned ObjCount = Objects.size();
    unsigned MatCount = Materials.size();
    cout<<"Objects: "<<ObjCount<<endl;
    cout<<"Materials: "<<MatCount<<endl<<endl;

    for (unsigned i=0;i<ObjCount;i++) //for each object
    {
        unsigned VertCount =Objects[i].CoreArr.Coordinates.size();
        unsigned PointCount=Objects[i].CoreArr.Points.size();
        unsigned LineCount =Objects[i].CoreArr.Lines.size();
        unsigned PlineCount=Objects[i].CoreArr.Polylines.size();
        unsigned SurfCount =Objects[i].CoreArr.Surfaces.size();

        unsigned VtCount   =Objects[i].VtArr.size();
        unsigned VnCount   =Objects[i].VnArr.size();
        cout<<"Object["<<i<<"] name:"<<Objects[i].Name<<endl<<"Starting file vertex:"<<Objects[i].FileVertIdx<<endl;
        cout<<"count:"<<VertCount<<" vertices"<<endl;
        cout<<"count:"<<PointCount<<" points"<<endl;
        cout<<"count:"<<LineCount<<" lines"<<endl;
        cout<<"count:"<<PlineCount<<" polylines"<<endl;
        cout<<"count:"<<SurfCount<<" surfaces"<<endl;


        for (unsigned j=0;j<VertCount;j++)
        {
            if (j==0) cout<<"Vertices"<<endl;
            Objects[i].CoreArr.Coordinates[j].Print("   v ");
        }

        for (unsigned j=0;j<VtCount;j++)
        {
            if (j==0) cout<<"Texture coordinates"<<endl;
            cout<<"   vt "<<Objects[i].VtArr[j][0]<<" "<<Objects[i].VtArr[j][1]<<endl;
        }

        for (unsigned j=0;j<VnCount;j++)
        {
            if (j==0) cout<<"Normals"<<endl;
            cout<<"   vn "<<Objects[i].VnArr[j][0]<<" "<<Objects[i].VnArr[j][1]<<" "<<Objects[i].VnArr[j][2]<<endl;
        }


        for (unsigned j=0;j<PointCount;j++)
        {
            if (j==0) cout<<"Points"<<endl;
            cout<<Objects[i].CoreArr.Points[j]<<" ";
        }

//        for (unsigned j=0;j<LineCount;j++)
//        {
//            if (j==0) cout<<"Lines"<<endl;
//            Objects[i].CoreArr.Lines[j].Print("l ");
//        }
//
//        for (unsigned j=0;j<PlineCount;j++)
//        {
//            if (j==0) cout<<"Polylines"<<endl;
//            Objects[i].CoreArr.Polylines[j].Print("pl ");
//        }

        for (unsigned j=0;j<SurfCount;j++)
        {
            unsigned SurfMatIdx=Objects[i].MatForSurface[j];
            cout<<"Surface["<<j<<"] using material["<<SurfMatIdx<<"]:"<<Materials[SurfMatIdx].Name<<endl<<"f ";
            unsigned SurfSize=Objects[i].CoreArr.Surfaces[j].Mesh.size();
            for (unsigned k=0;k<SurfSize;k++)
            {
                cout<<Objects[i].CoreArr.Surfaces[j].Mesh[k]<<" ";
            }
            cout<<endl;
        }

    }
}
//----------------------------------------------------------------------------------------------------------

//***Struct Properties implementation*************************************************************************
void TypeImageProperties::Print (const string& comment) const
{
    cout<<comment<<endl;
    cout<<"Image format: "<<ImageFormat<<endl;
    cout<<"Width       : "<<width<<endl;
    cout<<"Height      : "<<height<<endl;
    cout<<"Bit depth   : "<<(int)BitDepth<<endl;
    cout<<"Bytes/pixel : "<<(int)BytesPerPixel<<endl;
    cout<<"Compression : "<<(int)CompressionMethod<<endl;
    cout<<"Color type  : ";
    switch (ColorType)
    {
    case 0:
        cout<<"Greyscale"<<endl; break;
    case 2:
        cout<<"Truecolour"<<endl; break;
    case 3:
        cout<<"Indexed-colour"<<endl; break;
    case 4:
        cout<<"Greyscale with alpha"<<endl; break;
    case 6:
        cout<<"Truecolour with alpha"<<endl; break;
    default:
        cout<<"UNKNOWN"<<endl;
    }

    //CompressionMethod,FilterMethod,InterlaceMethod;
}
//----------------------------------------------------------------------------------------------------------

//***Class Image file implementation**************************************************************************
//************************************************************************************************************
bool TypeImageFile::LoadDataFrom (const string& ThisFile)
{
    ifstream fromImageFile; fromImageFile.open(ThisFile, ifstream::binary);
    if (fromImageFile.good()==false) { cout<<"Could not find file: "<<ThisFile<<endl; return false; }

    unsigned char OneByte;
    while (fromImageFile.good()==true)
    {
        OneByte=fromImageFile.get();
        RawImageBytes.push_back(OneByte);
    }
    fromImageFile.close();
    return true;
}

bool TypeImageFile::isMatch(const vector<unsigned char>& master, unsigned fromIdx)
{
    unsigned len=master.size();
    for (unsigned i=0;i<len;i++) if (master[i]!=RawImageBytes[i+fromIdx]) {return false;}

    return true;
}

unsigned TypeImageFile::BigEndian(unsigned fromIdx, short bytes)
{
    if (fromIdx>RawImageBytes.size()-4) return 0;
    unsigned result=0;
    for (short i=0;i<bytes;i++) result=result<<CHAR_BIT | RawImageBytes[fromIdx+i];
    return result;
}

unsigned TypeImageFile::LittleEndian(unsigned fromIdx, short bytes)
{
    if (fromIdx>RawImageBytes.size()-bytes) return 0;
    unsigned result=0;
    for (short i=0;i<bytes;i++) {result=result<<CHAR_BIT | RawImageBytes[fromIdx+bytes-1-i];};
    return result;
}

//PUBLIC methods--------------------------------------------------------------------------------------------
bool TypeImageFile::LoadPNGimage (const string& ThisFile)
{   //Note: PNG is in Big-endian form

    if (LoadDataFrom(ThisFile)!=true) return false;
    unsigned RawDataLen=RawImageBytes.size();

    vector<unsigned char> MagicNum{{ 137, 80, 78, 71, 13, 10, 26, 10 }}; //PNG magic number (first 8 bytes)
    if (isMatch(MagicNum,0)==false) {cout<<"No PNG signature in file"<<endl; return false;}
    Properties.FileName=ThisFile;
    Properties.ImageFormat="PNG";

    //The IHDR chunk contains
    //Width 	         4 bytes
    //Height 	         4 bytes
    //Bit depth 	     1 byte
    //Color type 	     1 byte
    //Compression method 1 byte
    //Filter method 	 1 byte
    //Interlace method 	 1 byte
    vector<unsigned char> IHDR{{73,72,68,82}}; //IHDR header
    unsigned ihdrIdx=11;
    while (!isMatch(IHDR,++ihdrIdx)) if (ihdrIdx>=RawDataLen-4) {cout<<"No IHDR header in PNG"<<endl; return false;}
    Properties.width =BigEndian(ihdrIdx+4);
    Properties.height=BigEndian(ihdrIdx+8);

    Properties.BitDepth         =RawImageBytes[ihdrIdx+12];
    Properties.ColorType        =RawImageBytes[ihdrIdx+13];
    Properties.CompressionMethod=RawImageBytes[ihdrIdx+14];
    Properties.FilterMethod     =RawImageBytes[ihdrIdx+15];
    Properties.InterlaceMethod  =RawImageBytes[ihdrIdx+16];

    vector<unsigned char> IDAT{{73,68,65,84}};
    unsigned idatIdx=ihdrIdx+16;
    while (!isMatch(IDAT,++idatIdx)) if (idatIdx>=RawDataLen-4) {cout<<"No data section in the PNG"<<endl; return false;}

    RawImageBytes.clear();
    return true;
}

bool TypeImageFile::LoadBMPimage (const string& ThisFile)
{
    if (LoadDataFrom(ThisFile)!=true) return false;
    unsigned RawDataLen=RawImageBytes.size();

    //BMP header
    vector<unsigned char>  BM{{66,77}};
    if (!isMatch(BM,0)) {cout<<"Not valid BMP image"<<endl; return false;}
    Properties.FileName=ThisFile;
    Properties.ImageFormat="BMP";
    Properties.ByteCount=LittleEndian(2);
    if (Properties.ByteCount!=RawDataLen) {cout<<"Reported file size ["<<Properties.ByteCount<<"]. Actual bytes ["<<RawDataLen<<"]"<<endl;}
    unsigned PixArrOffset=LittleEndian(10);

    //DIB Header. (Byte offsets are hard coded)
    Properties.width= LittleEndian(18);
    Properties.height=LittleEndian(22);
    Properties.BitDepth=LittleEndian(28,2);
    Properties.BytesPerPixel=Properties.BitDepth/CHAR_BIT;
    if (Properties.BytesPerPixel==3) Properties.ColorType=2;
    if (Properties.BytesPerPixel==4) Properties.ColorType=6;
    if (Properties.BytesPerPixel==0) {cout<<"Error reading bit-depth"<<endl; return false;}
    Properties.CompressionMethod=LittleEndian(30);
    if (Properties.CompressionMethod!=0) {cout<<"Currently can only handle BI_RGB"<<endl; return false;}
    //unsigned pixelArrayReportedSize=LittleEndian(34);
    Properties.PrintResW=LittleEndian(38);
    Properties.PrintResH=LittleEndian(42);
    unsigned BytesPerRow=int((Properties.BitDepth*Properties.width+31)/32)*4; //includes padding

    //Pixel array section.
    //Note: BMP stores color in BGR, so it has to be rearranged.
    PixelArray.reserve(Properties.height*Properties.width*Properties.BytesPerPixel);
    unsigned ArrIdx=0;
    for (unsigned j=0;j<Properties.height;j++)
    for (unsigned i=0;i<BytesPerRow;i+=Properties.BytesPerPixel)
    {
        if (i>=Properties.width*Properties.BytesPerPixel) {continue;} //skip the padding
        //cout<<"i "<<i<<" total arr size"<<pixelArrayReportedSize<<endl;
        ArrIdx=j*BytesPerRow+i;
        //Get the pixel colors
        unsigned char RGB[4];
        if (Properties.BytesPerPixel>0) RGB[2] = RawImageBytes[PixArrOffset+ArrIdx+0];
        if (Properties.BytesPerPixel>1) RGB[1] = RawImageBytes[PixArrOffset+ArrIdx+1]; else RGB[1]=RGB[2];
        if (Properties.BytesPerPixel>2) RGB[0] = RawImageBytes[PixArrOffset+ArrIdx+2]; else RGB[0]=RGB[1];
        if (Properties.BytesPerPixel>3) RGB[3] = RawImageBytes[PixArrOffset+ArrIdx+3]; else RGB[3]=255;
        PixelArray.push_back(RGB[0]); //R
        PixelArray.push_back(RGB[1]); //G
        PixelArray.push_back(RGB[2]); //B
        PixelArray.push_back(RGB[3]); //A
    }
    cout<<"Final pixArrSize "<<PixelArray.size()<<endl;
    RawImageBytes.clear();
    return true;
}

void TypeImageFile::PrintBytes(const vector<unsigned char>& theseBytes, const string& comment, bool inHex, unsigned SoMany)
{
    unsigned Len=theseBytes.size(); if(Len==0) return;
    if (SoMany>Len || SoMany==0) SoMany=Len;

    //Save original settings
    int old_precision = cout.precision(); ios::fmtflags old_settings  = cout.flags();
    cout<<comment; if (SoMany<Len) cout<<" (Showing only "<<SoMany<<")"<<endl; else cout<<endl;
    for (unsigned i=0;i<SoMany;i++)
    {
        //bitset<8> x=OneByte;
        if (inHex==true) cout<<setfill('0')<<hex<<setw(2); else cout<<setw(3);
        cout<<(int)theseBytes[i];
        if (i>0 && (i+1)%20 == 0) cout<<endl; else cout<<" ";
    }
    cout<<endl;
    //restore original settings
    cout.flags(old_settings); cout.precision(old_precision);
}
