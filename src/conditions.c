#include "conditions.h"
#include <logging/log.h>

LOG_MODULE_DECLARE(event_system);

void condition_init(struct condition *cond, condition_type_t type,
		    flight_state_var_t variable, numeric_t parameter)
{
	cond->type = type;
	cond->last_var.type = NUMERIC_TYPE_INVALID;
	cond->param = parameter,
	cond->var = variable,
	cond->next = NULL;
}

static int condition_goes_above(condition_t *cond, const numeric_t *var_val, bool *res)
{
	int rc = 0;

	if (cond->last_var.type == NUMERIC_TYPE_INVALID) {
		*res = 0;
		cond->last_var = *var_val;
		return 0;
	}

	bool resi;
	bool resi2;
	rc = numeric_gt(var_val, &cond->param, &resi);

	if (rc != 0) {
		return rc;
	}

	rc = numeric_loe(&cond->last_var, &cond->param, &resi2);
	cond->last_var = *var_val;

	if(resi && resi2) {
		*res = true;
	} else {
		*res = false;
	}

	return rc;
}

static int condition_goes_below(condition_t *cond, const numeric_t *var_val, bool *res)
{
	int rc = 0;

	if (cond->last_var.type == NUMERIC_TYPE_INVALID) {
		*res = 0;
		cond->last_var = *var_val;
		return 0;
	}

	bool resi;
	bool resi2;
	rc = numeric_lt(var_val, &cond->param, &resi);

	if (rc != 0) {
		return rc;
	}

	rc = numeric_goe(&cond->last_var, &cond->param, &resi2);
	cond->last_var = *var_val;

	if(resi && resi2) {
		*res = true;
	} else {
		*res = false;
	}

	return rc;
}

bool conditions_evaluate(condition_t *cond) {
	if (cond == NULL) {
		return true;
	}

	bool ret = true;
	int rc = 0;
	while (1) {
		bool res;
		numeric_t val = flightstate_get_var(cond->var);
		switch (cond->type) {
		case COND_EQ:
			rc = numeric_eq(&val, &cond->param, &res);
		break;
		case COND_GOE:
			rc = numeric_goe(&val, &cond->param, &res);
		break;
		case COND_GT:
			rc = numeric_gt(&val, &cond->param, &res);
		break;
		case COND_LOE:
			rc = numeric_loe(&val, &cond->param, &res);
		break;
		case COND_LT:
			rc = numeric_lt(&val, &cond->param, &res);
		break;
		case COND_GOES_BELOW:
			rc = condition_goes_below(cond, &val, &res);
		break;
		case COND_GOES_ABOVE:
			rc = condition_goes_above(cond, &val, &res);
		break;
		}

		if (rc != 0) {
			LOG_ERR("comparing numerics with differnt type");
		}

		ret &= res;

		if(!ret) {
			break;
		}

		if (cond->next == NULL) {
			break;
		}

		cond = cond->next;
	}

	return ret;
}
