# ThomasRubikCube
Description:

    Using OpenGL 3.3 and FreeGlut
    Using Fragment and vertex shaders (no more immediate mode)
    Using self made matrix and vector math objects from scratch
    Using self made obj loader
    Self made image loader (supports 24bit and 32bit BMP)
    (I started a PNG loader, but need to implement Huffman encoding for compress/decompress)
    Support up to six texture units per draw call (per object)
    My GLobject class supports true instancing with an intricate pointer structure that allows each clone object to link to vertex data (on the same memory) from the master object (no deep copies) on multiple aspects; instanced objects can borrow vertex coordinates, or vertex colors, or texture UVs, or vertex normals individually.
    Geometry class object can generate primitive objects (similar to what GLUT was offering)
    Each object can have thousands of surfaces drawn on a SINGLE draw call.
    Has infrastructure for grouping of objects and group nesting.
    Support ambient illumination and a point light, fixed relative to camera, illuminating different aspects of an object as the camera moves around. Can support unlimited amount of lights if needed (no more GLUT limitation of 10)
    The above code was used to create a Rubik's cube.
    The cube is reverse engineered from an actual physical cube measured within 1/16th of an inch and modeled in Rhino3D with extensive detail replicating all internal components including rounded filletted edges for all its parts
    The cube features six textures (one per side). The textures are used on multiple parts of the cube faces, the corner pieces featuring three textures simultaneously.
    The cube only has 5 modeled pieces which were then instanced and textured (each piece having its own UVs
    Support smooth mouse scene rotation and zoom
    The cube can expand to show its internals (and then contract)
    The cube can have some of its parts selectively hidden
    Can switch to partial wireframe mode
    Of course all movements are supported (9 planes of motion)

Screenshots:

![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-1.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-2.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-3.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-4.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-5.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-6.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-7.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/Clipboard-8.jpg)
![alt tag](https://github.com/ThomasAn73/ThomasRubikCube/blob/master/ScreenShots/CubeMechanism.jpg)
