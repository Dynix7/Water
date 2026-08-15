# Sum of Sines Water
This is also my first project in raylib and graphics programming. It was heavily inspired by the goat Youtuber Acerola and specifically the video:
[How Games Fake Water - Acerola](https://www.youtube.com/watch?v=PH9q0HNBjT4) <br>
This is a water simulation through the use of the sum of sines method where you add a lot of oscillating waves. The vertices of the plane are then displaced by the sum. <br>
The normal vectors for the vertices and pixel are calculating using the partial derivatives to get the tangents along the plane and then the cross product to get the normal vector <br>
The lighting is a basic blinn-phong model with simulated scattering and some fake foam on the tips. <br>

# Running The Program
Use WASD to move around and press M for the menu to adjust wave settings and press ESC to exit. Performance should be good on all modern computers but varies on the wave parameters set.
## Linux
I've statically linked raylib within the binary so you should be able to run it without installing any dependencies (that you probably don't already have) Just make sure to run it in the folder since it needs to read the shader and assets. It was tested working CachyOS<br>

## Windows
Simply unzip the file provided for windows and run the .exe file within the the folder. Windows Smartscreen may pop up but just click continue to run the program. It was tested working on Windows 11.
