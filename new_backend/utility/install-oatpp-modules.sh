#!/bin/sh

rm -rf tmp

mkdir tmp
cd tmp

##########################################################
## install oatpp

MODULE_NAME="oatpp"

git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

cp -R ./$MODULE_NAME/src/$MODULE_NAME ./../src/

cd $MODULE_NAME
mkdir build
cd build

cmake ..
make install

cd ../../

##########################################################
## install oatpp-websocket

MODULE_NAME="oatpp-websocket"

git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

cp -R ./$MODULE_NAME/src/$MODULE_NAME ./../src/


cd $MODULE_NAME
mkdir build
cd build

cmake ..
make install

cd ../../

##########################################################

cd ../

sudo rm -rf tmp
