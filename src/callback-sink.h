/*
 * Copyright (C) 2024 Tether Operations Limited
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <gst/base/base.h>

G_BEGIN_DECLS

#define MARS_TYPE_CALLBACK_SINK mars_callback_sink_get_type ()
G_DECLARE_FINAL_TYPE (MarsCallbackSink, mars_callback_sink, MARS, CALLBACK_SINK, GstBaseSink)

typedef void (*MarsBufferCallback) (GstBuffer *buffer, gpointer user_data);
typedef void (*MarsBufferListCallback) (GPtrArray *buffer_list, gpointer user_data);

GstElement *mars_callback_sink_new (void);

void        mars_callback_sink_set_buffer_callback (MarsCallbackSink  *self,
                                                    MarsBufferCallback buffer_cb,
                                                    gpointer           user_data,
                                                    GDestroyNotify     destroy);
void        mars_callback_sink_set_buffer_list_callback (MarsCallbackSink      *self,
                                                         MarsBufferListCallback buffer_list_cb,
                                                         gpointer               user_data,
                                                         GDestroyNotify         destroy);

G_END_DECLS
