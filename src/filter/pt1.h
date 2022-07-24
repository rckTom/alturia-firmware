#ifndef ALTURIA__PT1__H
#define ALTURIA__PT1__H

struct pt1_state {
	float y_n1;
	float K;
	float T1;
};

void pt1_init(struct pt1_state *state, float initial_state, float K, float T1);
float pt1_update(struct pt1_state *state, float u, float dt);

#endif