##Useful commands

###To build
`$ # clone llvm-project`
`$ cd llvm-project`
`$ mkdir build`
`$ cd build`
`$ cmake -DLLVM_ENABLE_PROJECTS=clang -G "Unix Makefiles" ../llvm`
`$ make -j4`

###To test
`$ ~/llvm-project/build/bin/clang                                        # clang command`
`$ clang -cc1 -analyzer-checker-help                                     # see list of the available checkers`
`$ clang -cc1 -analyze -analyzer-checker=<checker> <test>                # analyze <test> with <checker>`
`$ ~/llvm-project/clang/lib/StaticAnalyzer/Checkers                      # checkers directory`

###To add new checker
`$ # add checker itself`
`$ ~/llvm-project/llvm/utils/gn/secondary/clang/lib/StaticAnalyzer/Checkers/BUILD.gn`
`$ ~/llvm-project/clang/lib/StaticAnalyzer/Checkers/CMakeLists.txt`
`$ ~/llvm-project/clang/include/clang/StaticAnalyzer/Checkers/Checkers.td`
`$ make -j4`

###To find smth in code
`$ grep -r PATTERN --exclude-dir=build ~/llvm-project/`
