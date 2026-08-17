///@cond INTERNAL
#include "Core/Ctrl_Config.h"
#include "Core/Ctrl_Core.h"
#include "hitmap2.h"

/**
 * Helper function to read a file to a string.
 *
 * @param file path to file
 * @return char* string with the contents of \p file . Must be freed after use.
 */
char *Ctrl_ReadFile(const char *file) {
	FILE *f = fopen(file, "r");

	if (!f) {
		fprintf(stderr, "Error: File for devices selection could not be opened: %s\n", file);
		exit(EXIT_FAILURE);
	}

	fseek(f, 0, SEEK_END);
	size_t length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *buffer = (char *)malloc(length + 1);

	size_t read_length = fread(buffer, 1, length, f);
	if (length != read_length) {
		free(buffer);
		fprintf(stderr, "Error: Reading devices selection file: %s\n", file);
		exit(EXIT_FAILURE);
	}
	fclose(f);

	buffer[length] = '\0';
	return buffer;
}

/**
 * @brief Parse a keyval argument list using \e strtok
 *
 * This function parses a single keyval argument and advances the internal \e strtok
 * buffer to the next argument's position.
 *
 * @note Usual \e strtok semantics apply regarding internal state, NULL usage and \p conf_line modification.
 *
 * @param conf_line String of the form \e key=val .
 * Use NULL to keep parsing the same string similar to \e strtok
 * @return keyval structure
 */
Ctrl_Config_KeyVal Ctrl_ParseKeyval(char *conf_line) {
	char *tok = strtok(conf_line, " =\t,\n");
	return (Ctrl_Config_KeyVal){.key = tok, .val = strtok(NULL, " =\t,;\n")};
}

/**
 * @param str String
 * @param c Character to count
 * @return Times \p c appears in \p str
 */
int Ctrl_CountChar(const char *str, char c) {
	int count = 0;
	for (int i = 0; str[i]; i++) {
		count += (str[i] == c);
	}
	return count;
}

/**
 * @brief Parse a device line from device selection config file
 *
 * @param conf_line Device config line
 * @return Parsed device information
 */
Ctrl_Config_Dev Ctrl_ParseDev(char *conf_line) {
	const char *pp_valid_keys[][CTRL_CONFIG_MAX_ARGS] = {
		{},                                             // NULL
		{"threads", "numa_range", "memmoves", "align"}, // CPU
		{"dev", "kstreams", "align"},                   // CUDA
		{"dev", "kstreams", "align"},                   // HIP
		{"platform", "dev", "kstreams", "align"},       // OPENCL_GPU
		{"platform", "dev", "kstreams", "align"},       // FPGA
	};

	char           *tok = strtok(conf_line, " \t");
	Ctrl_Config_Dev dev = {};
	if (tok == NULL)
		return dev;

	if (!strcmp(tok, "cpu")) {
		dev = (Ctrl_Config_Dev){.type = CTRL_TYPE_CPU};
	} else if (!strcmp(tok, "cuda")) {
		dev = (Ctrl_Config_Dev){.type = CTRL_TYPE_CUDA};
	} else if (!strcmp(tok, "hip")) {
		dev = (Ctrl_Config_Dev){.type = CTRL_TYPE_HIP};
	} else if (!strcmp(tok, "opencl")) {
		dev = (Ctrl_Config_Dev){.type = CTRL_TYPE_OPENCL_GPU};
	} else if (!strcmp(tok, "fpga")) {
		dev = (Ctrl_Config_Dev){.type = CTRL_TYPE_FPGA};
	} else {
		fprintf(stderr, "[Ctrl_ParseDev] Error: Unknown device type in device selection file (proc %d), %s\n", hit_Rank, conf_line);
		exit(EXIT_FAILURE);
	}
	int i = 0;
	for (Ctrl_Config_KeyVal kv = Ctrl_ParseKeyval(NULL); kv.key != NULL; kv = Ctrl_ParseKeyval(NULL)) {
		if (i >= CTRL_CONFIG_MAX_ARGS) {
			fprintf(stderr, "[Ctrl_ParseDev] Error: Too many arguments in device config line.(%s)\n", conf_line);
			fprintf(stderr, "Max args is set to: CTRL_CONFIG_MAX_ARGS %d in Ctrl/inc/Core/Ctrl_Config.h\n", CTRL_CONFIG_MAX_ARGS);
			fflush(stderr);
		}
		bool found = false;
		for (int j = 0; j < CTRL_CONFIG_MAX_ARGS; j++) {
			if (pp_valid_keys[dev.type][j] != NULL && !strcmp(kv.key, pp_valid_keys[dev.type][j])) {
				found = true;
				break;
			}
		}
		if (!found) {
			fprintf(stderr, "[Ctrl_ParseConfig] Warning: arg %s is invalid for ctrls of type %d. Ignoring.\n", kv.key, dev.type);
			fflush(stderr);
		}

		dev.args[i++] = kv;
	}
	return dev;
}

/**
 * @brief Convert a string into a list of lines
 *
 * @param str string to split
 * @param arr[out] array of lines
 */
void Ctrl_Str2Arr(char *str, char *arr[]) {
	for (char *tok = strtok(str, "\n"); tok != NULL; tok = strtok(NULL, "\n")) {
		*(arr++) = tok;
	}
}

/**
 * @brief Find the current node section in \p buffer
 *
 * @param buffer String containing a Ctrl device selection config
 * @return Location of the current node's section in \p buffer
 */
char *Ctrl_FindNode(char *buffer) {
	char *hostname = hit_comNodeName();
	char  node_name[MPI_MAX_PROCESSOR_NAME + 6]; // extra space for the "NODE " string
	sprintf(node_name, "node %s", hostname);

	char *node_data = strstr(buffer, node_name);
	if (node_data == NULL) {
		// Fallback: try to locate wildcard node section
		sprintf(node_name, "node *");
		node_data = strstr(buffer, node_name);

		if (node_data == NULL) {
			fprintf(stderr, "[Ctrl_ParseConfig] Error: No NODE section in device selection file for node %s\n", hostname);
			exit(EXIT_FAILURE);
		}
	}
	// Locate end of this machine name section and clean the rest of the buffer
	char *end_node_data = strstr(node_data + 4, "node ");
	if (end_node_data != NULL) *end_node_data = '\0';

	return node_data;
}

/**
 * @brief Find the current node section in \p buffer
 *
 * @param buffer String containing a node section of a Ctrl device selection config
 * @return Location of the current rank's section in \p buffer
 */
char *Ctrl_FindRank(char *buffer) {
	// Find rank in the current node
	int rank_in_node = hit_comNodeGroupRank();

	// Assume optional arguments for this rank
	char proc_str[10];
	sprintf(proc_str, "proc %d ", rank_in_node);
	char *rank_data = strstr(buffer, proc_str);

	// Assume no optional arguments for this rank
	char proc_str_noopt[10];
	sprintf(proc_str_noopt, "proc %d\n", rank_in_node);
	char *rank_data_noaff = strstr(buffer, proc_str_noopt);

	// Check errors
	if (rank_data == NULL && rank_data_noaff == NULL) {
		fprintf(stderr, "[Ctrl_ParseConfig] Error: No config found in device selection file for rank %d of node %s\n", rank_in_node, hit_comNodeName());
		exit(EXIT_FAILURE);
	} else if (rank_data != NULL && rank_data_noaff != NULL) {
		fprintf(stderr, "[Ctrl_ParseConfig] Error: Multiple configs found in device selection file for rank %d of node %s\n", rank_in_node, hit_comNodeName());
		exit(EXIT_FAILURE);
	}

	rank_data = rank_data == NULL ? rank_data_noaff : rank_data;

	// Locate end of this rank section and clean the rest of the buffer
	char *end_rank_data = strstr(rank_data + 4, "proc ");
	if (end_rank_data != NULL) *end_rank_data = '\0';

	return rank_data;
}

void Ctrl_ParseConfig(const char *file) {
	// Read config file in a string buffer
	if (file == NULL) {
		if ((file = getenv("CTRL_CFG_PATH")) == NULL) {
			fprintf(stderr, "[Ctrl_ParseConfig] Error: Ctrl configuration not found."
							"Use the __ctrl_block__ argument or the environment variable CTRL_CFG_PATH.\n");
			exit(EXIT_FAILURE);
		}
	}
	char *buffer = Ctrl_ReadFile(file);

	// Locate relevant section
	char *node_data = Ctrl_FindNode(buffer);
	char *rank_data = Ctrl_FindRank(node_data);

	// 1st line for proc + one per device
	// +1 to account for the config ending on EOF
	int n_rank_lines = Ctrl_CountChar(rank_data, '\n') + 1;
	if (n_rank_lines <= 1) {
		fprintf(stderr, "[Ctrl_ParseConfig] Error: No devices, premature end of section in file %s, node %s, rank %d\n", file, hit_comNodeName(), hit_comNodeGroupRank());
		exit(EXIT_FAILURE);
	}
	char **rank_lines = (char **)calloc(n_rank_lines, sizeof(char *));
	Ctrl_Str2Arr(rank_data, rank_lines);

	// Initialize cfg structure
	Ctrl_Config cfg = {
		.p_devs         = (Ctrl_Config_Dev *)calloc(n_rank_lines - 1, sizeof(Ctrl_Config_Dev)),
		.ndevs          = n_rank_lines - 1,
		.host_affinity  = 0,
		.weight         = 1.0f,
		.host_alignment = 0,
	};

	// Parse proc line
	// skip "proc <rank>" on proc line
	strtok(rank_lines[0] + 4, " \t\n");
	for (Ctrl_Config_KeyVal kv = Ctrl_ParseKeyval(NULL); kv.key != NULL; kv = Ctrl_ParseKeyval(NULL)) {
		if (!strcmp(kv.key, "weight")) {
			cfg.weight = atof(kv.val);
		} else if (!strcmp(kv.key, "numa")) {
			cfg.host_affinity = atoi(kv.val);
		} else if (!strcmp(kv.key, "align")) {
			cfg.host_affinity = atoi(kv.val);
		}
	}

	// Parse device lines
	for (int i = 0; i < cfg.ndevs; i++) {
		cfg.p_devs[i] = Ctrl_ParseDev(rank_lines[i + 1]);
	}

	#ifdef _CTRL_DEBUG_
	printf("[%d] Affinity %d weight %f\n", hit_Rank, cfg.host_affinity, cfg.weight);
	fflush(stdout);

	for (int i = 0; i < cfg.ndevs; i++) {
		if (cfg.p_devs[i].type == CTRL_TYPE_NULL)
			continue;

		printf("[%d] Type: %d\n", hit_Rank, cfg.p_devs[i].type);
		fflush(stdout);
		for (int j = 0; j < CTRL_CONFIG_MAX_ARGS && cfg.p_devs[i].args[j].key != NULL; j++) {
			printf("[%d] Arg %d: key %s, val %s\n", hit_Rank, j, cfg.p_devs[i].args[j].key, cfg.p_devs[i].args[j].val);
			fflush(stdout);
		}
	}
	#endif // _CTRL_DEBUG_

	Ctrl_InitCore(cfg);

	// Free temp data structures
	free(cfg.p_devs);
	free(rank_lines);
	free(buffer);
}

char *Ctrl_Config_GetVal(Ctrl_Config_Dev dev, const char *key, char *default_val) {
	for (int i = 0; i < CTRL_CONFIG_MAX_ARGS; i++) {
		if (dev.args[i].key != NULL && !strcmp(dev.args[i].key, key))
			return dev.args[i].val;
	}
	if (default_val == NULL) {
		fprintf(stderr, "[Ctrl_Config] Error: Mandatory argument %s missing. See DEVICE_SELECTION.md for details on the required arguments for each kind of device.\n", key);
		exit(EXIT_FAILURE);
	}
	return default_val;
}

///@endcond
