create_pblock pblock_medianblur_0
add_cells_to_pblock [get_pblocks pblock_medianblur_0] [get_cells -quiet [list design_1_i/MedianBlur_0]]
resize_pblock [get_pblocks pblock_medianblur_0] -add {SLICE_X52Y12:SLICE_X105Y45}
resize_pblock [get_pblocks pblock_medianblur_0] -add {DSP48_X3Y6:DSP48_X4Y17}
resize_pblock [get_pblocks pblock_medianblur_0] -add {RAMB18_X3Y6:RAMB18_X4Y17}
resize_pblock [get_pblocks pblock_medianblur_0] -add {RAMB36_X3Y3:RAMB36_X4Y8}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_medianblur_0]
set_property SNAPPING_MODE ROUTING [get_pblocks pblock_medianblur_0]

create_pblock pblock_gaussianblur_0
add_cells_to_pblock [get_pblocks pblock_gaussianblur_0] [get_cells -quiet [list design_1_i/GaussianBlur_0]]
resize_pblock [get_pblocks pblock_gaussianblur_0] -add {SLICE_X54Y56:SLICE_X107Y93}
resize_pblock [get_pblocks pblock_gaussianblur_0] -add {DSP48_X3Y24:DSP48_X4Y35}
resize_pblock [get_pblocks pblock_gaussianblur_0] -add {RAMB18_X3Y24:RAMB18_X5Y35}
resize_pblock [get_pblocks pblock_gaussianblur_0] -add {RAMB36_X3Y12:RAMB36_X5Y17}
set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_gaussianblur_0]
set_property SNAPPING_MODE ROUTING [get_pblocks pblock_gaussianblur_0]

