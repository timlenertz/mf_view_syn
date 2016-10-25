setlocal
pushd ..\mf
call build.bat %1
popd
mkdir build
pushd build
cmake ^
	-G "Visual Studio 14 2015 Win64" ^
    -DOpenCV_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}/../../opencv/build ^
    -DOpenCV_FOUND:BOOL=TRUE ^
    -DOPENCV_FOUND:BOOL=TRUE ^
    -DCMAKE_BUILD_TYPE=%1 ^
    -DCMAKE_INSTALL_PREFIX=../dist ..
msbuild /nologo /verbosity:minimal /clp:NoSummary view_syn.vcxproj
msbuild /nologo INSTALL.vcxproj
popd

