
if [ $# -lt 1 ]; then
echo "Fatal Error: Missing file name"
exit 1
fi 
if [ $# -lt 2 ]; then
/opt/homebrew/opt/llvm/bin/clang++ -O3 $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core`
else
/opt/homebrew/opt/llvm/bin/clang++ -O3 $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags --ldflags --system-libs --libs core` -o $2
fi