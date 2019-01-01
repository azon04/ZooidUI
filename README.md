# Zooid UI
version: 0.1.1

Zooid UI is a C++ Immediate Mode Graphical User Interface (IMGUI for short) and developed with separate rendering in mind. It means the project need to be integrated with current/host application rendering pipeline.
This project mainly is used and tested in Zooid Engine (https://github.com/azon04/ZooidEngine). But I create some examples project that use OpenGL, DirectX (coming soon), and some other rendering APIs (maybe Vulkan and Metal).

# Integration
The current way to integrate the codes into your project is little bit works. Still figuring out what is the best and easiest way to integrate the code into other project.

# Things Done
There are some features already implemented and tested in OpenGL.

# Library and External Codes
All these libraries and external codes are mainly used in examples
* **GLEW 2.00**
* **GLFW**
* **stb_image** - https://github.com/nothings/stb to load image png, jpg, etc for texture.
* **FreeType** - this one is needed for the project to load font. But I will make this optional in the future.

# References
* Immediate-Mode Graphical User Interfaces (2005), Casey Muratori, https://caseymuratori.com/blog_0001
