#include "cc_scout.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_source(cc_scout_address address, const char *client_name,
                         const char *port_name, void *userdata)
{
    (void)userdata;
    printf("%u:%u  %s / %s\n", (unsigned int)address.client,
           (unsigned int)address.port, client_name, port_name);
}

static void print_event(const cc_scout_event *event)
{
    unsigned int client = event->source.client;
    unsigned int port = event->source.port;
    switch (event->type) {
    case CC_SCOUT_NOTE_ON:
    case CC_SCOUT_NOTE_OFF:
        printf("%u:%u %s ch=%u note=%u velocity=%d\n", client, port,
               event->type == CC_SCOUT_NOTE_ON ? "note-on" : "note-off",
               event->channel, event->number, event->value);
        break;
    case CC_SCOUT_CONTROL_CHANGE:
        printf("%u:%u cc ch=%u controller=%u value=%d\n", client, port,
               event->channel, event->number, event->value);
        break;
    }
}

static int parse_address(const char *text, cc_scout_address *address)
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

static int monitor(cc_scout *scout, int count, char **addresses)
{
    int result = cc_scout_prepare_monitor(scout);
    if (result < 0) {
        fprintf(stderr, "Cannot create monitor port: %s\n", cc_scout_error(result));
        return result;
    }

    for (int i = 0; i < count; ++i) {
        cc_scout_address source;
        if (parse_address(addresses[i], &source) < 0)
            return -EINVAL;
        result = cc_scout_subscribe(scout, source);
        if (result < 0) {
            fprintf(stderr, "Cannot subscribe to %s: %s\n",
                    addresses[i], cc_scout_error(result));
            return result;
        }
    }

    fputs("Monitoring", stderr);
    for (int i = 0; i < count; ++i)
        fprintf(stderr, " %s", addresses[i]);
    fputs(" (Ctrl+C to stop)\n", stderr);
    setvbuf(stdout, NULL, _IOLBF, 0);
    for (;;) {
        cc_scout_event event;
        result = cc_scout_read_event(scout, &event);
        if (result < 0) {
            fprintf(stderr, "Cannot read MIDI events: %s\n", cc_scout_error(result));
            return result;
        }
        print_event(&event);
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
        cc_scout_address source;
        if (parse_address(argv[i], &source) < 0) {
            fprintf(stderr, "Invalid MIDI port: %s\n", argv[i]);
            return 2;
        }
    }

    cc_scout *scout;
    int result = cc_scout_open(&scout);
    if (result < 0) {
        fprintf(stderr, "Cannot open ALSA sequencer: %s\n", cc_scout_error(result));
        return EXIT_FAILURE;
    }

    if (listing) {
        result = cc_scout_list_sources(scout, print_source, NULL);
        if (result < 0)
            fprintf(stderr, "Cannot list MIDI sources: %s\n", cc_scout_error(result));
    } else {
        result = monitor(scout, argc - 2, argv + 2);
    }

    cc_scout_close(scout);
    return result < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
