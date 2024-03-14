#include "chunker.h"

#include <gst/gst.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char *output = NULL;
static char *muxer = NULL;

static GOptionEntry entries[] =
{
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
  GstElement *audiotestsrc;
  g_autoptr (MarsChunker) chunker = NULL;
  char key = 0;

  context = g_option_context_new ("Chunk the audio stream by silence");
  g_option_context_add_main_entries (context, entries, NULL);

  if (!g_option_context_parse (context, &argc, &argv, &error)) {
    g_print ("Error: %s\n", error->message);
    return EXIT_FAILURE;
  }

  if (output == NULL || muxer == NULL) {
    g_print ("Error: Output and muxer must be provided\n");
    return EXIT_FAILURE;
  }

  gst_init (&argc, &argv);

  audiotestsrc = gst_element_factory_make_full ("audiotestsrc",
                                                "wave", 6,
                                                NULL);
  chunker = g_object_new (MARS_TYPE_CHUNKER,
                          "src", audiotestsrc,
                          "output", output,
                          "muxer", muxer,
                          "rate", 8000,
                          "maximum-chunk-time", 2 * GST_SECOND,
                          NULL);
  mars_chunker_play (chunker);

  printf ("Enter any key to quit: ");
  scanf ("%c", &key);
  mars_chunker_stop (chunker);

  return EXIT_SUCCESS;
}
