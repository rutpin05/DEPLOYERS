THE COMMUNITY EDITION OF THE GLG TOOLKIT README FILE

The Community Edition of the GLG Toolkit contains the Community Edition 
of the following components:

 - GLG Graphics Builder
 - GLG HMI Configurator
 - GLG Widgets (Dials, Meters, Charts, etc.)
 - GLG C/C++ Libraries
 - GLG ActiveX Control
 - GLG Java Class Library
 - GLG Graphics Server for JSP / Mobile
 - GLG Map Server

GLG GRAPHICS BUILDER

The Community Edition of the GLG Graphics Builder is installed in the 
following location:

    <glg_dir>\DEMOS\main_demo\GlgDemo.exe

The Builder may be started from the Start Menu. 
It may also be started from the command line using the -editor option, 
which should be used as the first option:

    <glg_dir>\DEMOS\main_demo\GlgDemo.exe -editor [other_options]

Shortcuts for starting the Graphics Builder with the GDI or OpenGL driver 
are also created in the <glg_dir>\bin directory.

GLG HMI CONFIGURATOR

The Community Edition of the GLG HMI Configurator uses the same executable
as the Graphics Builder:

    <glg_dir>\DEMOS\main_demo\GlgDemo.exe

The HMI Configurator may be started from the Start Menu. 
It may also be started from the command line using the -hmi-editor option, 
which should be used as the first option:

    <glg_dir>\DEMOS\main_demo\GlgDemo.exe -hmi-editor [other_options]

Shortcuts for starting the HMI Configurator with the GDI or OpenGL driver 
are also created in the <glg_dir>\bin directory.

GLG WIDGETS

The Community Edition of all GLG widget sets is included:

    Controls (Dials, Meters, Sliders, Buttons and Toggles)
    Avionics Dials
    Real Time Charts
    2D Graphs
    3D Graphs
    Process Control Symbols
    Special Widgets (menus, spinners, misc. input widgets)

The widgets are available in the Graphics Builder's and HMI Configurator's
widget palettes. The widgets' drawings are located in the 
<glg_dir>\DEMOS\widgets directory.

GLG C/C++ LIBRARIES

The Community Edition of the GLG C/C++ Libraries is installed in the 
following location:

 Standard API, includes the map server librrary:
   <glg_dir>\lib\Glg.dll             - Dynamic DLL
   <glg_dir>\lib\GlgLib.dll          - Static library (compiled with /MD)
   <glg_dir>\lib\GlgLibMT.dll        - Static library (compiled with /MT)

 Intermediate API
   <glg_dir>\lib\GlgIn.dll           - Dynamic DLL
   <glg_dir>\lib\GlgInLib.dll        - Static library (compiled with /MD)
   <glg_dir>\lib\GlgInLibMT.dll      - Static library (compiled with /MT)

 Extended API
   <glg_dir>\lib\GlgEx.dll           - Dynamic DLL
   <glg_dir>\lib\GlgExLib.dll        - Static library (compiled with /MD)
   <glg_dir>\lib\GlgExLibMT.dll      - Static library (compiled with /MT)

 Map Server Libraries:
   <glg_dir>\lib\libglg_map.a
   <glg_dir>\lib\libglg_map_stub.a   - stubs for linking withou the map server

 Include files directory:
   <glg_dir>\include

 C/C++ bindings:
   <glg_dir>\src\GlgClass.cpp

The following directories contain C/C++ demos and examples with the source 
code:

   <glg_dir>\DEMOS                  - C/C++ demos
   <glg_dir>\examples_c_cpp         - C/C++ examples
   <glg_dir>\examples_mfc           - MFC C++ examples

GLG ACTIVEX CONTROL

The Community Edition of the GLG ActiveX Control is installed in the 
following location:

   <glg_dir>\bin\glg.ocx

The following directories contain .NET examples with the source code:

   <glg_dir>\examples_csharp      - C# examples 
   <glg_dir>\examples_vbnet       - VB.net examples

GLG JAVA CLASS LIBRARY

The Community Edition of the GLG Java Class Library is installed in the 
following location:

   <glg_dir>\lib\GlgCE.jar

The following directories contain Java demos and examples with the source code:

   <glg_dir>\DEMOS\java_demos     - Java demos
   <glg_dir>\examples_java        - examples of various programming tasks
   <glg_dir>\src\Player.java      - source code of a viewer for GLG drawings

GLG GRAPHICS SERVER

The Community Edition of the GLG Graphics Server is installed in the following 
location:

   <glg_dir>\lib\GlgServerCE.jar

The Graphics Server demos and examples with the source code are located in 
the following directory:

   <glg_dir>\examples_jsp

GLG MAP SERVER

The Map Server executable for the Community Edition is installed in the 
following location:

    <glg_dir>\map_server\bin\GlmMap.exe

The map server sample dataset is installed in the following directory:

    <glg_dir>\map_data


DOCUMENTATION

Online Documentation for the GLG Toolkit is located at:

   http://www.genlogic.com/doc_html/start.html

NON-COMMERCIAL LICENSE

The Community Edition Non-Commercial License is located at:

   <glg_dir>\LICENSE_CE.TXT

It is also available online:

   http://www.genlogic.com/LICENSE_CE.TXT