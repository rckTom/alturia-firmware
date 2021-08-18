#ifndef ALTURIA__SIGNAL_PROCESSING__H
#define ALTURIA__SIGNAL_PROCESSING__H

enum signal_processing_state {
    SIGNAL_PROCESSING_STARTING,
    SIGNAL_PROCESSING_ACTIVE,
    SIGNAL_PROCESSING_INACTIVE,
};

enum signal_processing_state signal_processing_get_state();
void signal_processing_init(void);
int signal_processing_main(void);

#endif