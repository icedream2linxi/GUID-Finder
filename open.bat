set CURRENT_DIR=%~dp0
set QTDIR=%CURRENT_DIR%..\QTDIR
set IDADIR=%CURRENT_DIR%..\
set IDASDK=%CURRENT_DIR%..\idasdk70\
set BOOSTDIR=f:\vcpkg\installed\x64-windows\
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv "%CURRENT_DIR%Source\IDA_GUID-Finder_PlugIn.sln"