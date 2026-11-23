; Synthetic reproduction of aggregate reinterpretation through pointers.
; No Autodesk code or shader capture is included.
target triple = "dxil-ms-dx"
%Matrix = type { [3 x <4 x float>] }

define void @main() {
entry:
  %source = alloca <12 x float>
  %destination = alloca <12 x float>
  %from = bitcast <12 x float>* %source to %Matrix*
  %matrix = load volatile %Matrix, %Matrix* %from
  %to = bitcast <12 x float>* %destination to %Matrix*
  store volatile %Matrix %matrix, %Matrix* %to
  ret void
}

!dx.version = !{!0}
!dx.shaderModel = !{!1}
!dx.entryPoints = !{!2}
!0 = !{i32 1, i32 0}
!1 = !{!"cs", i32 6, i32 0}
!2 = !{void ()* @main, !"main", null, null, !3}
!3 = !{i32 4, !4}
!4 = !{i32 1, i32 1, i32 1}
