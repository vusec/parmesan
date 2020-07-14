; RUN: %opt_idassign -analyze -idassign %s > %t
; RUN: cat %t | FileCheck %s
; ModuleID = 'hello.c'
source_filename = "hello.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [25 x i8] c"usage: %s print MESSAGE\0A\00", align 1
@.str.1 = private unnamed_addr constant [6 x i8] c"print\00", align 1

; CHECK-DAG: 0x{{[0-9a-f]+}}: Function: (print_message) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:6:3
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument:	(print_message) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:6:3
; Function Attrs: nounwind uwtable
define dso_local void @print_message(i8* nocapture readonly %message) local_unnamed_addr #0 !dbg !7 {
entry:
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock:	(print_message) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:6:3
  call void @llvm.dbg.value(metadata i8* %message, metadata !14, metadata !DIExpression()), !dbg !15
; CHECK-DAG: 0x{{[0-9a-f]+}}: Instruction:  (print_message) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:7:1
  %call = tail call i32 @puts(i8* %message), !dbg !16
  ret void, !dbg !17
}

; CHECK-DAG: 0x{{[0-9a-f]+}}: Function: (puts)
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument: (puts)
; Function Attrs: nounwind
declare dso_local i32 @puts(i8* nocapture readonly) local_unnamed_addr #1

; CHECK-DAG: 0x{{[0-9a-f]+}}: Function:  (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:10:12
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument:  (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:10:12
; CHECK-DAG: 0x{{[0-9a-f]+}}: Argument:  (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:10:12
; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) local_unnamed_addr #0 !dbg !18 {
entry:
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:10:12
  call void @llvm.dbg.value(metadata i32 %argc, metadata !25, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i8** %argv, metadata !26, metadata !DIExpression()), !dbg !28
; CHECK-DAG: 0x{{[0-9a-f]+}}: Instruction:  (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:10:12
  %cmp = icmp eq i32 %argc, 3, !dbg !29
  br i1 %cmp, label %if.end, label %if.then, !dbg !31

if.then:                                          ; preds = %entry
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:11:41
  %0 = load i8*, i8** %argv, align 8, !dbg !32, !tbaa !34
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i64 0, i64 0), i8* %0), !dbg !38
  tail call void @exit(i32 1) #5, !dbg !39
  unreachable, !dbg !39

if.end:                                           ; preds = %entry
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:15:15
  %arrayidx1 = getelementptr inbounds i8*, i8** %argv, i64 1, !dbg !40
  %1 = load i8*, i8** %arrayidx1, align 8, !dbg !40, !tbaa !34
  %call2 = tail call i32 @strcmp(i8* %1, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i64 0, i64 0)) #6, !dbg !42
  %tobool = icmp eq i32 %call2, 0, !dbg !42
  br i1 %tobool, label %if.then3, label %if.end5, !dbg !43

if.then3:                                         ; preds = %if.end
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:16:19
  %arrayidx4 = getelementptr inbounds i8*, i8** %argv, i64 2, !dbg !44
  %2 = load i8*, i8** %arrayidx4, align 8, !dbg !44, !tbaa !34
  call void @llvm.dbg.value(metadata i8* %2, metadata !14, metadata !DIExpression()) #7, !dbg !46
; CHECK-DAG: 0x{{[0-9a-f]+}}: Instruction: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:6:3 @[ hello.c:16:5 ]
  %call.i = tail call i32 @puts(i8* %2) #7, !dbg !48
  br label %if.end5, !dbg !49

if.end5:                                          ; preds = %if.end, %if.then3
; CHECK-DAG: 0x{{[0-9a-f]+}}: BasicBlock: (main) /home/egeretto/Documents/instructions_count/IDAssigner/source/test/src/hello.c:19:3
  ret i32 0, !dbg !50
}

; Function Attrs: nounwind
declare dso_local i32 @printf(i8* nocapture readonly, ...) local_unnamed_addr #1

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) local_unnamed_addr #2

; Function Attrs: nounwind readonly
declare dso_local i32 @strcmp(i8* nocapture, i8* nocapture) local_unnamed_addr #3

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #4

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone speculatable }
attributes #5 = { noreturn nounwind }
attributes #6 = { nounwind readonly }
attributes #7 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "hello.c", directory: "/home/egeretto/Documents/instructions_count/IDAssigner/source/test/src")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 8.0.0 "}
!7 = distinct !DISubprogram(name: "print_message", scope: !1, file: !1, line: 5, type: !8, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !12)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!13 = !{!14}
!14 = !DILocalVariable(name: "message", arg: 1, scope: !7, file: !1, line: 5, type: !10)
!15 = !DILocation(line: 5, column: 32, scope: !7)
!16 = !DILocation(line: 6, column: 3, scope: !7)
!17 = !DILocation(line: 7, column: 1, scope: !7)
!18 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 9, type: !19, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !24)
!19 = !DISubroutineType(types: !20)
!20 = !{!21, !21, !22}
!21 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!24 = !{!25, !26}
!25 = !DILocalVariable(name: "argc", arg: 1, scope: !18, file: !1, line: 9, type: !21)
!26 = !DILocalVariable(name: "argv", arg: 2, scope: !18, file: !1, line: 9, type: !22)
!27 = !DILocation(line: 9, column: 14, scope: !18)
!28 = !DILocation(line: 9, column: 26, scope: !18)
!29 = !DILocation(line: 10, column: 12, scope: !30)
!30 = distinct !DILexicalBlock(scope: !18, file: !1, line: 10, column: 7)
!31 = !DILocation(line: 10, column: 7, scope: !18)
!32 = !DILocation(line: 11, column: 41, scope: !33)
!33 = distinct !DILexicalBlock(scope: !30, file: !1, line: 10, column: 18)
!34 = !{!35, !35, i64 0}
!35 = !{!"any pointer", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C/C++ TBAA"}
!38 = !DILocation(line: 11, column: 5, scope: !33)
!39 = !DILocation(line: 12, column: 5, scope: !33)
!40 = !DILocation(line: 15, column: 15, scope: !41)
!41 = distinct !DILexicalBlock(scope: !18, file: !1, line: 15, column: 7)
!42 = !DILocation(line: 15, column: 8, scope: !41)
!43 = !DILocation(line: 15, column: 7, scope: !18)
!44 = !DILocation(line: 16, column: 19, scope: !45)
!45 = distinct !DILexicalBlock(scope: !41, file: !1, line: 15, column: 34)
!46 = !DILocation(line: 5, column: 32, scope: !7, inlinedAt: !47)
!47 = distinct !DILocation(line: 16, column: 5, scope: !45)
!48 = !DILocation(line: 6, column: 3, scope: !7, inlinedAt: !47)
!49 = !DILocation(line: 17, column: 3, scope: !45)
!50 = !DILocation(line: 19, column: 3, scope: !18)
