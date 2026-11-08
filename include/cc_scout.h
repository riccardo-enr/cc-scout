#ifndef CC_SCOUT_H
#define CC_SCOUT_H

#include <poll.h>

typedef struct cc_scout cc_scout;

typedef struct {
  unsigned char client;
  unsigned char port;
} cc_scout_address;

typedef enum {
  CC_SCOUT_NOTE_ON,
  CC_SCOUT_NOTE_OFF,
  CC_SCOUT_CONTROL_CHANGE
} cc_scout_event_type;

typedef struct {
  cc_scout_address source;
  cc_scout_event_type type;
  unsigned int channel;
  unsigned int number;
  int value;
} cc_scout_event;

typedef void (*cc_scout_source_fn)(cc_scout_address address,
                                   const char *client_name,
                                   const char *port_name, void *userdata);

int cc_scout_open(cc_scout **scout);
void cc_scout_close(cc_scout *scout);
const char *cc_scout_error(int status);

int cc_scout_list_sources(cc_scout *scout, cc_scout_source_fn callback,
                          void *userdata);
int cc_scout_prepare_monitor(cc_scout *scout);
int cc_scout_subscribe(cc_scout *scout, cc_scout_address source);
int cc_scout_set_nonblocking(cc_scout *scout);
int cc_scout_poll_descriptor_count(cc_scout *scout);
int cc_scout_poll_descriptors(cc_scout *scout, struct pollfd *fds,
                              unsigned int count);
int cc_scout_poll_revents(cc_scout *scout, struct pollfd *fds,
                          unsigned int count);
int cc_scout_read_event(cc_scout *scout, cc_scout_event *event);

#endif
