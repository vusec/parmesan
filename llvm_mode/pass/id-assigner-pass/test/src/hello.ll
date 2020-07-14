; RUN: %opt_idassign -analyze -idassign %s > %t
; RUN: cat %t | FileCheck %s
; RUN: %opt_idassign -idassign -idassign-emit-info -idassign-info-file %t.csv %s > /dev/null
; RUN: cat %t.csv | FileCheck %s -check-prefix=CHECK-CSV

; CHECK-CSV: id,type,function,debug_info

; ModuleID = 'hello.c'
source_filename = "hello.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"usage: %s print MESSAGE\0A\00", align 1
@.str.1 = private unnamed_addr constant [6 x i8] c"print\00", align 1

; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},func,print_message,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Function: (print_message)
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},farg,print_message,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument: (print_message)
; Function Attrs: nounwind uwtable
define dso_local void @print_message(i8* nocapture readonly %message) local_unnamed_addr #0 {
entry:
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,print_message,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:	(print_message)
; CHECK-DAG: 0x{{[0-9a-f]+}}: Instruction:  (print_message)
  %call = tail call i32 @puts(i8* %message)
  ret void
}

; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},func,puts,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Function: (puts) 
; Function Attrs: nounwind
declare dso_local i32 @puts(i8* nocapture readonly) local_unnamed_addr #1

; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},func,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Function:  (main)
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},farg,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument:  (main)
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},farg,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument:  (main)
; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) local_unnamed_addr #0 {
entry:
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:  (main)
; CHECK-DAG: 0x{{[0-9a-f]+}}: Instruction:  (main)
  %cmp = icmp eq i32 %argc, 3
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:  (main)
  %0 = load i8*, i8** %argv, align 8, !tbaa !2
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i64 0, i64 0), i8* %0)
  tail call void @exit(i32 1) #4
  unreachable

if.end:                                           ; preds = %entry
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:  (main)
  %arrayidx1 = getelementptr inbounds i8*, i8** %argv, i64 1
  %1 = load i8*, i8** %arrayidx1, align 8, !tbaa !2
  %call2 = tail call i32 @strcmp(i8* %1, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i64 0, i64 0)) #5
  %tobool = icmp eq i32 %call2, 0
  br i1 %tobool, label %if.then3, label %if.end5

if.then3:                                         ; preds = %if.end
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:  (main)
  %arrayidx4 = getelementptr inbounds i8*, i8** %argv, i64 2
  %2 = load i8*, i8** %arrayidx4, align 8, !tbaa !2
  %call.i = tail call i32 @puts(i8* %2) #6
  br label %if.end5

if.end5:                                          ; preds = %if.end, %if.then3
; CHECK-CSV-DAG: 0x{{[0-9a-f]+}},babl,main,""
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:  (main)
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #2

; Function Attrs: nounwind readonly
declare dso_local i32 @strcmp(i8* nocapture, i8* nocapture) local_unnamed_addr #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn nounwind }
attributes #5 = { nounwind readonly }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 "}
!2 = !{!3, !3, i64 0}
!3 = !{!"any pointer", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
