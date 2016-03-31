; TODO: Add AVX512BW shift support
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=knl -mattr=+avx512dq | FileCheck %s --check-prefix=ALL --check-prefix=AVX512 --check-prefix=AVX512DQ

;
; Variable Shifts
;

define <8 x i64> @var_shift_v8i64(<8 x i64> %a, <8 x i64> %b) nounwind {
; ALL-LABEL: var_shift_v8i64:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsravq %zmm1, %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <8 x i64> %a, %b
  ret <8 x i64> %shift
}

define <16 x i32> @var_shift_v16i32(<16 x i32> %a, <16 x i32> %b) nounwind {
; ALL-LABEL: var_shift_v16i32:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsravd %zmm1, %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <16 x i32> %a, %b
  ret <16 x i32> %shift
}

define <32 x i16> @var_shift_v32i16(<32 x i16> %a, <32 x i16> %b) nounwind {
; ALL-LABEL: var_shift_v32i16:
; ALL:       ## BB#0:
; ALL-NEXT:    vpxor %ymm4, %ymm4, %ymm4
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm5 = ymm2[4],ymm4[4],ymm2[5],ymm4[5],ymm2[6],ymm4[6],ymm2[7],ymm4[7],ymm2[12],ymm4[12],ymm2[13],ymm4[13],ymm2[14],ymm4[14],ymm2[15],ymm4[15]
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm6 = ymm0[4,4,5,5,6,6,7,7,12,12,13,13,14,14,15,15]
; ALL-NEXT:    vpsravd %ymm5, %ymm6, %ymm5
; ALL-NEXT:    vpsrld $16, %ymm5, %ymm5
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm2 = ymm2[0],ymm4[0],ymm2[1],ymm4[1],ymm2[2],ymm4[2],ymm2[3],ymm4[3],ymm2[8],ymm4[8],ymm2[9],ymm4[9],ymm2[10],ymm4[10],ymm2[11],ymm4[11]
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm0 = ymm0[0,0,1,1,2,2,3,3,8,8,9,9,10,10,11,11]
; ALL-NEXT:    vpsravd %ymm2, %ymm0, %ymm0
; ALL-NEXT:    vpsrld $16, %ymm0, %ymm0
; ALL-NEXT:    vpackusdw %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm2 = ymm3[4],ymm4[4],ymm3[5],ymm4[5],ymm3[6],ymm4[6],ymm3[7],ymm4[7],ymm3[12],ymm4[12],ymm3[13],ymm4[13],ymm3[14],ymm4[14],ymm3[15],ymm4[15]
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm5 = ymm1[4,4,5,5,6,6,7,7,12,12,13,13,14,14,15,15]
; ALL-NEXT:    vpsravd %ymm2, %ymm5, %ymm2
; ALL-NEXT:    vpsrld $16, %ymm2, %ymm2
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm3 = ymm3[0],ymm4[0],ymm3[1],ymm4[1],ymm3[2],ymm4[2],ymm3[3],ymm4[3],ymm3[8],ymm4[8],ymm3[9],ymm4[9],ymm3[10],ymm4[10],ymm3[11],ymm4[11]
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm1 = ymm1[0,0,1,1,2,2,3,3,8,8,9,9,10,10,11,11]
; ALL-NEXT:    vpsravd %ymm3, %ymm1, %ymm1
; ALL-NEXT:    vpsrld $16, %ymm1, %ymm1
; ALL-NEXT:    vpackusdw %ymm2, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <32 x i16> %a, %b
  ret <32 x i16> %shift
}

define <64 x i8> @var_shift_v64i8(<64 x i8> %a, <64 x i8> %b) nounwind {
; ALL-LABEL: var_shift_v64i8:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsllw $5, %ymm2, %ymm2
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8],ymm2[8],ymm0[9],ymm2[9],ymm0[10],ymm2[10],ymm0[11],ymm2[11],ymm0[12],ymm2[12],ymm0[13],ymm2[13],ymm0[14],ymm2[14],ymm0[15],ymm2[15],ymm0[24],ymm2[24],ymm0[25],ymm2[25],ymm0[26],ymm2[26],ymm0[27],ymm2[27],ymm0[28],ymm2[28],ymm0[29],ymm2[29],ymm0[30],ymm2[30],ymm0[31],ymm2[31]
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm5 = ymm0[8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31]
; ALL-NEXT:    vpsraw $4, %ymm5, %ymm6
; ALL-NEXT:    vpblendvb %ymm4, %ymm6, %ymm5, %ymm5
; ALL-NEXT:    vpsraw $2, %ymm5, %ymm6
; ALL-NEXT:    vpaddw %ymm4, %ymm4, %ymm4
; ALL-NEXT:    vpblendvb %ymm4, %ymm6, %ymm5, %ymm5
; ALL-NEXT:    vpsraw $1, %ymm5, %ymm6
; ALL-NEXT:    vpaddw %ymm4, %ymm4, %ymm4
; ALL-NEXT:    vpblendvb %ymm4, %ymm6, %ymm5, %ymm4
; ALL-NEXT:    vpsrlw $8, %ymm4, %ymm4
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm2 = ymm0[0],ymm2[0],ymm0[1],ymm2[1],ymm0[2],ymm2[2],ymm0[3],ymm2[3],ymm0[4],ymm2[4],ymm0[5],ymm2[5],ymm0[6],ymm2[6],ymm0[7],ymm2[7],ymm0[16],ymm2[16],ymm0[17],ymm2[17],ymm0[18],ymm2[18],ymm0[19],ymm2[19],ymm0[20],ymm2[20],ymm0[21],ymm2[21],ymm0[22],ymm2[22],ymm0[23],ymm2[23]
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm0 = ymm0[0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23]
; ALL-NEXT:    vpsraw $4, %ymm0, %ymm5
; ALL-NEXT:    vpblendvb %ymm2, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $2, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm2
; ALL-NEXT:    vpblendvb %ymm2, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $1, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm2
; ALL-NEXT:    vpblendvb %ymm2, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsrlw $8, %ymm0, %ymm0
; ALL-NEXT:    vpackuswb %ymm4, %ymm0, %ymm0
; ALL-NEXT:    vpsllw $5, %ymm3, %ymm2
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm3 = ymm0[8],ymm2[8],ymm0[9],ymm2[9],ymm0[10],ymm2[10],ymm0[11],ymm2[11],ymm0[12],ymm2[12],ymm0[13],ymm2[13],ymm0[14],ymm2[14],ymm0[15],ymm2[15],ymm0[24],ymm2[24],ymm0[25],ymm2[25],ymm0[26],ymm2[26],ymm0[27],ymm2[27],ymm0[28],ymm2[28],ymm0[29],ymm2[29],ymm0[30],ymm2[30],ymm0[31],ymm2[31]
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8],ymm1[8],ymm0[9],ymm1[9],ymm0[10],ymm1[10],ymm0[11],ymm1[11],ymm0[12],ymm1[12],ymm0[13],ymm1[13],ymm0[14],ymm1[14],ymm0[15],ymm1[15],ymm0[24],ymm1[24],ymm0[25],ymm1[25],ymm0[26],ymm1[26],ymm0[27],ymm1[27],ymm0[28],ymm1[28],ymm0[29],ymm1[29],ymm0[30],ymm1[30],ymm0[31],ymm1[31]
; ALL-NEXT:    vpsraw $4, %ymm4, %ymm5
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $2, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm3, %ymm3, %ymm3
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $1, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm3, %ymm3, %ymm3
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm3
; ALL-NEXT:    vpsrlw $8, %ymm3, %ymm3
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm2 = ymm0[0],ymm2[0],ymm0[1],ymm2[1],ymm0[2],ymm2[2],ymm0[3],ymm2[3],ymm0[4],ymm2[4],ymm0[5],ymm2[5],ymm0[6],ymm2[6],ymm0[7],ymm2[7],ymm0[16],ymm2[16],ymm0[17],ymm2[17],ymm0[18],ymm2[18],ymm0[19],ymm2[19],ymm0[20],ymm2[20],ymm0[21],ymm2[21],ymm0[22],ymm2[22],ymm0[23],ymm2[23]
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm1 = ymm0[0],ymm1[0],ymm0[1],ymm1[1],ymm0[2],ymm1[2],ymm0[3],ymm1[3],ymm0[4],ymm1[4],ymm0[5],ymm1[5],ymm0[6],ymm1[6],ymm0[7],ymm1[7],ymm0[16],ymm1[16],ymm0[17],ymm1[17],ymm0[18],ymm1[18],ymm0[19],ymm1[19],ymm0[20],ymm1[20],ymm0[21],ymm1[21],ymm0[22],ymm1[22],ymm0[23],ymm1[23]
; ALL-NEXT:    vpsraw $4, %ymm1, %ymm4
; ALL-NEXT:    vpblendvb %ymm2, %ymm4, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $2, %ymm1, %ymm4
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm2
; ALL-NEXT:    vpblendvb %ymm2, %ymm4, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $1, %ymm1, %ymm4
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm2
; ALL-NEXT:    vpblendvb %ymm2, %ymm4, %ymm1, %ymm1
; ALL-NEXT:    vpsrlw $8, %ymm1, %ymm1
; ALL-NEXT:    vpackuswb %ymm3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <64 x i8> %a, %b
  ret <64 x i8> %shift
}

;
; Uniform Variable Shifts
;

define <8 x i64> @splatvar_shift_v8i64(<8 x i64> %a, <8 x i64> %b) nounwind {
; ALL-LABEL: splatvar_shift_v8i64:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsraq %xmm1, %zmm0, %zmm0
; ALL-NEXT:    retq
  %splat = shufflevector <8 x i64> %b, <8 x i64> undef, <8 x i32> zeroinitializer
  %shift = ashr <8 x i64> %a, %splat
  ret <8 x i64> %shift
}

define <16 x i32> @splatvar_shift_v16i32(<16 x i32> %a, <16 x i32> %b) nounwind {
; ALL-LABEL: splatvar_shift_v16i32:
; ALL:       ## BB#0:
; ALL-NEXT:    vxorps %xmm2, %xmm2, %xmm2
; ALL-NEXT:    vmovss %xmm1, %xmm2, %xmm1
; ALL-NEXT:    vpsrad %xmm1, %zmm0, %zmm0
; ALL-NEXT:    retq
  %splat = shufflevector <16 x i32> %b, <16 x i32> undef, <16 x i32> zeroinitializer
  %shift = ashr <16 x i32> %a, %splat
  ret <16 x i32> %shift
}

define <32 x i16> @splatvar_shift_v32i16(<32 x i16> %a, <32 x i16> %b) nounwind {
; ALL-LABEL: splatvar_shift_v32i16:
; ALL:       ## BB#0:
; ALL-NEXT:    vmovd %xmm2, %eax
; ALL-NEXT:    movzwl %ax, %eax
; ALL-NEXT:    vmovd %eax, %xmm2
; ALL-NEXT:    vpsraw %xmm2, %ymm0, %ymm0
; ALL-NEXT:    vpsraw %xmm2, %ymm1, %ymm1
; ALL-NEXT:    retq
  %splat = shufflevector <32 x i16> %b, <32 x i16> undef, <32 x i32> zeroinitializer
  %shift = ashr <32 x i16> %a, %splat
  ret <32 x i16> %shift
}

define <64 x i8> @splatvar_shift_v64i8(<64 x i8> %a, <64 x i8> %b) nounwind {
; ALL-LABEL: splatvar_shift_v64i8:
; ALL:       ## BB#0:
; ALL-NEXT:    vpbroadcastb %xmm2, %ymm2
; ALL-NEXT:    vpsllw $5, %ymm2, %ymm2
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm3 = ymm0[8],ymm2[8],ymm0[9],ymm2[9],ymm0[10],ymm2[10],ymm0[11],ymm2[11],ymm0[12],ymm2[12],ymm0[13],ymm2[13],ymm0[14],ymm2[14],ymm0[15],ymm2[15],ymm0[24],ymm2[24],ymm0[25],ymm2[25],ymm0[26],ymm2[26],ymm0[27],ymm2[27],ymm0[28],ymm2[28],ymm0[29],ymm2[29],ymm0[30],ymm2[30],ymm0[31],ymm2[31]
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31]
; ALL-NEXT:    vpsraw $4, %ymm4, %ymm5
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $2, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm3, %ymm3, %ymm6
; ALL-NEXT:    vpblendvb %ymm6, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $1, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm6, %ymm6, %ymm7
; ALL-NEXT:    vpblendvb %ymm7, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsrlw $8, %ymm4, %ymm4
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm2 = ymm0[0],ymm2[0],ymm0[1],ymm2[1],ymm0[2],ymm2[2],ymm0[3],ymm2[3],ymm0[4],ymm2[4],ymm0[5],ymm2[5],ymm0[6],ymm2[6],ymm0[7],ymm2[7],ymm0[16],ymm2[16],ymm0[17],ymm2[17],ymm0[18],ymm2[18],ymm0[19],ymm2[19],ymm0[20],ymm2[20],ymm0[21],ymm2[21],ymm0[22],ymm2[22],ymm0[23],ymm2[23]
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm0 = ymm0[0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23]
; ALL-NEXT:    vpsraw $4, %ymm0, %ymm5
; ALL-NEXT:    vpblendvb %ymm2, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $2, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm8
; ALL-NEXT:    vpblendvb %ymm8, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $1, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm8, %ymm8, %ymm9
; ALL-NEXT:    vpblendvb %ymm9, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsrlw $8, %ymm0, %ymm0
; ALL-NEXT:    vpackuswb %ymm4, %ymm0, %ymm0
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8],ymm1[8],ymm0[9],ymm1[9],ymm0[10],ymm1[10],ymm0[11],ymm1[11],ymm0[12],ymm1[12],ymm0[13],ymm1[13],ymm0[14],ymm1[14],ymm0[15],ymm1[15],ymm0[24],ymm1[24],ymm0[25],ymm1[25],ymm0[26],ymm1[26],ymm0[27],ymm1[27],ymm0[28],ymm1[28],ymm0[29],ymm1[29],ymm0[30],ymm1[30],ymm0[31],ymm1[31]
; ALL-NEXT:    vpsraw $4, %ymm4, %ymm5
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm3
; ALL-NEXT:    vpsraw $2, %ymm3, %ymm4
; ALL-NEXT:    vpblendvb %ymm6, %ymm4, %ymm3, %ymm3
; ALL-NEXT:    vpsraw $1, %ymm3, %ymm4
; ALL-NEXT:    vpblendvb %ymm7, %ymm4, %ymm3, %ymm3
; ALL-NEXT:    vpsrlw $8, %ymm3, %ymm3
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm1 = ymm0[0],ymm1[0],ymm0[1],ymm1[1],ymm0[2],ymm1[2],ymm0[3],ymm1[3],ymm0[4],ymm1[4],ymm0[5],ymm1[5],ymm0[6],ymm1[6],ymm0[7],ymm1[7],ymm0[16],ymm1[16],ymm0[17],ymm1[17],ymm0[18],ymm1[18],ymm0[19],ymm1[19],ymm0[20],ymm1[20],ymm0[21],ymm1[21],ymm0[22],ymm1[22],ymm0[23],ymm1[23]
; ALL-NEXT:    vpsraw $4, %ymm1, %ymm4
; ALL-NEXT:    vpblendvb %ymm2, %ymm4, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $2, %ymm1, %ymm2
; ALL-NEXT:    vpblendvb %ymm8, %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $1, %ymm1, %ymm2
; ALL-NEXT:    vpblendvb %ymm9, %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpsrlw $8, %ymm1, %ymm1
; ALL-NEXT:    vpackuswb %ymm3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %splat = shufflevector <64 x i8> %b, <64 x i8> undef, <64 x i32> zeroinitializer
  %shift = ashr <64 x i8> %a, %splat
  ret <64 x i8> %shift
}

;
; Constant Shifts
;

define <8 x i64> @constant_shift_v8i64(<8 x i64> %a) nounwind {
; ALL-LABEL: constant_shift_v8i64:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsravq {{.*}}(%rip), %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <8 x i64> %a, <i64 1, i64 7, i64 31, i64 62, i64 1, i64 7, i64 31, i64 62>
  ret <8 x i64> %shift
}

define <16 x i32> @constant_shift_v16i32(<16 x i32> %a) nounwind {
; ALL-LABEL: constant_shift_v16i32:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsravd {{.*}}(%rip), %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <16 x i32> %a, <i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 8, i32 7, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 8, i32 7>
  ret <16 x i32> %shift
}

define <32 x i16> @constant_shift_v32i16(<32 x i16> %a) nounwind {
; ALL-LABEL: constant_shift_v32i16:
; ALL:       ## BB#0:
; ALL-NEXT:    vpxor %ymm2, %ymm2, %ymm2
; ALL-NEXT:    vmovdqa {{.*#+}} ymm3 = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15]
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm4 = ymm3[4],ymm2[4],ymm3[5],ymm2[5],ymm3[6],ymm2[6],ymm3[7],ymm2[7],ymm3[12],ymm2[12],ymm3[13],ymm2[13],ymm3[14],ymm2[14],ymm3[15],ymm2[15]
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm5 = ymm0[4,4,5,5,6,6,7,7,12,12,13,13,14,14,15,15]
; ALL-NEXT:    vpsravd %ymm4, %ymm5, %ymm5
; ALL-NEXT:    vpsrld $16, %ymm5, %ymm5
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm2 = ymm3[0],ymm2[0],ymm3[1],ymm2[1],ymm3[2],ymm2[2],ymm3[3],ymm2[3],ymm3[8],ymm2[8],ymm3[9],ymm2[9],ymm3[10],ymm2[10],ymm3[11],ymm2[11]
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm0 = ymm0[0,0,1,1,2,2,3,3,8,8,9,9,10,10,11,11]
; ALL-NEXT:    vpsravd %ymm2, %ymm0, %ymm0
; ALL-NEXT:    vpsrld $16, %ymm0, %ymm0
; ALL-NEXT:    vpackusdw %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpunpckhwd {{.*#+}} ymm3 = ymm1[4,4,5,5,6,6,7,7,12,12,13,13,14,14,15,15]
; ALL-NEXT:    vpsravd %ymm4, %ymm3, %ymm3
; ALL-NEXT:    vpsrld $16, %ymm3, %ymm3
; ALL-NEXT:    vpunpcklwd {{.*#+}} ymm1 = ymm1[0,0,1,1,2,2,3,3,8,8,9,9,10,10,11,11]
; ALL-NEXT:    vpsravd %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpsrld $16, %ymm1, %ymm1
; ALL-NEXT:    vpackusdw %ymm3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <32 x i16> %a, <i16 0, i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 8, i16 9, i16 10, i16 11, i16 12, i16 13, i16 14, i16 15, i16 0, i16 1, i16 2, i16 3, i16 4, i16 5, i16 6, i16 7, i16 8, i16 9, i16 10, i16 11, i16 12, i16 13, i16 14, i16 15>
  ret <32 x i16> %shift
}

define <64 x i8> @constant_shift_v64i8(<64 x i8> %a) nounwind {
; ALL-LABEL: constant_shift_v64i8:
; ALL:       ## BB#0:
; ALL-NEXT:    vmovdqa {{.*#+}} ymm2 = [0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,0,0,1,2,3,4,5,6,7,7,6,5,4,3,2,1,0]
; ALL-NEXT:    vpsllw $5, %ymm2, %ymm2
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm3 = ymm0[8],ymm2[8],ymm0[9],ymm2[9],ymm0[10],ymm2[10],ymm0[11],ymm2[11],ymm0[12],ymm2[12],ymm0[13],ymm2[13],ymm0[14],ymm2[14],ymm0[15],ymm2[15],ymm0[24],ymm2[24],ymm0[25],ymm2[25],ymm0[26],ymm2[26],ymm0[27],ymm2[27],ymm0[28],ymm2[28],ymm0[29],ymm2[29],ymm0[30],ymm2[30],ymm0[31],ymm2[31]
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8,8,9,9,10,10,11,11,12,12,13,13,14,14,15,15,24,24,25,25,26,26,27,27,28,28,29,29,30,30,31,31]
; ALL-NEXT:    vpsraw $4, %ymm4, %ymm5
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $2, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm3, %ymm3, %ymm6
; ALL-NEXT:    vpblendvb %ymm6, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsraw $1, %ymm4, %ymm5
; ALL-NEXT:    vpaddw %ymm6, %ymm6, %ymm7
; ALL-NEXT:    vpblendvb %ymm7, %ymm5, %ymm4, %ymm4
; ALL-NEXT:    vpsrlw $8, %ymm4, %ymm4
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm2 = ymm0[0],ymm2[0],ymm0[1],ymm2[1],ymm0[2],ymm2[2],ymm0[3],ymm2[3],ymm0[4],ymm2[4],ymm0[5],ymm2[5],ymm0[6],ymm2[6],ymm0[7],ymm2[7],ymm0[16],ymm2[16],ymm0[17],ymm2[17],ymm0[18],ymm2[18],ymm0[19],ymm2[19],ymm0[20],ymm2[20],ymm0[21],ymm2[21],ymm0[22],ymm2[22],ymm0[23],ymm2[23]
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm0 = ymm0[0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,16,16,17,17,18,18,19,19,20,20,21,21,22,22,23,23]
; ALL-NEXT:    vpsraw $4, %ymm0, %ymm5
; ALL-NEXT:    vpblendvb %ymm2, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $2, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm2, %ymm2, %ymm8
; ALL-NEXT:    vpblendvb %ymm8, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $1, %ymm0, %ymm5
; ALL-NEXT:    vpaddw %ymm8, %ymm8, %ymm9
; ALL-NEXT:    vpblendvb %ymm9, %ymm5, %ymm0, %ymm0
; ALL-NEXT:    vpsrlw $8, %ymm0, %ymm0
; ALL-NEXT:    vpackuswb %ymm4, %ymm0, %ymm0
; ALL-NEXT:    vpunpckhbw {{.*#+}} ymm4 = ymm0[8],ymm1[8],ymm0[9],ymm1[9],ymm0[10],ymm1[10],ymm0[11],ymm1[11],ymm0[12],ymm1[12],ymm0[13],ymm1[13],ymm0[14],ymm1[14],ymm0[15],ymm1[15],ymm0[24],ymm1[24],ymm0[25],ymm1[25],ymm0[26],ymm1[26],ymm0[27],ymm1[27],ymm0[28],ymm1[28],ymm0[29],ymm1[29],ymm0[30],ymm1[30],ymm0[31],ymm1[31]
; ALL-NEXT:    vpsraw $4, %ymm4, %ymm5
; ALL-NEXT:    vpblendvb %ymm3, %ymm5, %ymm4, %ymm3
; ALL-NEXT:    vpsraw $2, %ymm3, %ymm4
; ALL-NEXT:    vpblendvb %ymm6, %ymm4, %ymm3, %ymm3
; ALL-NEXT:    vpsraw $1, %ymm3, %ymm4
; ALL-NEXT:    vpblendvb %ymm7, %ymm4, %ymm3, %ymm3
; ALL-NEXT:    vpsrlw $8, %ymm3, %ymm3
; ALL-NEXT:    vpunpcklbw {{.*#+}} ymm1 = ymm0[0],ymm1[0],ymm0[1],ymm1[1],ymm0[2],ymm1[2],ymm0[3],ymm1[3],ymm0[4],ymm1[4],ymm0[5],ymm1[5],ymm0[6],ymm1[6],ymm0[7],ymm1[7],ymm0[16],ymm1[16],ymm0[17],ymm1[17],ymm0[18],ymm1[18],ymm0[19],ymm1[19],ymm0[20],ymm1[20],ymm0[21],ymm1[21],ymm0[22],ymm1[22],ymm0[23],ymm1[23]
; ALL-NEXT:    vpsraw $4, %ymm1, %ymm4
; ALL-NEXT:    vpblendvb %ymm2, %ymm4, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $2, %ymm1, %ymm2
; ALL-NEXT:    vpblendvb %ymm8, %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpsraw $1, %ymm1, %ymm2
; ALL-NEXT:    vpblendvb %ymm9, %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpsrlw $8, %ymm1, %ymm1
; ALL-NEXT:    vpackuswb %ymm3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <64 x i8> %a, <i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0, i8 0, i8 1, i8 2, i8 3, i8 4, i8 5, i8 6, i8 7, i8 7, i8 6, i8 5, i8 4, i8 3, i8 2, i8 1, i8 0>
  ret <64 x i8> %shift
}

;
; Uniform Constant Shifts
;

define <8 x i64> @splatconstant_shift_v8i64(<8 x i64> %a) nounwind {
; ALL-LABEL: splatconstant_shift_v8i64:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsraq $7, %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <8 x i64> %a, <i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7, i64 7>
  ret <8 x i64> %shift
}

define <16 x i32> @splatconstant_shift_v16i32(<16 x i32> %a) nounwind {
; ALL-LABEL: splatconstant_shift_v16i32:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsrad $5, %zmm0, %zmm0
; ALL-NEXT:    retq
  %shift = ashr <16 x i32> %a, <i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5, i32 5>
  ret <16 x i32> %shift
}

define <32 x i16> @splatconstant_shift_v32i16(<32 x i16> %a) nounwind {
; ALL-LABEL: splatconstant_shift_v32i16:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsraw $3, %ymm0, %ymm0
; ALL-NEXT:    vpsraw $3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <32 x i16> %a, <i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3>
  ret <32 x i16> %shift
}

define <64 x i8> @splatconstant_shift_v64i8(<64 x i8> %a) nounwind {
; ALL-LABEL: splatconstant_shift_v64i8:
; ALL:       ## BB#0:
; ALL-NEXT:    vpsrlw $3, %ymm0, %ymm0
; ALL-NEXT:    vmovdqa {{.*#+}} ymm2 = [31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31,31]
; ALL-NEXT:    vpand %ymm2, %ymm0, %ymm0
; ALL-NEXT:    vmovdqa {{.*#+}} ymm3 = [16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16]
; ALL-NEXT:    vpxor %ymm3, %ymm0, %ymm0
; ALL-NEXT:    vpsubb %ymm3, %ymm0, %ymm0
; ALL-NEXT:    vpsrlw $3, %ymm1, %ymm1
; ALL-NEXT:    vpand %ymm2, %ymm1, %ymm1
; ALL-NEXT:    vpxor %ymm3, %ymm1, %ymm1
; ALL-NEXT:    vpsubb %ymm3, %ymm1, %ymm1
; ALL-NEXT:    retq
  %shift = ashr <64 x i8> %a, <i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3, i8 3>
  ret <64 x i8> %shift
}
