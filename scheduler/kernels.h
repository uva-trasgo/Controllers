#ifndef _CTRL_INCLUDE_KERNELS
#define _CTRL_INCLUDE_KERNELS

CTRL_KERNEL_CHAR(GaussianBlur, MANUAL, 8, 8);
CTRL_KERNEL_CHAR(MedianBlur, MANUAL, LOCAL_SIZE_0, LOCAL_SIZE_0);

CTRL_KERNEL_PROTO( MedianBlur,
        1, PYNQ, DEFAULT, 5,
        IN, HitTile_int, in_matrix,
        OUT, HitTile_int, out_matrix,
        INVAL, int, H,
        INVAL, int, W,
        INVAL, int, iters);

CTRL_KERNEL_PROTO( GaussianBlur,
        1, PYNQ, DEFAULT, 4,
        IN, HitTile_int, in_matrix,
        OUT, HitTile_int, out_matrix,
        INVAL, int, H,
        INVAL, int, W);

#endif // _CTRL_INCLUDE_KERNELS
