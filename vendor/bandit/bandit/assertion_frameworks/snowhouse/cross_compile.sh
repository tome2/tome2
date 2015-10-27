#!/bin/bash

STATUS=""

function build_for {
  local CC=$1
  local CXX=$2
  local CXX_VERSION=$3

  echo "Compiling for $CC, $CXX, $CXX_VERSION..."

  if [[ "$CXX_VERSION" == "CXX" ]]; then
      local SNOWHOUSE_IS_CPP11=OFF
  else
      local SNOWHOUSE_IS_CPP11=ON
  fi

  echo "SNOWHOUSE_IS_CPP11=$SNOWHOUSE_IS_CPP11"

  BUILD_DIR=build-$CC-$CXX_VERSION
  mkdir $BUILD_DIR
  pushd $BUILD_DIR
  CC=$CC CXX=$CXX cmake -DSNOWHOUSE_IS_CPP11=$SNOWHOUSE_IS_CPP11 ../..
  make
  STATUS="$STATUS\n$BUILD_DIR - Status: $?"
  popd
}

if [[ -d builds ]]; then
  rm -rf builds
fi

mkdir builds
pushd builds

build_for gcc-4.5 g++-4.5 CXX
build_for gcc-4.6 g++-4.6 CXX
build_for gcc-4.6 g++-4.6 CXX11
build_for gcc-4.7 g++-4.7 CXX
build_for gcc-4.7 g++-4.7 CXX11
build_for gcc-4.8 g++-4.8 CXX
build_for gcc-4.8 g++-4.8 CXX11
build_for gcc-4.9 g++-4.9 CXX
build_for gcc-4.9 g++-4.9 CXX11
build_for clang clang++ CXX
build_for clang clang++ CXX11
popd

echo "============================================"
echo -e $STATUS
