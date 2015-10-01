#version 330 //for openGL 3.3

//---------------------------------------------------------------------------------
//uniform variables stay constant for the whole glDraw call
uniform mat4   ProjViewModelMatrix; //Already transposed (column major order)
uniform mat4   NormalsMatrix;       //the model-matrix inversed and transposed
uniform vec4   DefaultColor;        //default object color (x=-1 is the signal to use per-vertex color instead of this)
uniform vec4   LightColor;          //single light source (could be an array in the future)
uniform vec3   LightPosition;       //single light position
uniform float  LightIntensity;      //controls how far the light reaches (throw)
uniform bool   ExcludeFromLight;    //Yes-No to compute light equation for this object
//---------------------------------------------------------------------------------
//non-uniform variables get fed per vertex from the buffers
layout (location=0) in vec3 VertexCoord;  //feeding from attribute=0 of the main code
layout (location=1) in vec4 VertexColor;  //per vertex color, feeding from attribute=1 of the main code
layout (location=2) in vec3 VertexNormal; //per vertex normals
layout (location=3) in vec2 VertexUVcoord;//texture coordinates
layout (location=4) in int  vertexTexUnit;//per vertex texture unit index
//---------------------------------------------------------------------------------
//Output variables to fragment shader
     out vec4  thisColor;          //Vertex color (will be ignored if there is an opaque texture)
     out vec2  vertexUVcoord;
flat out int   TexUnitIdx;         //This is 'flat' means: it will not get interpolated in the fragment shader
     out float VertLightIntensity; //Light intensity at the particular vertex
//---------------------------------------------------------------------------------

void main ()
{
    vertexUVcoord = VertexUVcoord; //Output to fragment shader
    TexUnitIdx    = vertexTexUnit; //Output to fragment shader

    //Output vertex color to fragment shader
    if (DefaultColor.x==-1) {thisColor = VertexColor;} //If no default color is set, use per vertex colors
    else {thisColor = DefaultColor;}

    //Output vertex light intensity to fragment shader
    if (LightIntensity>0)
    {
        vec3  adjustedVertNormal = mat3(NormalsMatrix) * VertexNormal;
        vec3  adjustedVertCoord  = mat3(NormalsMatrix) * VertexCoord;
        vec3  VertToLight        = LightPosition - adjustedVertCoord;

        float cosTheta           = dot ( normalize(VertToLight), normalize(adjustedVertNormal));
        float LightDistance      = length(VertToLight);

        VertLightIntensity = LightIntensity * (1.0/pow(LightDistance,2)) * (1+cosTheta);
    }
    else
    {
        VertLightIntensity = -1.0;
    }

    //Output vertex position to the graphics card
    gl_Position = ProjViewModelMatrix * vec4(VertexCoord,1.0);
}
