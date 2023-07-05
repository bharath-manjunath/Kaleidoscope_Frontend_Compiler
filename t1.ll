; ModuleID = 'my cool jit'
source_filename = "my cool jit"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-darwin22.5.0"

define double @sq(double %b, double %c, double %v) {
entry:
  %multmp = fmul double %b, 2.000000e+00
  %addtmp = fadd double %b, %multmp
  ret double %addtmp
}

define double @fac(double %a) {
entry:
  %subtmp = fsub double %a, 1.000000e+00
  %multmp = fmul double %a, %subtmp
  ret double %multmp
}
