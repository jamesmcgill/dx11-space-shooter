@echo off
:: Source files are located in the same directory as this batch file
set SRC_DIR=%~dp0

:: Output files will be created in the parent directory of this batch file
cd ..
%SRC_DIR%\texconv.exe %SRC_DIR%\star.png -pmalpha -m 1 -f BC3_UNORM -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\player.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\shot.obj -y