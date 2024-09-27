set CURRENT_DIR=%~dp0
set QTDIR=%CURRENT_DIR%QTDIR
set IDADIR=%CURRENT_DIR%..\
set IDASDK=%CURRENT_DIR%idasdk83\
set BOOSTDIR=f:\vcpkg\installed\x64-windows\
set VS143COMNTOOLS=c:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\
call "%VS143COMNTOOLS%..\..\VC\Auxiliary\Build\vcvars32.bat"
devenv "%CURRENT_DIR%Source\IDA_GUID-Finder_PlugIn.sln"