cd "$(dirname "$0")"
top=$(pwd)

mkdir -p cpp_runtime

# Download ANTLR and the CPP runtime
curl http://www.antlr.org/download/antlr-4.7.1-complete.jar -o antlr.jar
curl http://www.antlr.org/download/antlr4-cpp-runtime-4.7.1-source.zip -o cpp_runtime/archive.zip

# Build cpp runtime
cd cpp_runtime
unzip archive.zip
mkdir build
cd build
cmake ..\
  -DCMAKE_BUILD_TYPE=Release \
  -DANTLR_JAR_LOCATION=$top/antlr.jar \
  -DWITH_DEMO=True
make
DESTDIR=$top/runtime/Cpp/run make install
cd $top
