@echo off
echo ================================================================
echo    Pushing Emergency Evacuation Simulator to GitHub
echo    Target: https://github.com/Rakeshhyadav12/EmergencyEvacuationSimulator
echo ================================================================
echo.

cd /d "%~dp0"

echo [1/5] Initializing Git repository...
git init

echo [2/5] Staging all files...
git add .

echo [3/5] Creating commit...
git commit -m "Initial commit: 3D AI-Based Emergency Evacuation Simulator"

echo [4/5] Setting main branch...
git branch -M main

echo [5/5] Configuring remote origin...
git remote remove origin 2>nul
git remote add origin https://github.com/Rakeshhyadav12/EmergencyEvacuationSimulator.git

echo.
echo Attempting to push to GitHub...
echo (If prompted, please authorize via the browser pop-up)
echo.
git push -u origin main

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================================================
    echo [SUCCESS] Project successfully pushed to:
    echo https://github.com/Rakeshhyadav12/EmergencyEvacuationSimulator
    echo ================================================================
) else (
    echo.
    echo ================================================================
    echo [IMPORTANT] If the repository does not exist on GitHub yet:
    echo 1. Open https://github.com/new in your browser
    echo 2. Set Repository name to: EmergencyEvacuationSimulator
    echo 3. Keep all checkboxes unchecked and click "Create repository"
    echo 4. Run this push_to_github.bat again!
    echo ================================================================
)

pause
