cd ./Pass/build && cmake -DCMAKE_BUILD_TYPE=Release ../Transforms/Liveness && make -j4 && cd -

fileName=1
cd test 
opt -load ../Pass/build/libLLVMLivenessPass.so  -ValueNumbering < $fileName.ll > /dev/null 2> $fileName-result.out
