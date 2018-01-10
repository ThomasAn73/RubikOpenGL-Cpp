//***CAMERA CLASS implementation******************************************************************************
//************************************************************************************************************

//---PRIVATE functions----------------------------------------------------------------------------------------
void TypeCamera::computeProjectionMatrix () //This is in ROW-major order [row][column] vectors are vertical
{
    ProjectionMatrix.SetZero();

    if (Ortho==false)
    {   //Perspective projection.
        ProjectionMatrix.data[0][0] = 2.0*nearPlane/(clipRight-clipLeft);
        ProjectionMatrix.data[0][2] = (clipRight+clipLeft)/(clipRight-clipLeft);
        ProjectionMatrix.data[1][1] = 2.0*nearPlane/(clipTop-clipBottom);
        ProjectionMatrix.data[1][2] = (clipTop+clipBottom)/(clipTop-clipBottom);
        ProjectionMatrix.data[2][2] = -(farPlane+nearPlane)/(farPlane-nearPlane);
        ProjectionMatrix.data[2][3] = -(2.0*farPlane*nearPlane)/(farPlane-nearPlane);
        ProjectionMatrix.data[3][2] = -1;
    }
    else
    {   //Parallel projection
        ProjectionMatrix.data[0][0] = 2.0/(clipRight-clipLeft);
        ProjectionMatrix.data[0][3] = -(clipRight+clipLeft)/(clipRight-clipLeft);
        ProjectionMatrix.data[1][1] = 2.0/(clipTop-clipBottom);
        ProjectionMatrix.data[1][3] = -(clipTop+clipBottom)/(clipTop-clipBottom);
        ProjectionMatrix.data[2][2] = -2.0/(farPlane-nearPlane);
        ProjectionMatrix.data[2][3] = -(farPlane+nearPlane)/(farPlane-nearPlane);
        ProjectionMatrix.data[3][3] = 1;
    }
}

void TypeCamera::computeViewMatrix () //This is in ROW-major order [row][column] vectors are vertical
{
    TypeXYZ camZ = Target-EyePos; camZ=camZ.ofLength(1);
    TypeXYZ camY = Up;
    TypeXYZ camX = camZ.MutualOrtho(camY);

    ViewMatrix.SetIdentity();
    ViewMatrix.SetRow(0,{camX     , -EyePos.DotProduct(camX)});
    ViewMatrix.SetRow(1,{camY     , -EyePos.DotProduct(camY)});
    ViewMatrix.SetRow(2,{camZ*(-1), +EyePos.DotProduct(camZ)});
}

//alternative computation
void TypeCamera::computeViewMatrix2 () //This is in ROW-major order [row][column] vectors are vertical
{
    TypeTmatrix temp;
    ViewMatrix.SetIdentity();
    //Apply transformations to the camera object until it is aligned to the world Z axis with UP in the +Y axis
    TypeXYZ CT = EyePos - Target;
    if (CT.x!=0 || CT.y!=0)
    temp.RotAboutZ(pi/2.0-CT.angleXY(),false);        //Rotate aboutZ using the shadow of CT on XY

    temp.RotAboutX(CT.angleTO({0,0,1}),false);        //Rotate aboutX using the angle of CT to the Z axis
    temp.RotAboutZ((temp*Up).angleXY()-pi/2.0,false); //Rotate aboutZ so that the UP vector aligns to the +Y axis
    //Now for real
    ViewMatrix.TranslateBy(Target*(-1));              //Walk from target to the origin (undoing camera pan)
    ViewMatrix.TranslateBy(Target-EyePos);            //Walk from camera to the target (undoing camera position)
    ViewMatrix=temp*ViewMatrix;                       //Apply rotations (undoing camera rotation)
}

void TypeCamera::computeFOV ()
{ //assume nearDistance is fixed. Computes all field of views X,Y, and diagonal.
    FOV.x = atan(abs(clipLeft)/nearPlane) + atan(abs(clipRight)/nearPlane);
    FOV.x = DegFromRad(FOV.x);

    FOV.y = atan(abs(clipTop)/nearPlane) + atan(abs(clipBottom)/nearPlane);
    FOV.y = DegFromRad(FOV.y);

    double diagA = sqrt(clipLeft*clipLeft + clipTop*clipTop);
    double diagB = sqrt(clipRight*clipRight + clipBottom*clipBottom);
    FOV.z = atan(diagA/nearPlane) + atan(diagB /nearPlane);
    FOV.z = DegFromRad(FOV.z);
}

void TypeCamera::computeAspect()
{aspect=(clipRight-clipLeft)/(clipTop-clipBottom);}

double TypeCamera::ComputeClipPlaneScale (double th1, double th2, double newFOV)
{
    //angles in rad
    //th1 is the angle from 0 to clipBottom
    //th2 is the angle from 0 to clipTop
    //th1+th2 = current FOV
    //newFOV is a different angle which will define a larger clip plane
    //This function returns the scalar needed to multiply the current clip plane in order for it to span the newFOV angle
    double a=tan(th1)*tan(th2)*tan(newFOV);
    double b=tan(th1)+tan(th2);
    double c=-tan(newFOV);

    //Solving a quadratic
    double L1=(-b+sqrt(b*b-4*a*c))/(2*a);
    double L2=(-b-sqrt(b*b-4*a*c))/(2*a);

    //Discard the negative value
    if (L1>0) return L1; else return L2;
}

// PUBLIC FUNCTIONS-------------------------------------------------------------------------------------------
//---CONSTRUCTORS---------------------------------------------------------------------------------------------
TypeCamera::TypeCamera()
{
    //These initial values have been manually calculated to be valid
    EyePos =TypeXYZ{0.0,0.0,12.5}; //Camera position (world coord)
    Target =TypeXYZ{0.0,0.0, 0.0}; //Camera target   (world coord)
    Up     =TypeXYZ{0.0,1.0, 0.0}; //Camera Up direction (vector on world 0,0,0)

    Ortho      =false;
    clipLeft   =-1.0;  //near plane left   (relative to camera)
    clipRight  = 1.0;  //near plane right  (relative to camera)
    clipTop    = 1.0;  //near plane top    (relative to camera)
    clipBottom =-1.0;  //near plane bottom (relative to camera)
    nearPlane  = 4.0;  //measuring from the eye (the eye considered 0)
    farPlane   =30.0;  //measuring from the eye

    computeFOV();
    computeAspect();

    computeViewMatrix();
    computeProjectionMatrix();
}

//---DESTRUCTOR-----------------------------------------------------------------------------------------------
TypeCamera::~TypeCamera()
{return;}

//---MISC methods---------------------------------------------------------------------------------------------
void TypeCamera::Move (TypeXYZ newTarget, bool isDelta /*=false*/)
{
    TypeXYZ delta=newTarget;
    if (isDelta==false) delta=newTarget-GetTarget();

    EyePos = EyePos + delta;
    Target = Target + delta;

    computeViewMatrix ();
}

//This zooms by changing the field of view
//An alternate method (and probably better) is to zoom the entire camera widget uniformly (using "target" as the pivot point)
void TypeCamera::Zoom (double deltaDegrees)
{
    double newFOVy = FOV.y-deltaDegrees;
    if (newFOVy>120 || newFOVy<5) return;

    SetFOVy(newFOVy); //This will also update the projection matrix
}

void TypeCamera::RotateZ (double angle)
{//Rotate about world Z-axis
    angle = RadFromDeg(angle);
    TypeXYZ newCam = EyePos-Target; //position as a vector from 0,0,0
    newCam = newCam.RotAboutZ(angle);
    EyePos=newCam+Target;
    Up = Up.RotAboutZ(angle);

    computeViewMatrix();
}

void TypeCamera::RotateX (double angle)
{//Rotate about X-axis relative to camera
    angle = RadFromDeg(angle);
    TypeXYZ newCam = EyePos-Target; //position as a vector from 0,0,0
    TypeXYZ CamX = newCam.MutualOrtho(Up); //orthogonal to position and the Up vector is the camera x-axis
    newCam = newCam.RotAboutAxis(CamX,angle);
    EyePos = newCam+Target;
    Up = Up.RotAboutAxis(CamX,angle);

    computeViewMatrix();
}

//---SET methods ---------------------------------------------------------------------------------------------
void TypeCamera::SetAspect(double newRatio) //assume ratio = width/height
{//will affect clipping values and FOV
    if (newRatio<=0) newRatio=1;
    aspect=newRatio;

    //Reasoning behind all this -->
    // cH/cW = wH/wW -> cH=cW * wH/wW (1)
    // cH=cT-cB, cW=cR-cL, then substitute into (1)
    // cT-cB=(cR-cL) * wH/wW -> cT - cB = cR*wH/wW - cL*wH/wW (2)
    // let cT = cR*wH/wW (3)
    // let cB = cL*wH/wW (4)
    // EQUATIONS (3)-(4) = (2)
    // We do it this way because the clipping plane might not have been centered to begin with
    if (newRatio<1) //Which means height>width
    {
        clipTop = clipRight * 1.0/newRatio;
        clipBottom = clipLeft * 1.0/newRatio;
    }
    else
    {
        clipRight = clipTop * newRatio;
        clipLeft = clipBottom * newRatio;
    }

    //Now adjust the FOV
    computeFOV(); computeProjectionMatrix();
}

void TypeCamera::SetFOVy (double degrees)
{//will affect clip plane values (size of near plane)
    double th1=atan(abs(clipBottom)/nearPlane); //first component angle of the FOVy angle in rads
    double th2=atan(abs(clipTop)/nearPlane); //second component angle of the FOVy angle in rads
    double scalar=ComputeClipPlaneScale(th1,th2,RadFromDeg(degrees));

    //Adjust the clip plane size
    clipBottom *= scalar;
    clipTop *= scalar;
    clipLeft *=scalar;
    clipRight *=scalar;

    //Given the new clip plane size we can compute the FOV
    computeFOV(); computeProjectionMatrix();
}

//Affects FOV, Aspect, and the projection matrix
void TypeCamera::SetClipPlane (double left, double right, double bottom, double top)
{
    clipLeft=left; clipRight=right; clipBottom=bottom; clipTop=top;
    computeFOV(); computeAspect(); computeProjectionMatrix();
}

//---GET methods ---------------------------------------------------------------------------------------------
