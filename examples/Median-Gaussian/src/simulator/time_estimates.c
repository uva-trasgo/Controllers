#include "pynq_api.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MM_AXI_SIZE 65536
#define BRAM_SIZE 8192
#define N_SAVED_VARS 10

#include <omp.h>

int generated_interrupts[2];
typedef struct { 
    volatile int *data; 
    int origAcumCard[4]; 
    int card[3]; 
    int offset; 
} KHitTile_int;

typedef struct { 
    int origAcumCard[4]; 
    int card[3]; 
    int offset; 
} fpga_wrapper_KHitTile_int;

typedef volatile int * data_KHitTile_int;

fpga_wrapper_KHitTile_int matrix_wrapper[2];
data_KHitTile_int matrix_data[2];

struct context {
    //Pointers to the loop index variables and the integer variables. Used for the user to record
    //for which variables the context should be saved when they call the macro save_context().
    int var[N_SAVED_VARS];
    int init_var[N_SAVED_VARS]; // initial value of variable
    int incr_var[N_SAVED_VARS]; // increment of the loop variable
    int saved[N_SAVED_VARS];
    int valid; // set to 0 if the kernel was interrupted in the middle of a saving context operation. 1 otherwise
};

// IP's addresses
#define IP_CTRL                 0x00
#define AP_START                0x1
#define GIER                    0x04
#define IP_IER                  0x08
#define IP_ISR                  0x0C
#define INTRPT_AP_DONE          0x1
#define INTRPT_AP_READY         0x2
#define OUT_REG                 0x10
#define INP_REG                 0x20
#define _INTRPT                 INTRPT_AP_DONE
#define IN_ARRAY                0x18
#define OUT_ARRAY               0x24
#define ITERS                   0x30

int bram_ctrl_address[2];
int rr_address[2];
int ktile_wrapper_args_offset[3];
int ktile_data_args_offset[3];
int float_args_offset[8];
int int_args_offset[8];
int return_offset;
int axi_firewall_master_address[2];
int axi_firewall_slave_address[2];

PYNQ_GPIO decouple_gpio[2];
PYNQ_GPIO reset_gpio[2];
PYNQ_GPIO decouple_status_gpio[2];
PYNQ_GPIO decouple_rst_clk_gpio[2];

PYNQ_HLS axi_firewall_master[2];
PYNQ_HLS kernel_state[2];

PYNQ_AXI_INTERRUPT_CONTROLLER int_controller;
PYNQ_UIO uio_state;


int * data[2];
PYNQ_SHARED_MEMORY sh_mem[2];

PYNQ_MMIO_WINDOW bram[2];

void clean_bram(int rr) {
    printf("Before cleaning bram\n");
    struct context context;
    for(int i = 0; i < 10; i++) {
        context.var[i] = 0;
        context.init_var[i] = 0;
        context.incr_var[i] = 0;
        context.saved[i] = 0;
    }
    context.valid = 1;

    //PYNQ_writeMMIO(&BRAMController[rr].bram, clear, 0x0, BRAM_SIZE);
    PYNQ_writeMMIO(&bram[rr], &context, 0x0, sizeof(context));
    //printf("After write\n");
    printf("After cleaning bram\n");
}

void setup_scheduler() {
    for(int i = 0; i < 2; i++) generated_interrupts[i] = 0;
    //setup_dma();
    bram_ctrl_address[0] = 0x40020000;
    bram_ctrl_address[1] = 0x40022000;

    rr_address[0] = 0x40000000;
    rr_address[1] = 0x40010000;

    // Arguments offsets
    ktile_wrapper_args_offset[0] = 0x10;
    ktile_wrapper_args_offset[1] = 0x40;
    ktile_wrapper_args_offset[2] = 0x70;

    ktile_data_args_offset[0] = 0x34;
    ktile_data_args_offset[1] = 0x64;
    ktile_data_args_offset[2] = 0x94;

    int_args_offset[0] = 0xA0;
    int_args_offset[1] = 0xA8;
    int_args_offset[2] = 0xB0;
    int_args_offset[3] = 0xB8;
    int_args_offset[4] = 0xC0;
    int_args_offset[5] = 0xC8;
    int_args_offset[6] = 0xD0;
    int_args_offset[7] = 0xD8;

    float_args_offset[0] = 0xE0;
    float_args_offset[1] = 0xE8;
    float_args_offset[2] = 0xF0;
    float_args_offset[3] = 0xF8;
    float_args_offset[4] = 0x100;
    float_args_offset[5] = 0x108;
    float_args_offset[6] = 0x110;
    float_args_offset[7] = 0x118;

    return_offset = 0x120;

    axi_firewall_master_address[0] = 0x40030000;
    axi_firewall_master_address[1] = 0x40040000;
    axi_firewall_slave_address[0] = 0x40050000;
    axi_firewall_slave_address[1] = 0x40060000;

    PYNQ_openGPIO(&decouple_gpio[0], 960, GPIO_OUT);
    PYNQ_openGPIO(&decouple_gpio[1], 961, GPIO_OUT);
    PYNQ_openGPIO(&reset_gpio[0], 962, GPIO_OUT);
    PYNQ_openGPIO(&reset_gpio[1], 963, GPIO_OUT);
    PYNQ_openGPIO(&decouple_status_gpio[0], 964, GPIO_IN);
    PYNQ_openGPIO(&decouple_status_gpio[1], 965, GPIO_IN);
    PYNQ_openGPIO(&decouple_rst_clk_gpio[0], 967, GPIO_OUT);
    PYNQ_openGPIO(&decouple_rst_clk_gpio[1], 968, GPIO_OUT);

    PYNQ_openHLS(&axi_firewall_master[0], axi_firewall_master_address[0], MM_AXI_SIZE);
    PYNQ_openHLS(&axi_firewall_master[1], axi_firewall_master_address[1], MM_AXI_SIZE);

    for(int i = 0; i < 2; i++)
        PYNQ_openHLS(&kernel_state[i], rr_address[i], MM_AXI_SIZE);

    for(int i = 0 ; i < 2; i++) {
        PYNQ_allocatedSharedMemory(&sh_mem[i], 602*602 * sizeof(int), 1);
        data[i] = (int*)sh_mem[i].pointer;
    }

    for(int i = 0; i < 602; i++) {
        for(int j = 0; j < 602; j++) {
            data[0][i * 602 + j] = i * 602 + j;
            data[1][i * 602 + j] = 0;
        }
    }



    for(int i = 0; i < 2; i++) {
        matrix_data[i] = (volatile int*)sh_mem[i].physical_address;
        //printf("matrix_data: %x, %lx\n", matrix_data[i], sh_mem[i].physical_address);
    }

    for(int i = 0; i < 2; i++) {
        for(int j = 0; j < 4; j++) {
            matrix_wrapper[i].origAcumCard[j] = 0;
        }
        for(int j = 0; j < 3; j++) {
            matrix_wrapper[i].card[i] = 0;
        }
        matrix_wrapper[i].offset = 0;
    }

    for(int rr = 0; rr < 2; rr++)
        PYNQ_createMMIOWindow(&bram[rr], bram_ctrl_address[rr], BRAM_SIZE);

    PYNQ_openInterruptController(&int_controller, 0x41200000);
    PYNQ_openUIO(&uio_state, 0);

    for(int rr = 0; rr < 2; rr++) {
        PYNQ_registerInterrupt(&int_controller, rr, 1);
    }
}

void clean_output() {
    for(int i = 0; i < 602*602; i++) data[1][i] = 0;
}

int output_checksum() {
    int sum = 0;
    for(int i = 0; i < 602*602; i++) sum += data[1][i];

    return sum;
}

int enable = 1;
int disable = 0;

int H = 600;
int W = 600;
//int iters = 2;

void print_axi_faults() {
    int fault_master[2];

    PYNQ_readFromHLS(&axi_firewall_master[0], &fault_master[0], 0x0, sizeof(int));
    PYNQ_readFromHLS(&axi_firewall_master[1], &fault_master[1], 0x0, sizeof(int));

    printf("fault_master0: %d\nfault_master1: %d\n", fault_master[0], fault_master[1]);
}

void launch(int rr, int kernel, int iters) {
    printf("Hello from launch\n");
    //print_axi_faults();
    PYNQ_writeToHLS(&kernel_state[rr], &matrix_wrapper[0], ktile_wrapper_args_offset[0], sizeof(fpga_wrapper_KHitTile_int));
    printf("argument\n");
    PYNQ_writeToHLS(&kernel_state[rr], &matrix_data[0], ktile_data_args_offset[0], sizeof(fpga_wrapper_KHitTile_int));
    printf("argument\n");
    PYNQ_writeToHLS(&kernel_state[rr], &matrix_wrapper[1], ktile_wrapper_args_offset[1], sizeof(fpga_wrapper_KHitTile_int));
    printf("argument\n");
    PYNQ_writeToHLS(&kernel_state[rr], &matrix_data[1], ktile_data_args_offset[1], sizeof(fpga_wrapper_KHitTile_int));
    printf("argument\n");
    PYNQ_writeToHLS(&kernel_state[rr], &H, int_args_offset[0], sizeof(int));
    printf("argument\n");
    PYNQ_writeToHLS(&kernel_state[rr], &W, int_args_offset[1], sizeof(int));
    printf("argument\n");

    //print_axi_faults();
    
    // MedianBlur
    if(kernel == 0)
        PYNQ_writeToHLS(&kernel_state[rr], &iters, int_args_offset[2], sizeof(int));

    printf("Before enable\n");

    PYNQ_writeToHLS(&kernel_state[rr], &enable, 0x0, sizeof(int));
    printf("After enable\n");
    //print_axi_faults();
}

void swap(int which, int rr) {
    int err;
    printf("Before swapping\n");
    //print_axi_faults();

    int counter = 0;

    PYNQ_writeGPIO(&reset_gpio[rr], &enable);
    //for(int i = 0; i < 100000; i++) counter++;
    PYNQ_writeGPIO(&reset_gpio[rr], &disable);
    PYNQ_writeGPIO(&decouple_rst_clk_gpio[rr], &enable);
    PYNQ_writeGPIO(&decouple_gpio[rr], &enable);
    //print_axi_faults();
    switch(which) {
        case 0:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-GaussianBlur_pblock_medianblur_0_partial.bit", 1);
            printf("Before swapping\n");
            //PYNQ_loadBitstream("/home/xilinx/Bitstreams/GaussianBlur-GaussianBlur_pblock_medianblur_0_partial.bit", 1);
            break;
        case 1:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-GaussianBlur_pblock_gaussianblur_0_partial.bit", 1);
            break;
        case 2:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-GaussianBlur_pblock_medianblur_0_partial.bit", 1);
            //PYNQ_loadBitstream("/home/xilinx/Bitstreams/MedianBlur-GaussianBlur_pblock_medianblur_0_partial.bit", 1);
            break;
        case 3:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-GaussianBlur_pblock_gaussianblur_0_partial.bit", 1);
            //PYNQ_loadBitstream("/home/xilinx/Bitstreams/MedianBlur-GaussianBlur_pblock_gaussianblur_0_partial.bit", 1);
            break;
        case 4:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-MedianBlur_pblock_medianblur_0_partial.bit", 1);
            //PYNQ_loadBitstream("/home/xilinx/Bitstreams/GaussianBlur-MedianBlur_pblock_medianblur_0_partial.bit", 1);
            break;
        case 5:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-MedianBlur_pblock_gaussianblur_0_partial.bit", 1);
            break;
        case 6:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-MedianBlur_pblock_medianblur_0_partial.bit", 1);
            break;
        case 7:
            err=PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-MedianBlur_pblock_gaussianblur_0_partial.bit", 1);
            break;

    }

    printf("After swapping. err: %d\n", err);
    //print_axi_faults();
    //PYNQ_writeGPIO(&decouple_rst_clk_gpio[rr], &disable);
    counter = 0;
    PYNQ_writeGPIO(&decouple_rst_clk_gpio[rr], &disable);
    PYNQ_writeGPIO(&reset_gpio[rr], &enable);
    //for(int i = 0; i < 1000000; i++) counter++;
    //printf("After enabling reset\n");
    PYNQ_writeGPIO(&reset_gpio[rr], &disable);
    //printf("After disabling reset\n");
    //counter = 0;
    //for(int i = 0; i < 2000000; i++) counter++;
    PYNQ_writeGPIO(&decouple_gpio[rr], &disable);
    PYNQ_writeGPIO(&reset_gpio[rr], &enable);
    //for(int i = 0; i < 100000; i++) counter++;
    //printf("After enabling reset\n");
    PYNQ_writeGPIO(&reset_gpio[rr], &disable);
    //print_axi_faults();
    printf("After swapping\n");
}


void reset_interrupt_controller() {
    PYNQ_AXI_INTERRUPT_CONTROLLER * interrupt_controller = &int_controller;

    unsigned int raised_irqs, work, current_irq=0;
    int IER_val;

    // Read the interrupts
    PYNQ_readMMIO(&(interrupt_controller->mmio_window), &raised_irqs, 0x04, sizeof(unsigned int));

    // ACKNOWLEDGE INTERRUPTS
    // Acknowledge in HLS block
    int ISR_HLS = 1;

    // Which RR triggered the interrupt?
    work = raised_irqs;
    for(int _rr = 0; _rr < 2; _rr++) {
        if(work & 0x1) {
            // Reset only RRs. This does not apply to host-tasks
            PYNQ_writeToHLS(&kernel_state[_rr], &ISR_HLS, 0xC, sizeof(int));
            generated_interrupts[_rr] = 1;
        }
        work = work >> 1;
    }

    // Acknowledge in controller
    PYNQ_writeMMIO(&(interrupt_controller->mmio_window), &raised_irqs, 0x0C, sizeof(unsigned int));
}

void enable_hls_interrupts(int rr) {
    PYNQ_writeToHLS(&kernel_state[rr], &enable, GIER, sizeof(int));
    PYNQ_writeToHLS(&kernel_state[rr], &enable, IP_IER, sizeof(int));
}

void main(int argc, char * argv[]) {
    double init_time;

    int swap_or_launch = atoi(argv[1]);
    int partial = atoi(argv[3]);

    if(swap_or_launch == 0) {
        PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-GaussianBlur.bit", 0);
        setup_scheduler();

        init_time = omp_get_wtime();
        if(partial)
            swap(2,0);
        else
            PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-GaussianBlur.bit", 0);
        printf("%lf\n", omp_get_wtime() - init_time);
    }
    else if(swap_or_launch == 1) {
        int kernel = atoi(argv[2]);

        switch(kernel) {
            case 0:
                PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/GaussianBlur-GaussianBlur.bit", 0);
                setup_scheduler();
                enable_hls_interrupts(0);
                init_time = omp_get_wtime();
                launch(0, 1, 0);
                break;
            case 1:
                init_time = omp_get_wtime();
                PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-GaussianBlur.bit", 0);
                setup_scheduler();
                enable_hls_interrupts(0);
                launch(0, 0, 1);
                break;
            case 2:
                init_time = omp_get_wtime();
                PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-GaussianBlur.bit", 0);
                setup_scheduler();
                enable_hls_interrupts(0);
                launch(0, 0, 2);
                break;
            case 3:
                init_time = omp_get_wtime();
                PYNQ_loadBitstream("/home/xilinx/Bitstreams-axi_firewall/MedianBlur-GaussianBlur.bit", 0);
                setup_scheduler();
                enable_hls_interrupts(0);
                launch(0, 0, 3);
                break;
        }
        int n_interrupts;
        PYNQ_waitForUIO(&uio_state, &n_interrupts);
        printf("%lf\n", omp_get_wtime() - init_time);

        reset_interrupt_controller();
    }
}
