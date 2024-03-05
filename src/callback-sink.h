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

typedef void (*MarsBufferCallback) (GstBuffer *);
typedef void (*MarsBufferListCallback) (GPtrArray *);

GstElement *mars_callback_sink_new (void);

void        mars_callback_sink_set_buffer_callback (MarsCallbackSink  *self,
                                                    MarsBufferCallback buffer_cb);
void        mars_callback_sink_set_buffer_list_callback (MarsCallbackSink      *self,
                                                         MarsBufferListCallback buffer_cb);

G_END_DECLS
