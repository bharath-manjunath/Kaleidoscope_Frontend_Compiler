; ModuleID = 'my cool jit'
source_filename = "my cool jit"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-darwin22.5.0"

declare double @putchard(double)

define double @printstar(double %n) {
entry:
  %calltmp = call double @putchard(double 1.000000e+01)
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi double [ 0.000000e+00, %entry ], [ %nextvar, %loop ]
  %calltmp1 = call double @putchard(double 4.200000e+01)
  %nextvar = fadd double %i, 1.000000e+00
  %cmptmp = fcmp ult double %i, %n
  %booltmp = uitofp i1 %cmptmp to double
  %loopcond = fcmp one double %booltmp, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  %calltmp2 = call double @putchard(double 1.000000e+01)
  ret double %calltmp2
}

define double @main() {
entry:
  %calltmp = call double @printstar(double 3.800000e+01)
  %calltmp1 = call double @putchard(double 7.200000e+01)
  %calltmp2 = call double @putchard(double 1.000000e+01)
  %calltmp3 = call double @putchard(double 6.400000e+01)
  %calltmp4 = call double @printstar(double 3.600000e+01)
  %calltmp5 = call double @putchard(double 5.500000e+01)
  %calltmp6 = call double @putchard(double 6.000000e+00)
  %addtmp = fadd double %calltmp5, %calltmp6
  ret double %addtmp
}

define double @__anon_expr() {
entry:
  %calltmp = call double @putchard(double 3.200000e+01)
  ret double %calltmp
}
