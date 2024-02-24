#include "chunker.h"

#include "gst/gst.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *input = MARS_CHUNKER_INPUT_MIC;
static char *output = NULL;
static char *muxer = NULL;

static GOptionEntry entries[] =
{
  { "input", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &input,
    "The input audio file to chunk like \"input.wav\"; use \"mic\" to listen from mic (default)",
    "I" },
  { "output", 'o', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &output,
    "The output format for chunks like \"output/%02d.wav\"", "O" },
  { "muxer", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &muxer,
    "The muxer to encode chunks like \"wavenc\"", "M"},
  G_OPTION_ENTRY_NULL,
};


int
main (int argc, char **argv)
{
  g_autoptr (GError) error = NULL;
  g_autoptr (GOptionContext) context;
  g_autoptr (MarsChunker) chunker = NULL;
  char key = 0;

  context = g_option_context_new ("Chunk the audio stream by silence");
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print ("Error: %s\n", error->message);
    return EXIT_FAILURE;
  }

  if (input == NULL || output == NULL || muxer == NULL) {
    g_print ("Error: Input, output and muxer must be provided\n");
    return EXIT_FAILURE;
  }

  gst_init (&argc, &argv);

  chunker = mars_chunker_new (input, output, muxer);
  mars_chunker_play (chunker);

  if (g_strcmp0 (input, MARS_CHUNKER_INPUT_MIC) == 0) {
    printf ("Listening from microphone; enter any key to quit: ");
    scanf ("%c", &key);
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
