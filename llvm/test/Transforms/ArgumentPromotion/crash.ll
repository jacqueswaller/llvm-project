; NOTE: Assertions have been autogenerated by utils/update_test_checks.py UTC_ARGS: --function-signature --scrub-attributes
; RUN: opt -S < %s -inline -argpromotion | FileCheck %s --check-prefixes=ARGPROMOTION,ALL_OLDPM
; RUN: opt -S < %s -passes=inline,argpromotion | FileCheck %s --check-prefixes=ARGPROMOTION,ALL_NEWPM

%S = type { %S* }

; Inlining should nuke the invoke (and any inlined calls) here even with
; argument promotion running along with it.
define void @zot() personality i32 (...)* @wibble {
; ARGPROMOTION-LABEL: define {{[^@]+}}@zot() personality i32 (...)* @wibble
; ARGPROMOTION-NEXT:  bb:
; ARGPROMOTION-NEXT:    unreachable
; ARGPROMOTION:       hoge.exit:
; ARGPROMOTION-NEXT:    br label [[BB1:%.*]]
; ARGPROMOTION:       bb1:
; ARGPROMOTION-NEXT:    unreachable
; ARGPROMOTION:       bb2:
; ARGPROMOTION-NEXT:    [[TMP:%.*]] = landingpad { i8*, i32 }
; ARGPROMOTION-NEXT:    cleanup
; ARGPROMOTION-NEXT:    unreachable
;
bb:
  invoke void @hoge()
  to label %bb1 unwind label %bb2

bb1:
  unreachable

bb2:
  %tmp = landingpad { i8*, i32 }
  cleanup
  unreachable
}

define internal void @hoge() {
bb:
  %tmp = call fastcc i8* @spam(i1 (i8*)* @eggs)
  %tmp1 = call fastcc i8* @spam(i1 (i8*)* @barney)
  unreachable
}

define internal fastcc i8* @spam(i1 (i8*)* %arg) {
bb:
  unreachable
}

define internal i1 @eggs(i8* %arg) {
; ALL_NEWPM-LABEL: define {{[^@]+}}@eggs()
; ALL_NEWPM-NEXT:  bb:
; ALL_NEWPM-NEXT:    unreachable
;
bb:
  %tmp = call zeroext i1 @barney(i8* %arg)
  unreachable
}

define internal i1 @barney(i8* %arg) {
bb:
  ret i1 undef
}

define i32 @test_inf_promote_caller(i32 %arg) {
; ARGPROMOTION-LABEL: define {{[^@]+}}@test_inf_promote_caller
; ARGPROMOTION-SAME: (i32 [[ARG:%.*]])
; ARGPROMOTION-NEXT:  bb:
; ARGPROMOTION-NEXT:    [[TMP:%.*]] = alloca [[S:%.*]]
; ARGPROMOTION-NEXT:    [[TMP1:%.*]] = alloca [[S]]
; ARGPROMOTION-NEXT:    [[TMP2:%.*]] = call i32 @test_inf_promote_callee(%S* [[TMP]], %S* [[TMP1]])
; ARGPROMOTION-NEXT:    ret i32 0
;
bb:
  %tmp = alloca %S
  %tmp1 = alloca %S
  %tmp2 = call i32 @test_inf_promote_callee(%S* %tmp, %S* %tmp1)

  ret i32 0
}

define internal i32 @test_inf_promote_callee(%S* %arg, %S* %arg1) {
; ARGPROMOTION-LABEL: define {{[^@]+}}@test_inf_promote_callee
; ARGPROMOTION-SAME: (%S* [[ARG:%.*]], %S* [[ARG1:%.*]])
; ARGPROMOTION-NEXT:  bb:
; ARGPROMOTION-NEXT:    [[TMP:%.*]] = getelementptr [[S:%.*]], %S* [[ARG1]], i32 0, i32 0
; ARGPROMOTION-NEXT:    [[TMP2:%.*]] = load %S*, %S** [[TMP]]
; ARGPROMOTION-NEXT:    [[TMP3:%.*]] = getelementptr [[S]], %S* [[ARG]], i32 0, i32 0
; ARGPROMOTION-NEXT:    [[TMP4:%.*]] = load %S*, %S** [[TMP3]]
; ARGPROMOTION-NEXT:    [[TMP5:%.*]] = call i32 @test_inf_promote_callee(%S* [[TMP4]], %S* [[TMP2]])
; ARGPROMOTION-NEXT:    ret i32 0
;
bb:
  %tmp = getelementptr %S, %S* %arg1, i32 0, i32 0
  %tmp2 = load %S*, %S** %tmp
  %tmp3 = getelementptr %S, %S* %arg, i32 0, i32 0
  %tmp4 = load %S*, %S** %tmp3
  %tmp5 = call i32 @test_inf_promote_callee(%S* %tmp4, %S* %tmp2)

  ret i32 0
}

declare i32 @wibble(...)
