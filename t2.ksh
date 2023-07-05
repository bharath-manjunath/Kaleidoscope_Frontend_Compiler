#putchard print this ascii char to standard output.
extern putchard(char);
def printstar(n){
    putchard(10); #\n
  for i = 0, i < n, 1.0 in
    putchard(42);  # ascii 42 = '*'
    putchard(10); #\n
    }
def main(){
printstar(38);
putchard(72);
putchard(10);
putchard(64);
printstar(36);
putchard(55) + putchard(6);
}

putchard(32);