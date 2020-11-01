for %%X in (msbuild.exe) do (set FOUND=%%~$PATH:X)
echo test0
if not defined FOUND (
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
) 

echo test

call msbuild.exe  model2dll/model2dll.sln /p:Configuration=Debug /l:FileLogger,Microsoft.Build.Engine;logfile=Manual_MSBuild_ReleaseVersion_LOG.log

XCOPY .\model2dll\x64\Debug\*.lib .\TFInUnreal\Source\ThirdParty\ /Y
XCOPY .\model2dll\x64\Debug\*.dll .\TFInUnreal\Source\ThirdParty\ /Y
XCOPY .\model2dll\x64\Debug\*.pdb .\TFInUnreal\Source\ThirdParty\ /Y
XCOPY .\model2dll\ThirdParty\libtensorflow\lib\*.lib .\TFInUnreal\Source\ThirdParty\ /Y
XCOPY .\model2dll\ThirdParty\libtensorflow\lib\*.dll .\TFInUnreal\Source\ThirdParty\ /Y
XCOPY .\model2dll\model2dll\*.h .\TFInUnreal\Source\ThirdParty\include\ /Y
XCOPY .\model\model.pb .\TFInUnreal\Source\ThirdParty\ /Y
