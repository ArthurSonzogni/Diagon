cd "$(dirname "$0")"
top=$(pwd)

if [ -d cpp_runtime ]
then
  exit
fi

mkdir -p cpp_runtime

## Download the ANTLR executable.
curl http://www.antlr.org/download/antlr-4.8-complete.jar -L -o antlr.jar

## Download the C++ runtime.
git clone --depth 1 https://github.com/antlr/antlr4 -b 4.8
