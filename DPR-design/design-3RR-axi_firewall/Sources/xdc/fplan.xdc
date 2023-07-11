add_cells_to_pblock [get_pblocks pblock_jacobi_0] [get_cells -quiet [list design_1_i/Jacobi_1]]

add_cells_to_pblock [get_pblocks pblock_mandelbrot_0] [get_cells -quiet [list design_1_i/Mandelbrot_1]]

add_cells_to_pblock [get_pblocks pblock_Jacobi_0_1] [get_cells -quiet [list design_1_i/Jacobi_0]]

create_pblock pblock_MedianBlur_0
add_cells_to_pblock [get_pblocks pblock_MedianBlur_0] [get_cells -quiet [list design_1_i/MedianBlur_0]]
resize_pblock [get_pblocks pblock_MedianBlur_0] -add {SLICE_X54Y107:SLICE_X107Y143}
resize_pblock [get_pblocks pblock_MedianBlur_0] -add {DSP48_X3Y44:DSP48_X4Y55}
resize_pblock [get_pblocks pblock_MedianBlur_0] -add {RAMB18_X3Y44:RAMB18_X5Y55}
resize_pblock [get_pblocks pblock_MedianBlur_0] -add {RAMB36_X3Y22:RAMB36_X5Y27}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_MedianBlur_0]
set_property SNAPPING_MODE ROUTING [get_pblocks pblock_MedianBlur_0]
create_pblock pblock_GaussianBlur_0
add_cells_to_pblock [get_pblocks pblock_GaussianBlur_0] [get_cells -quiet [list design_1_i/GaussianBlur_0]]
resize_pblock [get_pblocks pblock_GaussianBlur_0] -add {SLICE_X54Y54:SLICE_X111Y93}
resize_pblock [get_pblocks pblock_GaussianBlur_0] -add {DSP48_X3Y22:DSP48_X4Y35}
resize_pblock [get_pblocks pblock_GaussianBlur_0] -add {RAMB18_X4Y22:RAMB18_X5Y35}
resize_pblock [get_pblocks pblock_GaussianBlur_0] -add {RAMB36_X4Y11:RAMB36_X5Y17}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_GaussianBlur_0]
set_property SNAPPING_MODE ROUTING [get_pblocks pblock_GaussianBlur_0]
create_pblock pblock_MedianBlur_1
add_cells_to_pblock [get_pblocks pblock_MedianBlur_1] [get_cells -quiet [list design_1_i/MedianBlur_1]]
resize_pblock [get_pblocks pblock_MedianBlur_1] -add {SLICE_X54Y2:SLICE_X113Y43}
resize_pblock [get_pblocks pblock_MedianBlur_1] -add {DSP48_X3Y2:DSP48_X4Y15}
resize_pblock [get_pblocks pblock_MedianBlur_1] -add {RAMB18_X4Y2:RAMB18_X5Y15}
resize_pblock [get_pblocks pblock_MedianBlur_1] -add {RAMB36_X4Y1:RAMB36_X5Y7}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_MedianBlur_1]
set_property SNAPPING_MODE ROUTING [get_pblocks pblock_MedianBlur_1]
