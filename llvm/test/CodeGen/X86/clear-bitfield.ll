; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-- -combiner-reduce-load-op-store-width=false | FileCheck %s

%struct.bit_fields = type { i32 }

define void @clear_b1(%struct.bit_fields* %ptr) {
; CHECK-LABEL: clear_b1:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    andl $-2, (%rdi)
; CHECK-NEXT:    retq
entry:
  %0 = bitcast %struct.bit_fields* %ptr to i32*
  %bf.load = load i32, i32* %0
  %bf.clear = and i32 %bf.load, -2
  store i32 %bf.clear, i32* %0
  ret void
}

define void @clear16(%struct.bit_fields* %ptr) {
; CHECK-LABEL: clear16:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    andw $-2, (%rdi)
; CHECK-NEXT:    retq
entry:
  %0 = bitcast %struct.bit_fields* %ptr to i16*
  %bf.load = load i16, i16* %0
  %bf.clear = and i16 %bf.load, -2
  store i16 %bf.clear, i16* %0
  ret void
}
