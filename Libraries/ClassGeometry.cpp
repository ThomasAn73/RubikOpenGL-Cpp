//---TYPE TMATRIX------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------

//Helper functions
TypeTmatrix::TypeTmatrix ()
{SetIdentity();}

TypeTmatrix::TypeTmatrix (float (&A)[4][4])
{for (int i=0; i<4;++i) for (int j=0; j<4; ++j) data[i][j]=A[i][j];}

TypeTmatrix::TypeTmatrix (float (&A)[16])
{for (int i=0;i<16;i++) {data[i/4][i%4]=A[i];}}

void TypeTmatrix::SetIdentity ()
{for (int i=0; i<4;++i) for (int j=0; j<4; ++j) if (i==j) data[i][j]=1.0; else data[i][j]=0.0;}

void TypeTmatrix::SetZero ()
{for (int i=0;i<4;i++) for (int j=0;j<4;j++) data[i][j]=0.0;}

void TypeTmatrix::SetRow (unsigned short Row, TypeXYZw a)
{if (Row>3) return; data[Row][0]=a[0]; data[Row][1]=a[1]; data[Row][2]=a[2]; data[Row][3]=a[3];}

void TypeTmatrix::SetColumn (unsigned short Col, TypeXYZw a)
{if (Col>3) return; data[0][Col]=a[0]; data[1][Col]=a[1]; data[2][Col]=a[2]; data[3][Col]=a[3];}

void TypeTmatrix::OrthogonalizeCols (unsigned short thisMany, bool normalize) //assumes vectors are stored vertically as in math texts
{//Using Gram-Schmidt
    if (thisMany<2) return;
    if (thisMany>4) thisMany=3;

    for (unsigned short i=0;i<thisMany;i++) //'thisMany' is a count, 'i' is an array index
    {
        if (i==0 && normalize==true) {SetColumn(i,GetColumn(i).Normalized()); continue;}
        TypeXYZw v = GetColumn(i);
        TypeXYZw u = v;
        for (unsigned short j=0;j<i;j++) {u = u - v.ProjectONTO(GetColumn(j));}
        if (normalize==true) SetColumn(i,u.Normalized()); else SetColumn(i,u);
    }
}

void TypeTmatrix::ScaleBy (double scaleX,double scaleY,double scaleZ)
{
    TypeTmatrix ScaleMatrix;

    ScaleMatrix.data[0][0]=scaleX;
    ScaleMatrix.data[1][1]=scaleY;
    ScaleMatrix.data[2][2]=scaleZ;

    (*this) = ScaleMatrix * (*this);
}

void TypeTmatrix::ScaleBy (double scale)
{ScaleBy(scale,scale,scale);}

void TypeTmatrix::TranslateBy (double x, double y, double z)
{
    TypeTmatrix TranslationMatrix;

    TranslationMatrix.data[0][3]=x;
    TranslationMatrix.data[1][3]=y;
    TranslationMatrix.data[2][3]=z;

    (*this) = TranslationMatrix * (*this);
}

void TypeTmatrix::TranslateBy (TypeXYZ delta)
{TranslateBy(delta.x,delta.y,delta.z);}

void TypeTmatrix::ScaleByPointDrag (TypeXYZ pivot, TypeXYZ fromA, TypeXYZ toB, bool isUniform)
{
    TypeXYZ A=fromA-pivot; if (A.x<epsilon || A.y<epsilon || A.z<epsilon) return;
    TypeXYZ B=toB-pivot;
    double BAratio=B.length()/A.length();

    if (isUniform==false) ScaleBy(B.x/A.x, B.y/A.y, B.z/A.z);
    else ScaleBy(BAratio,BAratio,BAratio);
}

void TypeTmatrix::ScaleBox (TypeXYZ boxCornerA, TypeXYZ boxCornerB, TypeXYZ cornerA, TypeXYZ cornerB)
{
    TypeXYZ oldDelta = boxCornerB-boxCornerA;
    TypeXYZ newDelta = cornerB-cornerA;
    TypeXYZ scale = {newDelta.x/oldDelta.x, newDelta.y/oldDelta.y, newDelta.z/oldDelta.z};
    if (oldDelta.x==0) scale.x=0; if (oldDelta.y==0) scale.y=0; if (oldDelta.z==0) scale.z=0; //make sure no component is NaN

    SetIdentity();
    ScaleBy(scale.x, scale.y, scale.z);

    TypeXYZ Translate = cornerA-boxCornerA.scale(scale.x, scale.y, scale.z);
    TranslateBy(Translate.x, Translate.y, Translate.z);
}

//left handed coordinate system -- from +X looking to the origin
void TypeTmatrix::RotAboutX (double angle, bool inDegrees /*=true*/)
{
    angle = inDegrees ? RadFromDeg(angle):angle;
    TypeTmatrix rotationMatrix;

    rotationMatrix.data[1][1]=cos(angle);
    rotationMatrix.data[1][2]=-sin(angle);
    rotationMatrix.data[2][1]=sin(angle);
    rotationMatrix.data[2][2]=cos(angle);

    (*this) = rotationMatrix * (*this);
}
//left handed coordinate system -- from +Y looking to the origin
void TypeTmatrix::RotAboutY (double angle, bool inDegrees /*=true*/)
{
    angle = inDegrees ? RadFromDeg(angle):angle;
    TypeTmatrix rotationMatrix;

    rotationMatrix.data[0][0]=cos(angle);
    rotationMatrix.data[0][2]=sin(angle);
    rotationMatrix.data[2][0]=-sin(angle);
    rotationMatrix.data[2][2]=cos(angle);

    (*this) = rotationMatrix * (*this);
}
//left handed coordinate system-- from +Z looking to the origin
void TypeTmatrix::RotAboutZ (double angle, bool inDegrees /*=true*/)
{
    angle = inDegrees ? RadFromDeg(angle):angle;
    TypeTmatrix rotationMatrix;

    rotationMatrix.data[0][0]=cos(angle);
    rotationMatrix.data[0][1]=-sin(angle);
    rotationMatrix.data[1][0]=sin(angle);
    rotationMatrix.data[1][1]=cos(angle);

    (*this) = rotationMatrix * (*this);
}

void TypeTmatrix::Print(string Comment) const
{
    using namespace std;

    //Save original settings
    ios::fmtflags old_settings  = cout.flags();      //save previous format flags
    int           old_precision = cout.precision();  // save previous precision setting

    cout<<Comment<<endl;
    cout.setf(ios::fixed, ios::floatfield); cout.precision(5);
    for (int i=0;i<4;i++) cout<<"["<<setw(9)<<data[i][0]<<","<<setw(9)<<data[i][1]<<","<<setw(9)<<data[i][2]<<","<<setw(9)<<data[i][3]<<"]"<<endl;
    cout<<endl;

    //restore original settings
    cout.flags(old_settings); cout.precision(old_precision);
}

TypeTmatrix TypeTmatrix::Transpose () const
{
    TypeTmatrix result;
    for (int i=0;i<4;i++)
        for(int j=i;j<4;j++)
    {
        result.data[j][i]=data[i][j];
        result.data[i][j]=data[j][i];
    }
    return result;
}

//Inverse matrix using row reduction of augmented matrix method
TypeTmatrix TypeTmatrix::Inverse () const
{
    //Will be working on both simultaneously
    TypeTmatrix InverseMatrix;
    TypeTmatrix SourceMatrix = *this;

    //Row reduction section
    for (int k=0;k<4;k+=3)  //two passes: one for low echelon form, and one for upper echelon form.
    {
        for (int l=0;l<4;l++)
        {
            int i=abs(k-l);
            TypeXYZw pivotRow=SourceMatrix.GetRow(i);
            TypeXYZw pivotRowInv=InverseMatrix.GetRow(i);
            if (pivotRow[i]!=0 && pivotRow[i]!=1)
            {
                pivotRowInv=pivotRowInv/pivotRow[i]; InverseMatrix.SetRow(i,pivotRowInv);
                pivotRow=pivotRow/pivotRow[i]; SourceMatrix.SetRow(i,pivotRow);
            }
            else if (pivotRow[i]==0) //Look for a non zero pivot
            {
                bool success=false;
                for (int p=3;p>i;p--)
                {
                    if (SourceMatrix.data[p][i]==0) continue;

                    //Pivot found. Do a swap.
                    pivotRow=SourceMatrix.GetRow(p);
                    pivotRowInv=InverseMatrix.GetRow(p);
                    SourceMatrix.SetRow(p,SourceMatrix.GetRow(i));
                    SourceMatrix.SetRow(i,pivotRow);
                    InverseMatrix.SetRow(p,InverseMatrix.GetRow(i));
                    InverseMatrix.SetRow(i,pivotRowInv);
                    success=true; break;
                }
                if (success==false) {InverseMatrix.SetInvalid(); return InverseMatrix;}
            }
            for (int m=l+1;m<4;m++)
            {
                int j=abs(k-m);
                TypeXYZw rowSourceNow=SourceMatrix.GetRow(j);
                TypeXYZw rowInverseNow=InverseMatrix.GetRow(j);
                if (rowSourceNow[i]==0) continue;
                double c=pivotRow[i]/rowSourceNow[i];
                rowSourceNow=rowSourceNow*c; rowInverseNow=rowInverseNow*c;
                rowSourceNow=pivotRow-rowSourceNow; rowInverseNow=pivotRowInv-rowInverseNow;
                SourceMatrix.SetRow(j,rowSourceNow);
                InverseMatrix.SetRow(j,rowInverseNow);
            }
        }
    }
    return InverseMatrix;
}

TypeTmatrix TypeTmatrix::operator*   (const TypeTmatrix& B) const
{
    TypeTmatrix C; C.SetZero();
    double r;

    //Matrix multiplication according to 'Systems book', probably unnecessary for such a small matrix
    for (int i=0;i<4;i++)
        for (int k=0;k<4;k++)
        {r=data[i][k]; for (int j=0;j<4;j++) C.data[i][j] += r * B.data[k][j];}

    return C;
}

TypeXYZ TypeTmatrix::operator*   (const TypeXYZ& v) const
{
    double temp[4]={0.0,0.0,0.0,1.0};
    double columnMatrix[4]={v.x,v.y,v.z,1.0};

    for (int i=0;i<4;i++) for (int j=0;j<4;j++) temp[i] += data[i][j] * columnMatrix[j];

    return TypeXYZ {temp[0],temp[1],temp[2]};
}

//***TypeGeometry CLASS implementation***************************************************************************
//***************************************************************************************************************

//---PRIVATE functions-------------------------------------------------------------------------------------------
void TypeGeometry::UpdateCentroid()
{
    //This is only a simple average. Not very accurate for complex shapes.
    //Multiply existing centroid by current number of points-1
    //add the current point and divide by number of points
    unsigned int NumberOfPoints = GetNumberOfCoords();
    ContentTable.Coordinates[0] = ContentTable.Coordinates[0] * (NumberOfPoints-1) + GetPointByIndex(NumberOfPoints);
    ContentTable.Coordinates[0] = ContentTable.Coordinates[0] /NumberOfPoints;
}

void TypeGeometry::InitializeBoundingBox ()
{
    //set the initial bounding box to be invertly infinite (the top left corner being bottom right infinite, etc)
    //this way it will snap to the very first point immediately
    BoundingBox[0]={1.0/0.0, -1.0/0.0, -1.0/0.0};
    BoundingBox[1]={-1.0/0.0, 1.0/0.0, 1.0/0.0};
}

void TypeGeometry::UpdateBoundingBox (TypeXYZ NewPoint)
{
    if (NewPoint.x<BoundingBox[0].x) BoundingBox[0].x=NewPoint.x;
    if (NewPoint.x>BoundingBox[1].x) BoundingBox[1].x=NewPoint.x;

    if (NewPoint.y>BoundingBox[0].y) BoundingBox[0].y=NewPoint.y;
    if (NewPoint.y<BoundingBox[1].y) BoundingBox[1].y=NewPoint.y;

    //In openGL, +z is coming at you. In Rhino3D, +z is away from you
    if (NewPoint.z>BoundingBox[0].z) BoundingBox[0].z=NewPoint.z; //Using OpenGL convention for now
    if (NewPoint.z<BoundingBox[1].z) BoundingBox[1].z=NewPoint.z;
}

void TypeGeometry::ComputeBoundingBox ()
{
    //Doing it from scratch
    InitializeBoundingBox();
    unsigned len = GetNumberOfCoords(); if (len==0) return;

    for (unsigned i=1;i<=len;i++)
    {
        TypeXYZ point=ContentTable.Coordinates[i];
        if (point.isValid()==true) UpdateBoundingBox(ContentTable.Coordinates[i]);
    }
}

void TypeGeometry::ComputeCentroid ()
{
    //Doing it from scratch
    unsigned len = GetNumberOfCoords(); if (len==0) return;

    TypeXYZ result;
    TypeXYZ onePoint;
    for (unsigned i=1;i<=len;i++)
    {
        onePoint=GetPointByIndex(i);
        if (onePoint.isValid()==true) {result = result + onePoint;}
    }
    result=result/len;
    ContentTable.Coordinates[0]=result;
}

// PUBLIC FUNCTIONS-------------------------------------------------------------------------------------------
//---CONSTRUCTORS---------------------------------------------------------------------------------------------
TypeGeometry::TypeGeometry()
{
        ContentTable.Coordinates.push_back(TypeXYZ{0,0,0});    //centroid (do not use the add point method)
        InitializeBoundingBox();
}

//---DESTRUCTOR-----------------------------------------------------------------------------------------------
TypeGeometry::~TypeGeometry () {}

//---MISC functions-------------------------------------------------------------------------------------------

void TypeGeometry::BakeTmatrixIntoGeometry (const TypeTmatrix& Tmatrix)
{
    unsigned len = GetNumberOfCoords();
    for (unsigned i=0;i<=len;i++)
        ContentTable.Coordinates[i] = Tmatrix*ContentTable.Coordinates[i];

    ComputeBoundingBox();
}

void TypeGeometry::GenerateGrid (double UnitLength, unsigned UnitCountX, unsigned subdivions)
{
    unsigned density = UnitCountX*subdivions*2.0;
    double LineLength=UnitLength*UnitCountX*2.0;
    double step = UnitLength/(double) subdivions;
    TypeXYZ Start = {-LineLength/2.0,-LineLength/2.0,0.0};
    cout<<"density"<<density<<" Line length"<<LineLength<<" StartX"<<Start.x<<" step"<<step<<endl;
    for (unsigned i=0;i<=density;i++)
    {
        //cout<<i<<endl;
        //Create a Y line
        AddPoint(Start.x+(i*step),Start.y,0);
        AddPoint(Start.x+(i*step),Start.y+LineLength,0);

        //Create an X line
        AddPoint(Start.x,Start.y+(i*step),0);
        AddPoint(Start.x+LineLength,Start.y+(i*step),0);
    }
}

//---SET methods----------------------------------------------------------------------------------------------
void TypeGeometry::SetCentroid (TypeXYZ a)
{ContentTable.Coordinates[0]=a;}

void TypeGeometry::SetCoordByIndex (unsigned int thisPoint, TypeXYZ newXYZ,bool updateCentroid, bool updateBoundingBox)
{
    if (thisPoint > GetNumberOfCoords()) return;
    ContentTable.Coordinates[thisPoint] = newXYZ;

    //The bounding box may have changed too
    //It is computationally expensive to keep recomputing the bounding box
    if (updateBoundingBox==true) ComputeBoundingBox();
    //The centroid has also changed
}

void TypeGeometry::SetCoordByIndex (unsigned int thisPoint, double x, double y, double z,bool updateCentroid, bool updateBoundingBox)
{SetCoordByIndex(thisPoint,TypeXYZ {x,y,z},updateCentroid,updateBoundingBox);}

void TypeGeometry::SetGeometryData (const TypeGeometryRegistry& source)
{
    ContentTable=source;
    ComputeBoundingBox();
    ComputeCentroid();
}

void TypeGeometry::AddPoint (TypeXYZ thisPoint)
{
    AddCoord(thisPoint);
    ContentTable.Points.push_back(ContentTable.Coordinates.size()-1);
}

void TypeGeometry::AddLine (TypeXYZ a, TypeXYZ b)
{
    //add two points
    AddCoord(a); AddCoord(b);
    //Register the line
    unsigned idx=ContentTable.Coordinates.size()-1;
    vector<unsigned> oneLine;
    oneLine.push_back(idx-1); oneLine.push_back(idx);
    ContentTable.Lines.push_back(oneLine);
}

void TypeGeometry::AddPolyline (short LastStrayPoints)
{
    //take so many "LastStrayPoints" from the single-points registry and move them to the polyline registry
    unsigned totalStrayPoints=GetNumberOfPoints();
    unsigned startIdx=totalStrayPoints-LastStrayPoints;
    if ((unsigned) LastStrayPoints<2 || (unsigned)LastStrayPoints>totalStrayPoints) return;
    vector<unsigned> onePolyline;
    //read indices from the single points array and create a new vector with them
    for (unsigned i=startIdx;i<totalStrayPoints;i++) onePolyline.push_back(ContentTable.Points[i]);

    ContentTable.Polylines.push_back(onePolyline); //add the new vector into the polylines registry
    ContentTable.Points.resize(startIdx);          //delete single point registrations from the single points array
}

void TypeGeometry::AddNgon (unsigned short sidesCount, TypeXYZ center, TypeXYZ RadiusPoint, TypeXYZ normal, bool inscribed)
{
    if (sidesCount<3 || normal.IsCollinear(center,RadiusPoint)==true ) return;

    double unitAngle = 2*pi/sidesCount;
    TypeXYZ radiusVec=RadiusPoint-center;
    if (normal.isOrtho(radiusVec)==false) normal = normal.AsOrthoTo(radiusVec); //orthogonalize the normal vector
    if (inscribed==false)
    {   //Circumscribed means the radiusPoint is the center of a cord. Need to find an edge.
        radiusVec=radiusVec.ofLength(radiusVec.length()/cos(unitAngle/2)); //This is now the true radius vector
        radiusVec=radiusVec.RotAboutAxis(normal,unitAngle/2);              //Rotate the vec to the edge
        RadiusPoint=radiusVec+center;                                      //Update the RadiusPoint itself
    }

    //Prepare to register the polygon
    unsigned CoordIdx=ContentTable.Coordinates.size();
    vector<unsigned> onePolyLine;

    //Generate coords and register the polygon
    for (unsigned short i=0;i<sidesCount;i++)
    {
        TypeXYZ Point=radiusVec.RotAboutAxis(normal,unitAngle*i) + center;
        AddCoord(Point);
        onePolyLine.push_back(CoordIdx++);

    }
    onePolyLine.push_back(CoordIdx-sidesCount); //To close the loop
    //AddCoord({NAN,NAN,NAN});
    //onePolyLine.push_back(CoordIdx);

    ContentTable.Polylines.push_back(onePolyLine);
}

void TypeGeometry::AddTriangleSrf (TypeXYZ a, TypeXYZ b, TypeXYZ c)
{
    //add three points directly
    AddCoord(a); AddCoord(b); AddCoord(c);
    unsigned idx=ContentTable.Coordinates.size()-1; //get the new size of the coords array

    //Register the triangle face
    TypeSurface OneSurface;
    OneSurface.Mesh.push_back(idx-2); OneSurface.Mesh.push_back(idx-1); OneSurface.Mesh.push_back(idx);

    //Add the surface edge polylines (in this case only one)
    vector<unsigned> OneEdge;
    OneEdge.push_back(idx-2);OneEdge.push_back(idx-1);OneEdge.push_back(idx);OneEdge.push_back(idx-2);
    OneSurface.Edges.push_back(OneEdge);

    ContentTable.Surfaces.push_back(OneSurface);
}

void TypeGeometry::AddRectangleSrf (TypeXYZ TopLeft, TypeXYZ BaseLeft, TypeXYZ BaseRight)
{
    //explained: (TopLeft-BaseLeft) + BaseRight --- is the same as (TopLeft-BaseLeft) + (BaseRight-BaseLeft) + BaseLeft
    TypeXYZ TopRight = (TopLeft-BaseLeft) + BaseRight;
    AddCoord(TopLeft); AddCoord(BaseLeft); AddCoord(BaseRight); AddCoord(TopRight);
    unsigned LastIdx=ContentTable.Coordinates.size()-1;

    TypeSurface OneSurface; //Create rectangle face to register
    OneSurface.Mesh.push_back(LastIdx-3); OneSurface.Mesh.push_back(LastIdx-2); OneSurface.Mesh.push_back(LastIdx-1); //first triangle
    OneSurface.Mesh.push_back(LastIdx-3); OneSurface.Mesh.push_back(LastIdx-1); OneSurface.Mesh.push_back(LastIdx);   //second triangle

    //Add the surface edge polylines (in this case only one)
    vector<unsigned> OneEdge;
    OneEdge.push_back(LastIdx-3); OneEdge.push_back(LastIdx-2); OneEdge.push_back(LastIdx-1); OneEdge.push_back(LastIdx); OneEdge.push_back(LastIdx-3);
    OneSurface.Edges.push_back(OneEdge);

    ContentTable.Surfaces.push_back(OneSurface);      //rectangle added to the surfaces registry
}

void TypeGeometry::RegisterSurface (const TypeSurface& ThisSurface)
{
    ContentTable.Surfaces.push_back(ThisSurface);
}

//void TypeGeometry::DeleteAllPoints ()
//{ContentTable.CoordinatesArray.clear(); ContentTable.CoordinatesArray.push_back(TypeXYZ{0,0,0});InitializeBoundingBox();}

//---GET methods----------------------------------------------------------------------------------------------
TypeXYZ TypeGeometry::GetPointByIndex (unsigned int i) const
{
    TypeXYZ result;
    if (i > GetNumberOfCoords() ) return result;

    result = ContentTable.Coordinates[i];

    return result;
}

long long  TypeGeometry::GetSurfaceIndex (const string& SurfaceName) const
{
    unsigned surfaceCount=ContentTable.Surfaces.size();
    for (unsigned i=0;i<surfaceCount;i++) {if (ContentTable.Surfaces[i].SurfaceName==SurfaceName) return i;}

    return -1;
}
