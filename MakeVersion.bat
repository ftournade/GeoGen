@echo off

REM ########### Settings ############

set AppName=GeoGen
set backupPath=..\..\..\Packaging

set ftpHost=ftp.felkowski.fr
set ftpPath=floflo
set ftpUser=felkowsk 
set ftpPass=K5uUDu27 

set /p buildVersion=Build version(e.g. 0.12.456):
REM set /p zipPass=Archive password: 

REM #################################

mkdir %backupPath%
mkdir %backupPath%\%AppName%
mkdir %backupPath%\%AppName%\Build_%buildVersion%
mkdir %backupPath%\%AppName%\Build_%buildVersion%\Shaders

copy /Y ..\..\..\Build\GeoGen-Release-x64.exe %backupPath%\%AppName%\Build_%buildVersion%\GeoGen64.exe
copy /Y .\Shaders\*.* %backupPath%\%AppName%\Build_%buildVersion%\Shaders

set zipName="%backupPath%\%AppName%\Build_%buildVersion%.7z"

"%ProgramFiles%\7-Zip\7z.exe" a %zipName% -r -y  %backupPath%\%AppName%\Build_%buildVersion%
REM -p%zipPass%


echo Backup done !

Pause