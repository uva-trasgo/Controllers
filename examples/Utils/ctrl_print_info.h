/**
 * @file ctrl_print_info.h
 * @brief Helper function to print information of all ctrl devices
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Ctrl.h"

// TODO @sergioalo should this be here or be part of ctrl library?
// TODO @sergioalo should exp mode be controlled via compilation macro or param flag?
// TODO @sergioalo consider printing host affinity

/**
 * @brief Pretty print information about ctrl devices to a string
 * @param[out] out_str Output buffer
 * @param exp_mode Flag to indicate output format
 * @return Number of characters written to buffer
 *
 * Must be called inside __ctrl_block__
 */
static inline int Ctrl_SPrintInfo(char *out_str, bool exp_mode) {

	char *p_buffer       = out_str;
	char *p_start_buffer = p_buffer;

	for (int i = 0; i < Ctrl_GetNCtrls(); i++) {
		Ctrl_Info info = Ctrl_GetInfo(Ctrl_Get(i));

		if (exp_mode) {
			if (!strcmp(info.type, "CUDA") || !strcmp(info.type, "HIP")) {
				p_buffer += sprintf(p_buffer, "%s-%s", info.type, info.device_name);
			} else if (!strcmp(info.type, "OpenCL_GPU")) {
				p_buffer += sprintf(p_buffer, "%s-%s-%s", "OpenCL-GPU", info.device_name, info.platform_name);
			} else if (!strcmp(info.type, "FPGA")) {
				p_buffer += sprintf(p_buffer, "%s-%s-%s", info.type, info.device_name, info.platform_name);
			} else if (!strcmp(info.type, "CPU")) {
				p_buffer += sprintf(p_buffer, "%s-%d-%d-%d-%s", info.type, info.n_threads, info.numa_range_min, info.numa_range_max, info.mem_transfers ? "ON" : "OFF");
			} else {
				fprintf(stderr, "[Ctrl_PrintInfo] ERROR: unknown ctrl type %s\n", info.type);
				exit(EXIT_FAILURE);
			}

			// separator between devices
			if (i < Ctrl_GetNCtrls() - 1) {
				p_buffer += sprintf(p_buffer, "#");
			}

		} else {
			p_buffer += sprintf(p_buffer, "\n CTRL TYPE: %s", info.type);
			p_buffer += sprintf(p_buffer, "\n DEVICE: %s", info.device_name);

			if (!strcmp(info.type, "CUDA") || !strcmp(info.type, "HIP")) continue;

			if (!strcmp(info.type, "OpenCL_GPU")) {
				p_buffer += sprintf(p_buffer, "\n PLATFORM: %s", info.platform_name);
			} else if (!strcmp(info.type, "FPGA")) {
				p_buffer += sprintf(p_buffer, "\n PLATFORM: %s", info.platform_name);
			} else if (!strcmp(info.type, "CPU")) {
				p_buffer += sprintf(p_buffer, "\n KERNEL THREADS: %d", info.n_threads);
				p_buffer += sprintf(p_buffer, "\n KERNEL NUMA RANGE: %d-%d", info.numa_range_min, info.numa_range_max);
				p_buffer += sprintf(p_buffer, "\n MEM_MOVES: %s", info.mem_transfers ? "ON" : "OFF");
			} else {
				fprintf(stderr, "[Ctrl_PrintInfo] ERROR: unknown ctrl type %s\n", info.type);
				exit(EXIT_FAILURE);
			}
		}
	}

	return p_buffer - p_start_buffer;
}

/**
 * @brief Pretty print information about ctrl devices
 *
 * Must be called inside __ctrl_block__
 */
static inline void Ctrl_PrintInfo() {
	char buff[1024];
	bool exp_mode;
	// Macro for backwards compatibility
	#ifdef _CTRL_EXAMPLES_EXP_MODE_
	exp_mode = true;
	#else // _CTRL_EXAMPLES_EXP_MODE_
	exp_mode = false;
	#endif // _CTRL_EXAMPLES_EXP_MODE_
	char *p_buffer = buff;
	p_buffer += Ctrl_SPrintInfo(buff, exp_mode);
	if (exp_mode)
		p_buffer += sprintf(p_buffer, ", ");
	printf("%s", buff);
}
