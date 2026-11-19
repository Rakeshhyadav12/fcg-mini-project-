@echo off
echo ================================================================
echo    Building 3D Emergency Evacuation Simulator (MinGW g++)
echo ================================================================
echo.

REM Try compilation with freeglut first
echo Compiling with -lfreeglut...
g++ -O2 -Wall -std=c++11 main.cpp Building.cpp Person.cpp Fire.cpp Pathfinding.cpp Camera.cpp Simulation.cpp -o EmergencyEvacuationSimulator.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32 2> build_error.log

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build succeeded! Created EmergencyEvacuationSimulator.exe
    echo Running EmergencyEvacuationSimulator.exe...
    EmergencyEvacuationSimulator.exe
    goto end
)

REM If freeglut failed, try legacy glut32
echo.
echo Trying fallback with -lglut32...
g++ -O2 -Wall -std=c++11 main.cpp Building.cpp Person.cpp Fire.cpp Pathfinding.cpp Camera.cpp Simulation.cpp -o EmergencyEvacuationSimulator.exe -lglut32 -lopengl32 -lglu32 -lwinmm -lgdi32 2>> build_error.log

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] Build succeeded with glut32! Created EmergencyEvacuationSimulator.exe
    echo Running EmergencyEvacuationSimulator.exe...
    EmergencyEvacuationSimulator.exe
    goto end
)

echo.
echo [ERROR] Compilation failed. Please check build_error.log
type build_error.log
echo.
echo If GLUT/FreeGLUT is not installed in your MinGW include/lib path,
echo please refer to the README.md for FreeGLUT setup in Code::Blocks/MinGW.

:end
pause
