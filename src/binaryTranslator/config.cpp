/*******************************************************************************
 *  config.cpp
 ******************************************************************************/

#include "config.h"

void parse_config_file() {
	FILE * input_config;
	if ((input_config = fopen("config/param.cfg", "r")) == NULL) {
		Assert("Cannot open config file.");
	}
	
	//
	char param[CFG_STRING_SIZE], value[CFG_STRING_SIZE];
	if (fscanf(input_config, "%s %s\n", &param, &value) == EOF) Assert("Corrupted config file.");
	const char* input_asm_file = param;
	printf("\t%s %s\n", param, value);
	if (value[0] == 'y')

	fclose(input_config);
}