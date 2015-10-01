#version 330 //for openGL 3.3

//---------------------------------------------------------------------------------
//uniform variables
uniform bool      useTextures;     //If no textures, don't bother reading the TextureUnit array
uniform vec4      AmbientColor;    //Background illumination
uniform sampler2D TextureUnit[6];  //Up to 6 texture units per draw call
//---------------------------------------------------------------------------------
//non-uniform variables
     in  vec2  vertexUVcoord;      //coming in from the vertex shader
     in  vec4  thisColor;          //coming in from the vertex shader
flat in  int   TexUnitIdx;         //active texture unit for the current vertex
     in  float VertLightIntensity;
//---------------------------------------------------------------------------------
//Output color to graphics card
out vec4 pixelColor;
//---------------------------------------------------------------------------------

void main ()
{
    vec4 tempColor;
    if (useTextures==false || TexUnitIdx<0 || TexUnitIdx>5) tempColor = thisColor;
    else
    {
        tempColor = texture2D(TextureUnit[TexUnitIdx], vertexUVcoord);
        tempColor = tempColor*tempColor.w + (1-tempColor.w)*thisColor; //Normal blending
    }

    if (VertLightIntensity<0) {pixelColor = tempColor;}
    else {pixelColor = tempColor * (AmbientColor + VertLightIntensity)/2.0;}
}
