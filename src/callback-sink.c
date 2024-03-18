/*
 * Copyright (C) 2024 Tether Operations Limited
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Author: Arun Mani J <arunmani@peartree.to>
 */

#define G_LOG_DOMAIN "mars-callback-sink"

#include "callback-sink.h"

#include <stdio.h>

/**
 * MarsCallbackSink:
 *
 * Calls the given function when there is a buffer or the stream has reached EOS
 * with an array of buffers.
 *
 * For all the callbacks, the arguments should not be freed.
 */

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
                                                                    GST_PAD_SINK,
                                                                    GST_PAD_ALWAYS,
                                                                    GST_STATIC_CAPS_ANY);

struct _MarsCallbackSink {
  GstBaseSink            parent;

  MarsBufferCallback     buffer_cb;
  gpointer               buffer_cb_user_data;
  GDestroyNotify         buffer_cb_destroy;
  MarsBufferListCallback buffer_list_cb;
  gpointer               buffer_list_cb_user_data;
  GDestroyNotify         buffer_list_cb_destroy;
  GstBufferList         *buffers;
};

G_DEFINE_TYPE (MarsCallbackSink, mars_callback_sink, GST_TYPE_BASE_SINK)


static gboolean
start (GstBaseSink *sink)
{
  g_debug ("Starting");

  return TRUE;
}


static GstFlowReturn
render (GstBaseSink *sink, GstBuffer *buffer)
{
  MarsCallbackSink *self = MARS_CALLBACK_SINK (sink);

  g_debug ("Rendering buffer: %d", gst_buffer_list_length (self->buffers) + 1);
  gst_buffer_list_add (self->buffers, gst_buffer_ref (buffer));

  if (self->buffer_cb)
    self->buffer_cb (buffer, self->buffer_cb_user_data);

  return GST_FLOW_OK;
}


static gboolean
stop (GstBaseSink *sink)
{
  MarsCallbackSink *self = MARS_CALLBACK_SINK (sink);

  g_debug ("Stopping with buffers: %d", gst_buffer_list_length (self->buffers));

  if (self->buffer_list_cb)
    self->buffer_list_cb (self->buffers, self->buffer_list_cb_user_data);

  gst_buffer_list_remove (self->buffers, 0, gst_buffer_list_length (self->buffers));

  return TRUE;
}


static void
mars_callback_sink_finalize (GObject *object)
{
  MarsCallbackSink *self = MARS_CALLBACK_SINK (object);

  if (self->buffer_cb_destroy != NULL)
    self->buffer_cb_destroy (self->buffer_cb_user_data);

  if (self->buffer_list_cb_destroy != NULL)
    self->buffer_list_cb_destroy (self->buffer_list_cb_user_data);

  gst_clear_buffer_list (&self->buffers);

  G_OBJECT_CLASS (mars_callback_sink_parent_class)->finalize (object);
}


static void
mars_callback_sink_class_init (MarsCallbackSinkClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *sink_class = GST_BASE_SINK_CLASS (klass);

  object_class->finalize = mars_callback_sink_finalize;

  sink_class->start = start;
  sink_class->render = render;
  sink_class->stop = stop;

  gst_element_class_add_static_pad_template (element_class, &sinktemplate);

  gst_element_class_set_static_metadata (element_class,
                                         "CallbackSink",
                                         "Sink",
                                         "Calls the callback for every buffer",
                                         "Arun Mani J <arunmani@peartree.to>");
}


static void
mars_callback_sink_init (MarsCallbackSink *self)
{
  self->buffers = gst_buffer_list_new ();
}


GstElement *
mars_callback_sink_new (void)
{
  return g_object_new (MARS_TYPE_CALLBACK_SINK, NULL);
}


void
mars_callback_sink_set_buffer_callback (MarsCallbackSink  *self,
                                        MarsBufferCallback buffer_cb,
                                        gpointer           user_data,
                                        GDestroyNotify     destroy)
{
  g_return_if_fail (MARS_IS_CALLBACK_SINK (self));

  if (self->buffer_cb_destroy != NULL)
    self->buffer_cb_destroy (self->buffer_cb_user_data);

  self->buffer_cb = buffer_cb;
  self->buffer_cb_user_data = user_data;
  self->buffer_cb_destroy = destroy;
}


void
mars_callback_sink_set_buffer_list_callback (MarsCallbackSink      *self,
                                             MarsBufferListCallback buffer_list_cb,
                                             gpointer               user_data,
                                             GDestroyNotify         destroy)
{
  g_return_if_fail (MARS_IS_CALLBACK_SINK (self));

  if (self->buffer_list_cb_destroy != NULL)
    self->buffer_list_cb_destroy (self->buffer_list_cb_user_data);

  self->buffer_list_cb = buffer_list_cb;
  self->buffer_list_cb_user_data = user_data;
  self->buffer_list_cb_destroy = destroy;
}
