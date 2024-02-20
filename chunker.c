/*
 * Copyright (C) 2024 Tether Operations Limited
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Arun Mani J <arunmani@peartree.to>
 */

#define G_LOG_DOMAIN "chunker"

#include <gst/gst.h>

#include <stdlib.h>

/*
 * Chunker:
 *
 * Chunk the incoming audio stream by silence.
 */

typedef struct Context {
  GMainLoop   *loop;
  GstPipeline *pipeline;
} Context;


static void
on_decoder_pad_added (GstElement *decoder, GstPad *pad, GstElement *silence)
{
  if (!gst_element_link (decoder, silence))
    g_critical ("Unable to link decoder and removesilence");
}


static void
on_eos (GstBus *bus, GstMessage *message, Context *ctx)
{
  g_message ("EOS reached");
  gst_element_set_state (GST_ELEMENT (ctx->pipeline), GST_STATE_NULL);
  g_main_loop_quit (ctx->loop);
}


static void
on_error (GstBus *bus, GstMessage *message, Context *ctx)
{
  g_autoptr (GError) error;
  gst_message_parse_error (message, &error, NULL);
  g_critical ("%s: %s", GST_OBJECT_NAME (message->src), error->message);
  gst_element_set_state (GST_ELEMENT (ctx->pipeline), GST_STATE_NULL);
  g_main_loop_quit (ctx->loop);
}


static void
on_message (GstBus *bus, GstMessage *message, GstElement *muxsink)
{
  const GstStructure *structure;
  guint64 value;

  if (!gst_message_has_name (message, "removesilence"))
    return;

  structure = gst_message_get_structure (message);

  if (!gst_structure_get_uint64 (structure, "silence_detected", &value))
    return;

  g_message ("Silence detected");
  g_signal_emit_by_name (muxsink, "split-now", NULL);
}


static void
create_chunker_pipeline (Context *ctx, char *input, char *output, char *muxer)
{
  GstElement *filesrc, *decoder, *silence, *encoder, *muxsink;
  GstPipeline *pipeline;

  g_autoptr (GstBus) bus;

  filesrc = gst_element_factory_make_full ("filesrc",
                                           "location", input,
                                           NULL);
  decoder = gst_element_factory_make_full ("decodebin", NULL);
  silence = gst_element_factory_make_full ("removesilence",
                                           "silent", FALSE,
                                           "remove", TRUE,
                                           "minimum-silence-time", GST_SECOND,
                                           NULL);
  encoder = gst_element_factory_make_full (muxer, NULL);
  muxsink = gst_element_factory_make_full ("splitmuxsink",
                                           "location", output,
                                           "muxer", encoder,
                                           NULL);

  pipeline = GST_PIPELINE (gst_pipeline_new ("main"));

  g_return_if_fail (filesrc != NULL && decoder != NULL &&
                    silence != NULL && encoder != NULL &&
                    muxsink != NULL && pipeline != NULL);

  ctx->pipeline = pipeline;

  g_signal_connect (decoder, "pad-added", G_CALLBACK (on_decoder_pad_added), silence);

  gst_bin_add_many (GST_BIN (pipeline), filesrc, decoder, silence, muxsink, NULL);

  gst_element_link (filesrc, decoder);
  gst_element_link (silence, muxsink);

  bus = gst_pipeline_get_bus (pipeline);
  gst_bus_add_signal_watch (bus);
  g_signal_connect (bus, "message::eos", G_CALLBACK (on_eos), ctx);
  g_signal_connect (bus, "message::error", G_CALLBACK (on_error), ctx);
  g_signal_connect (bus, "message", G_CALLBACK (on_message), muxsink);
}


static char *input = NULL;
static char *output = NULL;
static char *muxer = NULL;

static GOptionEntry entries[] =
{
  { "input", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &input,
    "The input audio file to chunk like input.wav", "I" },
  { "output", 'o', G_OPTION_FLAG_NONE, G_OPTION_ARG_FILENAME, &output,
    "The output format for chunks like output/%02d.wav", "O" },
  { "muxer", 'm', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &muxer,
    "The muxer to encode chunks like wavenc", "M"},
  G_OPTION_ENTRY_NULL,
};

int
main (int argc, char *argv[])
{
  Context ctx;

  g_autoptr (GError) error;
  g_autoptr (GOptionContext) context;

  context = g_option_context_new ("Chunk the audio file by silence");
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

  ctx.loop = g_main_loop_new (NULL, FALSE);
  create_chunker_pipeline (&ctx, input, output, muxer);

  if (ctx.pipeline == NULL)
    return EXIT_FAILURE;

  g_message ("Changing state to playing");
  gst_element_set_state (GST_ELEMENT (ctx.pipeline), GST_STATE_PLAYING);
  g_main_loop_run (ctx.loop);

  g_free (ctx.loop);
  g_clear_object (&ctx.pipeline);

  return EXIT_SUCCESS;
}
