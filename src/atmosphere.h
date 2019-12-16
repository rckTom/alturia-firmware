#ifndef __ALTURIA_ATMOSPHERE_H__
#define __ALTURIA_ATMOSPHERE_H__

void init_atmosphere();
float calc_height(float pressure);
float calc_pressure(float height);
float calc_temperature(float height);

#endif /* __ALTURIA_ATMOSPHERE_H__ */
