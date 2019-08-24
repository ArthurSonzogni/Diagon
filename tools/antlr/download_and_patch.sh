cd "$(dirname "$0")"
top=$(pwd)

if [ -d cpp_runtime ]
then
  exit
fi

mkdir cpp_runtime

## Download ANTLR and the CPP runtime
curl http://www.antlr.org/download/antlr-4.7.1-complete.jar -Lo antlr.jar
curl http://www.antlr.org/download/antlr4-cpp-runtime-4.7.1-source.zip -Lo cpp_runtime/archive.zip

## Unzip cpp runtime
cd cpp_runtime
unzip *.zip

## Patch the cpp runtime CMakeLists.txt.
patch CMakeLists.txt < ../CMakeLists.txt.patch
