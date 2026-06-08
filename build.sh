# remember to actually set these..
export SPLAT_DIR=/home/deck/splat
export TOOLCHAIN=/home/deck/toolchain/clang-cl-msvc.cmake
export HOST_ARCH=x86_64

# you can also use `-G Ninja` here
cmake \
  -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN \
  -DCMAKE_BUILD_TYPE=Release \
  -B build

cmake --build build --config Release
