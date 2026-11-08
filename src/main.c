#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void list_sources(snd_seq_t *seq)
{
    snd_seq_client_info_t *client;
    snd_seq_port_info_t *port;
    snd_seq_client_info_alloca(&client);
    snd_seq_port_info_alloca(&port);

    snd_seq_client_info_set_client(client, -1);
    while (snd_seq_query_next_client(seq, client) >= 0) {
        int id = snd_seq_client_info_get_client(client);
        if (id == SND_SEQ_CLIENT_SYSTEM)
            continue;

        snd_seq_port_info_set_client(port, id);
        snd_seq_port_info_set_port(port, -1);
        while (snd_seq_query_next_port(seq, port) >= 0) {
            unsigned int required = SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ;
            if ((snd_seq_port_info_get_capability(port) & required) != required)
                continue;

            printf("%d:%d  %s / %s\n", id, snd_seq_port_info_get_port(port),
                   snd_seq_client_info_get_name(client), snd_seq_port_info_get_name(port));
        }
    }
}

static void print_event(const snd_seq_event_t *event)
{
    unsigned int client = event->source.client;
    unsigned int port = event->source.port;
    switch (event->type) {
    case SND_SEQ_EVENT_NOTEON:
    case SND_SEQ_EVENT_NOTEOFF:
        printf("%u:%u %s ch=%d note=%u velocity=%u\n", client, port,
               event->type == SND_SEQ_EVENT_NOTEON && event->data.note.velocity
                   ? "note-on" : "note-off",
               event->data.note.channel + 1, (unsigned int)event->data.note.note,
               (unsigned int)event->data.note.velocity);
        break;
    case SND_SEQ_EVENT_CONTROLLER:
        printf("%u:%u cc ch=%d controller=%u value=%d\n", client, port,
               event->data.control.channel + 1, event->data.control.param,
               event->data.control.value);
        break;
    }
}

static int parse_address(const char *text, snd_seq_addr_t *address)
{
    char *end;
    if (*text < '0' || *text > '9')
        return -1;
    unsigned long client = strtoul(text, &end, 10);
    if (*end != ':' || client > 255)
        return -1;

    text = end + 1;
    if (*text < '0' || *text > '9')
        return -1;
    unsigned long port = strtoul(text, &end, 10);
    if (*end != '\0' || port > 255)
        return -1;

    address->client = (unsigned char)client;
    address->port = (unsigned char)port;
    return 0;
}

static int monitor(snd_seq_t *seq, int count, char **addresses)
{
    int port = snd_seq_create_simple_port(seq, "Monitor",
                                         SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_NO_EXPORT,
                                         SND_SEQ_PORT_TYPE_APPLICATION);
    if (port < 0) {
        fprintf(stderr, "Cannot create monitor port: %s\n", snd_strerror(port));
        return port;
    }

    for (int i = 0; i < count; ++i) {
        snd_seq_addr_t source;
        if (parse_address(addresses[i], &source) < 0)
            return -EINVAL;

        int result = snd_seq_connect_from(seq, port, source.client, source.port);
        if (result < 0) {
            fprintf(stderr, "Cannot subscribe to %s: %s\n",
                    addresses[i], snd_strerror(result));
            return result;
        }
    }

    fputs("Monitoring", stderr);
    for (int i = 0; i < count; ++i)
        fprintf(stderr, " %s", addresses[i]);
    fputs(" (Ctrl+C to stop)\n", stderr);
    setvbuf(stdout, NULL, _IOLBF, 0);
    for (;;) {
        snd_seq_event_t *event;
        int result = snd_seq_event_input(seq, &event);
        if (result < 0) {
            fprintf(stderr, "Cannot read MIDI events: %s\n", snd_strerror(result));
            return result;
        }
        print_event(event);
    }
}

static void usage(FILE *stream)
{
    fputs("Usage: cc-scout list | monitor CLIENT:PORT [CLIENT:PORT ...] | --help\n", stream);
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        usage(stdout);
        return EXIT_SUCCESS;
    }

    int listing = argc == 2 && strcmp(argv[1], "list") == 0;
    if (!listing && !(argc >= 3 && strcmp(argv[1], "monitor") == 0)) {
        usage(stderr);
        return 2;
    }
    for (int i = 2; i < argc; ++i) {
        snd_seq_addr_t source;
        if (parse_address(argv[i], &source) < 0) {
            fprintf(stderr, "Invalid MIDI port: %s\n", argv[i]);
            return 2;
        }
    }

    snd_seq_t *seq;
    int result = snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0);
    if (result < 0) {
        fprintf(stderr, "Cannot open ALSA sequencer: %s\n", snd_strerror(result));
        return EXIT_FAILURE;
    }

    if (listing) {
        list_sources(seq);
    } else {
        result = monitor(seq, argc - 2, argv + 2);
    }

    snd_seq_close(seq);
    return result < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
