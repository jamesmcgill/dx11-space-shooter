@echo off
:: Source files are located in the same directory as this batch file
set SRC_DIR=%~dp0

:: Output files will be created in the parent directory of this batch file
cd ..
%SRC_DIR%\texconv.exe %SRC_DIR%\star.png -pmalpha -m 1 -f BC3_UNORM -y
%SRC_DIR%\texconv.exe %SRC_DIR%\explosion.png -pmalpha -m 1 -f BC3_UNORM -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\player.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship1.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship2.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship3.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship4.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship5.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship6.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship7.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship8.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\ship9.obj -y
%SRC_DIR%\meshconvert.exe -n -c -sdkmesh %SRC_DIR%\shot.obj -y
%SRC_DIR%\MakeSpriteFont.exe "Verdana" verdana32.spritefont /FontSize:32
%SRC_DIR%\MakeSpriteFont.exe "Verdana" verdana8.spritefont /FontSize:8
%SRC_DIR%\MakeSpriteFont.exe "Courier New" mono32.spritefont /FontSize:32
%SRC_DIR%\MakeSpriteFont.exe "Courier New" mono8.spritefont /FontSize:8