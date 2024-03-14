/*
 * Copyright (C) 2024 Tether Operations Limited
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Arun Mani J <arunmani@peartree.to>
 */

#define G_LOG_DOMAIN "mars-chunker"

#include "chunker.h"

#include <gst/gst.h>

/**
 * MarsChunker:
 *
 * Chunk the incoming audio stream by silence.
 *
 * Use `mic` to read from microphone. [signal@Mars.Chunker::chunked] can be used
 * to signal chunking. [property@Mars.Chunker:playing] can be used to know if
 * the processing has finished for audio streams from files.
 */

enum {
  CHUNKED,
  N_SIGNALS,
};
static guint signals[N_SIGNALS];

enum {
  PROP_0,
  PROP_INPUT,
  PROP_OUTPUT,
  PROP_SINK,
  PROP_MUXER,
  PROP_RATE,
  PROP_MAXIMUM_CHUNK_TIME,
  PROP_MINIMUM_SILENCE_TIME,
  PROP_SILENCE_HYSTERESIS,
  PROP_SILENCE_THRESHOLD,
  PROP_PLAYING,
  PROP_LAST_PROP,
};
static GParamSpec *props[PROP_LAST_PROP];

struct _MarsChunker {
  GObject     parent;

  char       *input;
  char       *output;
  GstElement *sink;
  char       *muxer;
  gint        rate;
  guint64     hysteresis;
  guint64     max_chunk_time;
  guint64     min_silence_time;
  gint        threshold;
  gboolean    playing;

  GstElement *muxsink;
  GstElement *pipeline;
};

G_DEFINE_TYPE (MarsChunker, mars_chunker, G_TYPE_OBJECT)


static void
mars_chunker_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  MarsChunker *self = MARS_CHUNKER (object);

  switch (property_id) {
  case PROP_INPUT:
    self->input = g_value_dup_string (value);
    break;
  case PROP_OUTPUT:
    self->output = g_value_dup_string (value);
    break;
  case PROP_SINK:
    self->sink = g_value_get_object (value);
    break;
  case PROP_MUXER:
    self->muxer = g_value_dup_string (value);
    break;
  case PROP_RATE:
    self->rate = g_value_get_int (value);
    break;
  case PROP_MAXIMUM_CHUNK_TIME:
    self->max_chunk_time = g_value_get_uint64 (value);
    break;
  case PROP_MINIMUM_SILENCE_TIME:
    self->min_silence_time = g_value_get_uint64 (value);
    break;
  case PROP_SILENCE_HYSTERESIS:
    self->hysteresis = g_value_get_uint64 (value);
    break;
  case PROP_SILENCE_THRESHOLD:
    self->threshold = g_value_get_int (value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


static void
mars_chunker_get_property (GObject    *object,
                           guint       property_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  MarsChunker *self = MARS_CHUNKER (object);

  switch (property_id) {
  case PROP_INPUT:
    g_value_set_string (value, self->input);
    break;
  case PROP_OUTPUT:
    g_value_set_string (value, self->input);
    break;
  case PROP_SINK:
    g_value_set_object (value, self->sink);
    break;
  case PROP_MUXER:
    g_value_set_string (value, self->muxer);
    break;
  case PROP_RATE:
    g_value_set_int (value, self->rate);
    break;
  case PROP_MAXIMUM_CHUNK_TIME:
    g_value_set_uint64 (value, self->max_chunk_time);
    break;
  case PROP_MINIMUM_SILENCE_TIME:
    g_value_set_uint64 (value, self->min_silence_time);
    break;
  case PROP_SILENCE_HYSTERESIS:
    g_value_set_uint64 (value, self->hysteresis);
    break;
  case PROP_SILENCE_THRESHOLD:
    g_value_set_int (value, self->threshold);
    break;
  case PROP_PLAYING:
    g_value_set_boolean (value, self->playing);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}


static const char* PIPELINE_TEMPLATE =
  "decodebin name=decodebin ! audioconvert "
  "! removesilence silent=false squash=true remove=true hysteresis=%lu "
  "  minimum-silence-time=%lu threshold=%i ! "
  "audioresample name=resample";


static GstElement *
create_pipeline (MarsChunker *self)
{
  gboolean using_mic;
  g_autofree char *parse_desc;
  g_autoptr (GError) error = NULL;
  g_autoptr (GstElement) decodebin = NULL;
  g_autoptr (GstElement) resample = NULL;
  g_autoptr (GstCaps) caps = NULL;
  GstElement *src;
  GstElement *splitmuxsink;
  GstElement *pipeline;

  using_mic = g_strcmp0 (self->input, MARS_CHUNKER_INPUT_MIC) == 0;

  if (using_mic)
    src = gst_element_factory_make ("pulsesrc", NULL);
  else
    src = gst_element_factory_make_full ("filesrc", "location", self->input, NULL);

  parse_desc = g_strdup_printf (PIPELINE_TEMPLATE,
                                self->hysteresis,
                                self->min_silence_time, self->threshold,
                                NULL);

  pipeline = gst_parse_launch (parse_desc, &error);

  if (error) {
    if (pipeline == NULL) {
      g_critical ("Unable to create pipeline: %s", error->message);
      return NULL;
    } else {
      g_warning ("%s", error->message);
    }
  }

  if (!gst_bin_add (GST_BIN (pipeline), src)) {
    g_critical ("Unable to add source");
    return NULL;
  }

  decodebin = gst_bin_get_by_name (GST_BIN (pipeline), "decodebin");

  if (!gst_element_link (src, decodebin)) {
    g_critical ("Unable to link source and decodebin");
    return NULL;
  }

  if (self->output) {
    splitmuxsink = gst_element_factory_make_full ("splitmuxsink",
                                                  "name", "muxsink",
                                                  "location", self->output,
                                                  "max-size-time", self->max_chunk_time,
                                                  "muxer-factory", self->muxer,
                                                  NULL);
  } else {
    splitmuxsink = gst_element_factory_make_full ("splitmuxsink",
                                                  "name", "muxsink",
                                                  "sink", self->sink,
                                                  "max-size-time", self->max_chunk_time,
                                                  "muxer-factory", self->muxer,
                                                  NULL);
  }

  if (!gst_bin_add (GST_BIN (pipeline), splitmuxsink)) {
    g_critical ("Unable to add splitmuxsink");
    return NULL;
  }

  resample = gst_bin_get_by_name (GST_BIN (pipeline), "resample");
  caps = gst_caps_new_simple ("audio/x-raw", "rate", G_TYPE_INT, self->rate, NULL);

  if (!gst_element_link_filtered (resample, splitmuxsink, caps)) {
    g_critical ("Unable to link audioresample and splitmuxsink");
    return NULL;
  }

  return pipeline;
}


static void
on_eos (MarsChunker *self, GstMessage *message)
{
  g_debug ("EOS reached");
  mars_chunker_stop (self);
}


static void
on_error (MarsChunker *self, GstMessage *message)
{
  g_autoptr (GError) error;

  gst_message_parse_error (message, &error, NULL);
  g_critical ("%s: %s", GST_OBJECT_NAME (message->src), error->message);

  mars_chunker_stop (self);
}


static void
on_message (MarsChunker *self, GstMessage *message)
{
  const GstStructure *structure;
  guint64 value;

  if (!gst_message_has_name (message, "removesilence"))
    return;

  structure = gst_message_get_structure (message);

  if (!gst_structure_get_uint64 (structure, "silence_detected", &value))
    return;

  g_debug ("Chunking");
  g_signal_emit_by_name (self->muxsink, "split-now", NULL);
  g_signal_emit (self, signals[CHUNKED], 0);
}


static GstBusSyncReply
sync_message_handler (GstBus *bus, GstMessage *message, MarsChunker *self)
{
  switch (GST_MESSAGE_TYPE (message)) {
  case GST_MESSAGE_EOS:
    on_eos (self, message);
    break;
  case GST_MESSAGE_ERROR:
    on_error (self, message);
    break;
  default:
    on_message (self, message);
  }
  return GST_BUS_PASS;
}


static void
mars_chunker_dispose (GObject *object)
{
  MarsChunker *self = MARS_CHUNKER (object);

  g_clear_object (&self->muxsink);
  g_clear_object (&self->pipeline);

  G_OBJECT_CLASS (mars_chunker_parent_class)->dispose (object);
}


static void
mars_chunker_finalize (GObject *object)
{
  MarsChunker *self = MARS_CHUNKER (object);

  g_free (self->input);
  g_free (self->output);
  g_free (self->muxer);

  G_OBJECT_CLASS (mars_chunker_parent_class)->finalize (object);
}


static void
mars_chunker_constructed (GObject *object)
{
  MarsChunker *self = MARS_CHUNKER (object);
  g_autoptr (GstBus) bus;

  G_OBJECT_CLASS (mars_chunker_parent_class)->constructed (object);

  self->pipeline = create_pipeline (self);

  if (self->pipeline == NULL)
    return;

  self->muxsink = gst_bin_get_by_name (GST_BIN (self->pipeline), "muxsink");
  bus = gst_element_get_bus (self->pipeline);
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) sync_message_handler, self, NULL);
}


static void
mars_chunker_class_init (MarsChunkerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = mars_chunker_constructed;
  object_class->dispose = mars_chunker_dispose;
  object_class->finalize = mars_chunker_finalize;
  object_class->set_property = mars_chunker_set_property;
  object_class->get_property = mars_chunker_get_property;

  /**
   * MarsChunker:input:
   *
   * Proxy for `Gst.filesrc:location`.
   * Use `mic` to read from microphone.  */
  props[PROP_INPUT] =
    g_param_spec_string ("input", "", "",
                         MARS_CHUNKER_INPUT_MIC,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:output:
   *
   * Proxy for `Gst.splitmuxsink:location`.
   * Takes precedence over `MarsChunker:sink`.  */
  props[PROP_OUTPUT] =
    g_param_spec_string ("output", "", "",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:sink:
   *
   * Proxy for `Gst.splitmuxsink:sink`.*/
  props[PROP_SINK] =
    g_param_spec_object ("sink", "", "",
                         GST_TYPE_ELEMENT,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:muxer:
   *
   * Proxy for `Gst.splitmuxsink:muxer-factory`. */
  props[PROP_MUXER] =
    g_param_spec_string ("muxer", "", "",
                         NULL,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:rate:
   *
   * Sample rate of chunked audio. */
  props[PROP_RATE] =
    g_param_spec_int ("rate", "", "",
                      1, G_MAXINT, 44100,
                      G_PARAM_READWRITE |
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:maximum-chunk-time:
   *
   * Proxy for `Gst.splitmuxsink:max-size-time`. */
  props[PROP_MAXIMUM_CHUNK_TIME] =
    g_param_spec_uint64 ("maximum-chunk-time", "", "",
                         0, G_MAXUINT64, MARS_CHUNKER_MAXIMUM_CHUNK_TIME,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:minimum-silence-time:
   *
   * Proxy for `Gst.removesilence:minimum-silence-time`. */
  props[PROP_MINIMUM_SILENCE_TIME] =
    g_param_spec_uint64 ("minimum-silence-time", "", "",
                         0, G_MAXUINT64, MARS_CHUNKER_MINIMUM_SILENCE_TIME,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:silence-hysteresis:
   *
   * Proxy for `Gst.removesilence:hysteresis`. */
  props[PROP_SILENCE_HYSTERESIS] =
    g_param_spec_uint64 ("silence-hysteresis", "", "",
                         0, G_MAXUINT64, MARS_CHUNKER_SILENCE_HYSTERESIS,
                         G_PARAM_READWRITE |
                         G_PARAM_CONSTRUCT_ONLY |
                         G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:silence-threshold:
   *
   * Proxy for `Gst.removesilence:threshold`. */
  props[PROP_SILENCE_THRESHOLD] =
    g_param_spec_int ("silence-threshold", "", "",
                      G_MININT, G_MAXINT, MARS_CHUNKER_SILENCE_THRESHOLD,
                      G_PARAM_READWRITE |
                      G_PARAM_CONSTRUCT_ONLY |
                      G_PARAM_STATIC_STRINGS);

  /**
   * MarsChunker:playing:
   *
   * Whether the pipeline is in playing state. */
  props[PROP_PLAYING] =
    g_param_spec_boolean ("playing", "", "",
                          FALSE,
                          G_PARAM_READABLE |
                          G_PARAM_EXPLICIT_NOTIFY |
                          G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, PROP_LAST_PROP, props);

  signals[CHUNKED] = g_signal_new ("chunked",
                                   G_OBJECT_CLASS_TYPE (object_class),
                                   G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                                   0,
                                   NULL, NULL,
                                   NULL,
                                   G_TYPE_NONE,
                                   0);
}


static void
mars_chunker_init (MarsChunker *self)
{
}


MarsChunker *
mars_chunker_new (char *input, char *output, char *muxer)
{
  return g_object_new (MARS_TYPE_CHUNKER, "input", input, "output", output, "muxer", muxer, NULL);
}


gboolean
mars_chunker_is_playing (MarsChunker *self)
{
  g_return_val_if_fail (MARS_IS_CHUNKER (self), FALSE);

  return self->playing;
}


void
mars_chunker_play (MarsChunker *self)
{
  g_return_if_fail (MARS_IS_CHUNKER (self));

  g_debug ("Starting playback");
  self->playing = TRUE;

  gst_element_set_state (self->pipeline, GST_STATE_PLAYING);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PLAYING]);
}


void
mars_chunker_pause (MarsChunker *self)
{
  g_return_if_fail (MARS_IS_CHUNKER (self));

  g_debug ("Pausing playback");
  self->playing = FALSE;

  gst_element_set_state (self->pipeline, GST_STATE_PAUSED);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PLAYING]);
}


void
mars_chunker_stop (MarsChunker *self)
{
  g_return_if_fail (MARS_IS_CHUNKER (self));

  g_debug ("Stopping playback");
  self->playing = FALSE;

  gst_element_set_state (self->pipeline, GST_STATE_NULL);
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_PLAYING]);
}
