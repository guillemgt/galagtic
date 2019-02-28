@echo off

if not defined DevEnvDir (
	"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

set CommonCompilerFlags=-WL -O2 -nologo -fp:fast -fp:except- -Gm- -GR- -EHsc -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 -GS- -Gs9999999 -DDEBUG_BUILD
set CommonCompilerFlags=-D_CRT_SECURE_NO_WARNINGS=1 %CommonCompilerFlags%

set ReleaseCompilerFlags=-WL -O2 -nologo -fp:fast -fp:except- -Gm- -GR- -EHsc -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -GS- -Gs9999999 -O2
set ReleaseCompilerFlags=-D_CRT_SECURE_NO_WARNINGS=1 %ReleaseCompilerFlags%

set CommonLinkerFlags= -STACK:0x100000,0x100000 -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib kernel32.lib

mkdir lag-windows\build
pushd lag-windows\build
cl ..\..\code\main.cpp -I"../../../Common Things/SDL/include" -I"../../../Common Things/glew/include" /Felag.exe %CommonCompilerFlags% /link /LIBPATH:"../../../Common Things/SDL/lib/x64" /LIBPATH:"../../../Common Things/glew/lib/x64" /SUBSYSTEM:CONSOLE %CommonLinkerFlags%

mt.exe -manifest ../lag.exe.manifest -outputresource:lag.exe
popd