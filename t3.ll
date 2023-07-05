; ModuleID = 'my cool jit'
source_filename = "my cool jit"
target datalayout = "e-m:o-i64:64-i128:128-n32:64-S128"
target triple = "arm64-apple-darwin22.5.0"

declare double @printd(double)

define double @fib(double %x) {
entry:
  %cmpltmp = fcmp ult double %x, 2.000000e+00
  %booltmp = uitofp i1 %cmpltmp to double
  %ifcond = fcmp one double %booltmp, 0.000000e+00
  br i1 %ifcond, label %then, label %else

then:                                             ; preds = %entry
  br label %ifcont

else:                                             ; preds = %entry
  %subtmp = fsub double %x, 1.000000e+00
  %calltmp = call double @fib(double %subtmp)
  %subtmp1 = fsub double %x, 2.000000e+00
  %calltmp2 = call double @fib(double %subtmp1)
  %addtmp = fadd double %calltmp, %calltmp2
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %iftmp = phi double [ 1.000000e+00, %then ], [ %addtmp, %else ]
  ret double %iftmp
}

define double @main() {
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %i = phi double [ 0.000000e+00, %entry ], [ %nextvar, %loop ]
  %calltmp = call double @fib(double %i)
  %calltmp1 = call double @printd(double %calltmp)
  %nextvar = fadd double %i, 1.000000e+00
  %cmpltmp = fcmp ult double %i, 2.000000e+01
  %booltmp = uitofp i1 %cmpltmp to double
  %loopcond = fcmp one double %booltmp, 0.000000e+00
  br i1 %loopcond, label %loop, label %afterloop

afterloop:                                        ; preds = %loop
  ret double 0.000000e+00
}
