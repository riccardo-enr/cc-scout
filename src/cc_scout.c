#include "cc_scout.h"

#include <alsa/asoundlib.h>
#include <errno.h>
#include <stdlib.h>

struct cc_scout {
  snd_seq_t *seq;
  int port;
};

int cc_scout_open(cc_scout **scout) {
  *scout = NULL;
  cc_scout *handle = malloc(sizeof(*handle));
  if (!handle) {
    return -ENOMEM;
  }

  int result = snd_seq_open(&handle->seq, "default", SND_SEQ_OPEN_INPUT, 0);
  if (result < 0) {
    free(handle);
    return result;
  }

  handle->port = -1;
  *scout = handle;
  return 0;
}

void cc_scout_close(cc_scout *scout) {
  snd_seq_close(scout->seq);
  free(scout);
}

const char *cc_scout_error(int status) { return snd_strerror(status); }

int cc_scout_list_sources(cc_scout *scout, cc_scout_source_fn callback,
                          void *userdata) {
  snd_seq_client_info_t *client;
  snd_seq_port_info_t *port;
  snd_seq_client_info_alloca(&client);
  snd_seq_port_info_alloca(&port);

  snd_seq_client_info_set_client(client, -1);
  int result;
  while ((result = snd_seq_query_next_client(scout->seq, client)) >= 0) {
    int id = snd_seq_client_info_get_client(client);
    if (id == SND_SEQ_CLIENT_SYSTEM) {
      continue;
    }

    snd_seq_port_info_set_client(port, id);
    snd_seq_port_info_set_port(port, -1);
    while ((result = snd_seq_query_next_port(scout->seq, port)) >= 0) {
      unsigned int required =
          SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
      if ((snd_seq_port_info_get_capability(port) & required) != required) {
        continue;
      }

      cc_scout_address address = {id, snd_seq_port_info_get_port(port)};
      callback(address, snd_seq_client_info_get_name(client),
               snd_seq_port_info_get_name(port), userdata);
    }
    if (result != -ENOENT) {
      return result;
    }
  }
  return result == -ENOENT ? 0 : result;
}

int cc_scout_prepare_monitor(cc_scout *scout) {
  if (scout->port >= 0) {
    return 0;
  }

  int port = snd_seq_create_simple_port(
      scout->seq, "Monitor",
      SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
      SND_SEQ_PORT_TYPE_APPLICATION);
  if (port < 0) {
    return port;
  }
  scout->port = port;
  return 0;
}

int cc_scout_subscribe(cc_scout *scout, cc_scout_address source) {
  if (scout->port < 0) {
    return -EINVAL;
  }
  return snd_seq_connect_from(scout->seq, scout->port, source.client,
                              source.port);
}

int cc_scout_set_nonblocking(cc_scout *scout) {
  return snd_seq_nonblock(scout->seq, 1);
}

int cc_scout_read_event(cc_scout *scout, cc_scout_event *event) {
  if (scout->port < 0) {
    return -EINVAL;
  }

  for (;;) {
    snd_seq_event_t *input;
    int result = snd_seq_event_input(scout->seq, &input);
    if (result < 0) {
      return result;
    }

    cc_scout_address source = {input->source.client, input->source.port};
    switch (input->type) {
      case SND_SEQ_EVENT_NOTEON:
      case SND_SEQ_EVENT_NOTEOFF: {
        cc_scout_event_type type =
            input->type == SND_SEQ_EVENT_NOTEON && input->data.note.velocity
                ? CC_SCOUT_NOTE_ON
                : CC_SCOUT_NOTE_OFF;
        *event =
            (cc_scout_event){source, type, input->data.note.channel + 1,
                             input->data.note.note, input->data.note.velocity};
        return 0;
      }
      case SND_SEQ_EVENT_CONTROLLER:
        *event = (cc_scout_event){
            source, CC_SCOUT_CONTROL_CHANGE, input->data.control.channel + 1,
            input->data.control.param, input->data.control.value};
        return 0;
    }
  }
}
