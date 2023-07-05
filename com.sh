
if [ $# -lt 1 ]; then
echo "Fatal Error: Missing file name"
exit 1
fi 
if [ $# -lt 2 ]; then
/opt/homebrew/opt/llvm/bin/clang++ -g -O3 $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags`
else
/opt/homebrew/opt/llvm/bin/clang++ -g -O3 $1 `/opt/homebrew/opt/llvm/bin/llvm-config --cxxflags` -o $2
fi
