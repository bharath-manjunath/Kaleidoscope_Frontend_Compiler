
if [ $# -lt 1 ]; then
echo "Fatal Error: Missing file name"
exit 1
fi 
if [ $# -lt 2 ]; then
/opt/homebrew/opt/llvm/bin/clang++ -rdynamic $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3
else
/opt/homebrew/opt/llvm/bin/clang++ -rdynamic $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core orcjit native` -O3 -o $2
fi