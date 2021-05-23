Remove-Item -Recurse -Force tmp -ErrorAction SilentlyContinue

Remove-Item -Recurse -Force src/vendor -ErrorAction SilentlyContinue
mkdir tmp
cd tmp

$VENDOR = '../../../src/vendor/'
## install oatpp

$MODULE_NAME="oatpp"

git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

cd $MODULE_NAME

mkdir build
cd build

cmake -DCMAKE_INSTALL_PREFIX="$VENDOR"  -G "Unix Makefiles" ..
make install

cd ../../

##########################################################
## install oatpp-websocket

$MODULE_NAME="oatpp-websocket"

git clone --depth=1 https://github.com/oatpp/$MODULE_NAME

cd $MODULE_NAME
mkdir build
cd build

cmake -DCMAKE_INSTALL_PREFIX="$VENDOR" -G "Unix Makefiles"  ..
make install

cd ../../

#End
cd ../

Remove-Item -Recurse -Force tmp -ErrorAction SilentlyContinue
