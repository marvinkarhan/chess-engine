
cd ./test
mkdir -p engine
cd ./engine
rm -rf lib
mkdir -p lib/catch 
cd lib/catch
wget https://raw.githubusercontent.com/catchorg/Catch2/devel/extras/catch_amalgamated.hpp
wget https://raw.githubusercontent.com/catchorg/Catch2/devel/extras/catch_amalgamated.cpp