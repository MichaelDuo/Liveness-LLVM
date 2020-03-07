cd ./Pass/build && cmake -DCMAKE_BUILD_TYPE=Release ../Transforms/Liveness && make -j4 && cd -

fileName=1
cd test 
clang -Xclang -disable-O0-optnone $fileName.c -O0 -S -emit-llvm -o $fileName.ll 
opt $fileName.ll -mem2reg -S -o $fileName.bc
opt -load ../Pass/build/libLLVMLivenessPass.so  -ValueNumbering < $fileName.bc > /dev/null 
# opt -load ../Pass/build/libLLVMLivenessPass.so  -ValueNumbering < $fileName.bc > /dev/null 2> $fileName.out
