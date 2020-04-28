#ifndef ALTURIA_CONDITIONS_H
#define ALTURIA_CONDITIONS_H

#include <zephyr.h>
#include <stdbool.h>
#include "flightstate.h"
#include "numeric.h"

typedef enum condition_type {
	COND_EQ,
	COND_GOE,
	COND_GT,
	COND_LOE,
	COND_LT,
	COND_GOES_ABOVE,
	COND_GOES_BELOW,
} condition_type_t;

typedef struct condition {
	condition_type_t type;
	flight_state_var_t var;
	numeric_t last_var;
	numeric_t param;
	struct condition *next;
} condition_t;

bool conditions_evaluate(struct condition *cond);
bool condition_validate(struct condition *cond);
void condition_init(struct condition *cond, condition_type_t type,
		    flight_state_var_t variable, numeric_t parameter);

#endif
