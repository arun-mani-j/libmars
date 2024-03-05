/*
 * Copyright (C) 2024 Tether Operations Limited
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define MARS_CHUNKER_INPUT_MIC "mic"
#define MARS_CHUNKER_MAXIMUM_CHUNK_TIME 7 * GST_SECOND
#define MARS_CHUNKER_MINIMUM_SILENCE_TIME GST_SECOND / 2
#define MARS_CHUNKER_SILENCE_HYSTERESIS 480
#define MARS_CHUNKER_SILENCE_THRESHOLD -60

#define MARS_TYPE_CHUNKER mars_chunker_get_type ()
G_DECLARE_FINAL_TYPE (MarsChunker, mars_chunker, MARS, CHUNKER, GObject)

MarsChunker *mars_chunker_new (char *input, char *output, char *muxer);

gboolean mars_chunker_is_playing (MarsChunker *self);

void     mars_chunker_play (MarsChunker *self);
void     mars_chunker_pause (MarsChunker *self);
void     mars_chunker_stop (MarsChunker *self);

G_END_DECLS
