# Sum of Sines Water
This is also my first project in raylib and graphics programming. It was heavily inspired by the goat Youtuber Acerola and specifically the video:
[How Games Fake Water - Acerola](https://www.youtube.com/watch?v=PH9q0HNBjT4) <br>
This is a water simulation through the use of the sum of sines method where you add a lot of oscillating waves. The vertices of the plane are then displaced by the sum. <br>
The normal vectors for the vertices and pixel are calculating using the partial derivatives to get the tangents along the plane and then the cross product to get the normal vector <br>
The lighting is a basic blinn-phong model with simulated scattering and some fake foam on the tips. <br>

# Running The Program
## Linux
I've statically linked raylib within the binary so you should be able to run it without installing any dependencies (other than stuff like x11, opengl, libc, etc.) Just make sure to run it in the folder since it needs to read the shader and assets <br>
