#include "flightstate.h"
#include "edge_detector.h"

void flightstate_set_burnout_count(int val)
{
	burnout_count = val;
}

int flightstate_get_burnout_count()
{
	return burnout_count;
}

void flightstate_set_mission() {
	mission_start_time = k_uptime_get();
}

void flightstate_update() {
	//statemachine tracking flightstate
	

	if( inflight) {
		//check triggers
	}


	if (flightstate == STATE_COAST) {
		if(edge_detector_update(ingnition_detector, k_uptime_get()) {
			//emit ignition signal
			//change flightstate to boost
			//increate ignition count
		}
	}

	if (flightstate == STATE_BOOST) {
		if(edge_detector_update(coast_detector, time)) {
			//emit burnout signal
			//increate burnout count
			//set flightstate to coast
		}
	}

	if (flightstate == COAST) {
		//check for apogee
		//if apogee set to descent
	}

	if (flightstate == DESCENT) {
		check for landing

	}
}
