rmdir /S /Q build32
mkdir build32

cd build32
cmake -G "Visual Studio 14 2015" -DCMAKE_SYSTEM_VERSION=8.1 -DCMAKE_INSTALL_PREFIX=..\\installed32 ..
cmake --build . --target install --config Release
cd ..