#include "chunker.h"
#include "callback-sink.h"

#include <gst/gst.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *input = MARS_CHUNKER_INPUT_MIC;
static char *muxer = NULL;

static GOptionEntry entries[] =
{
  { "input", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &input,
    "The input audio file to chunk like \"input.wav\"; use \"mic\" to listen from mic (default)",
    "I" },
  { "muxer", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &muxer,
    "The muxer to encode chunks like \"wavenc\"", "M"},
  G_OPTION_ENTRY_NULL,
};


static void
on_buffer_list_cb (GPtrArray *buffers, gpointer user_data)
{
  printf ("Got buffers: %d\n", buffers->len);
  fflush (stdout);
}


int
main (int argc, char **argv)
{
  g_autoptr (GError) error = NULL;
  g_autoptr (GOptionContext) context;
  g_autoptr (MarsChunker) chunker = NULL;
  GstElement *sink;
  char key = 0;

  context = g_option_context_new ("Chunk the audio stream by silence");
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print ("Error: %s\n", error->message);
    return EXIT_FAILURE;
  }

  if (input == NULL || muxer == NULL) {
    g_print ("Error: Input, output and muxer must be provided\n");
    return EXIT_FAILURE;
  }

  gst_init (&argc, &argv);

  sink = mars_callback_sink_new ();
  mars_callback_sink_set_buffer_list_callback (MARS_CALLBACK_SINK (sink),
                                               on_buffer_list_cb, NULL, NULL);

  chunker = g_object_new (MARS_TYPE_CHUNKER,
                          "input", input,
                          "sink", sink,
                          "muxer", muxer,
                          "rate", 8000,
                          "maximum-chunk-time", 2 * GST_SECOND,
                          NULL);

  mars_chunker_play (chunker);

  if (g_strcmp0 (input, MARS_CHUNKER_INPUT_MIC) == 0) {
    printf ("Listening from microphone; enter any key to quit: ");
    scanf ("%c", &key);
    mars_chunker_stop (chunker);
  } else {
    printf ("Waiting for %s to be chunked…\n", input);
    fflush (stdout);
    /* Wait for 1s so GStreamer can update its states. */
    sleep (1);
    while (mars_chunker_is_playing (chunker)) {
    }
  }

  return EXIT_SUCCESS;
}
