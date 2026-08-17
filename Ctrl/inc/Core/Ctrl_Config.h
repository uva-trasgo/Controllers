#ifndef _CTRL_CORE_CONFIG_H_
#define _CTRL_CORE_CONFIG_H_
///@cond INTERNAL
/**
 * @file Ctrl_Config.h
 * @brief Struct for the configuration of a Ctrl.
 *
 * @copyright This software is part of the Controller project by Trasgo Group, UVa.
 * The relevant license, warranty and copyright notice is available in the Controller project repository.
 */

#include "Core/Ctrl_Type.h"

/**
 * @brief Key value pair
 *
 * Used for ctrl device selection files device arguments
 */
typedef struct Ctrl_Config_KeyVal {
	char *key; /**< Name of the argument */
	char *val; /**< Value of the argument */
} Ctrl_Config_KeyVal;

/**
 * Max number of allowed arguments for a device on a device selection file
 */
#define CTRL_CONFIG_MAX_ARGS 8

/**
 * @brief Config description for a ctrl device.
 *
 * The allowed/required arguments depend on the \p type of the ctrl and
 * are validated by the relevant backend ctrl creation function.
 */
typedef struct Ctrl_Config_Dev {
	Ctrl_Type          type;                       /**< Type of the device */
	Ctrl_Config_KeyVal args[CTRL_CONFIG_MAX_ARGS]; /**< Arguments of the device */
} Ctrl_Config_Dev;

/**
 * Controllers config description for a process.
 */
typedef struct Ctrl_Config {
	Ctrl_Config_Dev *p_devs;         /**< List of devices for this rank */
	int              ndevs;          /**< Number of entries in p_devs */
	int              host_affinity;  /**< NUMA affinity for the host on this process */
	float            weight;         /**< Computational weight of the current process. For weighted work distributions. */
	int              host_alignment; /**< Host memory alignment in bytes. */
} Ctrl_Config;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Parse device selection configuration file and create all ctrls specified in it.
 *
 * @param file path to config file.
 * @note This function can only be called after Ctrl_Init and before Ctrl_Finalize
 */
void Ctrl_ParseConfig(const char *file);

/**
 * @brief Get the value of an argument from \p dev by \p key
 *
 * @param dev Device config to query
 * @param key Key to find
 * @param default_val default value if \p key is not found.
 * If \e NULL is passed the \p key is assumed mandatory and an error is thrown if \p key is missing.
 * @return Value of arg \p key, if not found return \p default_val.
 */
char *Ctrl_Config_GetVal(Ctrl_Config_Dev dev, const char *key, char *default_val);

#ifdef __cplusplus
}
#endif

///@endcond
#endif // _CTRL_CORE_CONFIG_H_
