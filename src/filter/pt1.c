#include "pt1.h"

void pt1_init(struct pt1_state *state, float initial_state, float K, float T1)
{
	state->y_n1 = initial_state;
	state->K = K;
	state->T1 = T1;
}

float pt1_update(struct pt1_state *state, float u, float dt)
{
	float result;

	result =
	    state->y_n1 + (state->K * u - state->y_n1) * dt / (state->T1 + dt);
	state->y_n1 = result;
	return result;
}