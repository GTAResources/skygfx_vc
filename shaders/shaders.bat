for /R "./ps/" %%f in (*.hlsl) do "%DXSDK_DIR%\Utilities\bin\x86\fxc.exe" /T ps_2_0 /nologo /E main /Fo ../resources/cso/%%~nf.cso %%f
for /R "./vs/" %%f in (*.hlsl) do "%DXSDK_DIR%\Utilities\bin\x86\fxc.exe" /T vs_2_0 /nologo /E main /Fo ../resources/cso/%%~nf.cso %%f

for /R "./ps/" %%f in (*.hlsl) do "%DXSDK_DIR%\Utilities\bin\x86\fxc.exe" /T ps_2_0 /nologo /E main /Fh %%~nf.h %%f
for /R "./vs/" %%f in (*.hlsl) do "%DXSDK_DIR%\Utilities\bin\x86\fxc.exe" /T vs_2_0 /nologo /E main /Fh %%~nf.h %%f

