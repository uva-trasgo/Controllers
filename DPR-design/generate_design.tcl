namespace import ::tcl::mathfunc::*

set KERNELS [lrange $argv 1 end]
set N_RR [lindex $argv 0]

puts $N_RR
puts $KERNELS

# Extracted from https://rosettacode.org/wiki/Permutations_with_repetitions#Tcl
proc permutate {values size offset} {
    set count [llength $values]
    set arr [list]
    for {set i 0} {$i < $size} {incr i} {
        set selector [expr [round [expr $offset / [pow $count $i]]] % $count];
        lappend arr [lindex $values $selector]

    }
    return $arr
}

proc permutations {values size} {
    set a [list]
    set c [pow [llength $values] $size]
    for {set i 0} {$i < $c} {incr i} {
        set permutation [permutate $values $size $i]
        lappend a $permutation
    }
    return $a
}

#set cells_list { MedianBlur_0 GaussianBlur_0 }
set cells_list { { MedianBlur_0 } { MedianBlur_0 GaussianBlur_0 } { MedianBlur_0 GaussianBlur_0 MedianBlur_1 } {
        MedianBlur_0 GaussianBlur_0 MedianBlur_1 GaussianBlur_1 } }

open_checkpoint design-${N_RR}RR-axi_firewall/Synth/Static/static.dcp
#open_checkpoint design-${N_RR}RR/Synth/Static/static.dcp

set k 0
foreach cell [lindex $cells_list $N_RR-1] {
        read_checkpoint -cell design_1_i/$cell Synth/reconfig_modules/[lindex $KERNELS $k].dcp
        set_property HD.RECONFIGURABLE 1 [get_cells design_1_i/$cell]
        incr k
}

write_checkpoint Checkpoint/top_link.dcp

#Define pblocks
read_xdc design-${N_RR}RR-axi_firewall/Sources/xdc/fplan.xdc
#read_xdc design-${N_RR}RR/Sources/xdc/fplan.xdc

#FIRST CONFIGURATION
opt_design
place_design
route_design

set full_name ""
for {set k 0} {$k < $N_RR-1} {incr k} {
        append full_name [lindex $KERNELS $k]
        append full_name "-"
}
append full_name [lindex $KERNELS $N_RR-1]

write_checkpoint -force Implement/$full_name/top_route_design.dcp

#Save checkpoints for the reconfigurable module
foreach cell [lindex $cells_list $N_RR-1] {
        write_checkpoint -force -cell design_1_i/$cell Checkpoint/${cell}_route_design.dcp
}

##Clear out existing RMs
foreach cell [lindex $cells_list $N_RR-1] {
        update_design -cell design_1_i/$cell -black_box
}
lock_design -level routing
write_checkpoint -force Checkpoint/static_route_design.dcp

#ALL THE CONFIGURATIONS
set kernel_permutations [ permutations $KERNELS  $N_RR ]
puts $kernel_permutations

set implementation_list {}

foreach kernel_permutation $kernel_permutations {
        open_checkpoint Checkpoint/static_route_design.dcp

        for {set k 0} {$k < $N_RR} {incr k} {
                read_checkpoint -cell design_1_i/[lindex [lindex $cells_list $N_RR] $k] Synth/reconfig_modules/[lindex $kernel_permutation $k].dcp
        }
        opt_design
        place_design
        route_design

        # Generate name of the bitstream
        set bitname ""

        for {set k 0} {$k < $N_RR-1} {incr k} {
                append bitname [lindex $kernel_permutation $k]
                append bitname "-"
        }
        append bitname [lindex $kernel_permutation $N_RR-1]

        lappend implementation_list $bitname

        write_checkpoint -force Implement/[lindex $implementation_list end]/top_route_design.dcp
}



foreach kernel_permutation $kernel_permutations {
        set bitname ""

        for {set k 0} {$k < $N_RR-1} {incr k} {
                append bitname [lindex $kernel_permutation $k]
                append bitname "-"
        }
        append bitname [lindex $kernel_permutation $N_RR-1]

        lappend implementation_list $bitname
}

set implementation_dir_list {}
foreach implementation $implementation_list {
        lappend implementation_dir_list Implement/$implementation/top_route_design.dcp
}

#Verify that the partial designs are correct
pr_verify -initial Implement/$full_name/top_route_design.dcp -additional $implementation_dir_list


#Generate bitstreams
#FULL BITSTREAM
open_checkpoint Implement/$full_name/top_route_design.dcp
write_bitstream -force -file Bitstreams/$full_name.bit
close_project

#PARTIAL BITSTREAMS
foreach bitname $implementation_list {
        open_checkpoint Implement/$bitname/top_route_design.dcp
        write_bitstream -force -file Bitstreams/$bitname.bit
        close_project
}

