@REM msbuild build/INSTALL.vcxproj /p:Configuration=Debug /m /p:CL_MPCount=5 
msbuild build/INSTALL.vcxproj /p:Configuration=Debug /m /p:CL_MPCount=5 /clp:ErrorsOnly;PerformanceSummary;Summary 