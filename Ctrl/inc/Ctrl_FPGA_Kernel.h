/**
 * @file Ctrl_FPGA_kernel.h
 * @brief FPGA-specific """temporal""" (😉) file to not include headers that the kernel compiler does not allow.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */
// Hitmap should be modified to have an equivalent file.
//   -- Manu 25/11/24 (let's see for how long this message lasts lmaoo.)
//   If you try to fix it and fail, add one bar:
//   █

/* Macro flag to change the behavior of other headers */
#define _CTRL_FPGA_KERNEL_FILE_

/* From hit_shape.h */
#define HIT_MAXDIMS 4 // HIT_MAXDIMS must be defined before including Ctrl_KHitTile.h

/* CTRL INCLUDED FILES */

#include <Core/Ctrl_KHitTile.h>
#include <Kernel/Ctrl_KernelProto.h>

/* MACROS */

/* Stripped-down version of Ctrl_newType() (from Ctrl_Core.h) */
#define Ctrl_NewType(type) hit_ktileNewType(type)

/* From hitmap2.h */
// Formatter disabled due to crazy alignment
// clang-format off
#define HIT2_COUNTPARAM(...) HIT2_COUNTPARAM_EXP(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
#define HIT2_COUNTPARAM_EXP(n1, n2, n3, n4, n5, n6, n7, n8, n9, n10, n11, n12, n13, n14, n15, n16, n17, n18, n19, n20, num, ...) num
// clang-format on

#define hit(var, ...)                    HIT2_HIT_VARIADIC(var, HIT2_COUNTPARAM(__VA_ARGS__), __VA_ARGS__)
#define HIT2_HIT_VARIADIC(var, num, ...) hit_tileElemAtNoStride(var, num, __VA_ARGS__)

/* From hit_tile.h */
#define hit_tileElemAtNoStride(var, ndims, ...) hit_tileElemAtNoStride##ndims(var, __VA_ARGS__)
#define hit_tileDimCard(var, dim)               ((var).card[dim])

/* From hit_tileP.h */
#define hit_tileElemAtNoStride1(var, pos)                    ((var).data[pos])
#define hit_tileElemAtNoStride2(var, pos1, pos2)             ((var).data[(pos1) * (var).origAcumCard[1] + (pos2)])
#define hit_tileElemAtNoStride3(var, pos1, pos2, pos3)       ((var).data[(pos1) * (var).origAcumCard[1] + (pos2) * (var).origAcumCard[2] + (pos3)])
#define hit_tileElemAtNoStride4(var, pos1, pos2, pos3, pos4) ((var).data[(pos1) * (var).origAcumCard[1] + (pos2) * (var).origAcumCard[2] + (pos3) * (var).origAcumCard[3] + (pos4)])
