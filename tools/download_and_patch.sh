cd "$(dirname "$0")"
top=$(pwd)

if [[ -f "antlr.jar" ]]
then
  exit
else
  curl http://www.antlr.org/download/antlr-4.8-complete.jar -L -o antlr.jar
fi
