# Mars

A simple library for audio chunking and processing using GStreamer.

## Getting Started

You need [GStreamer](https://gstreamer.freedesktop.org/documentation/index.html)
for the usage. [Meson](https://mesonbuild.com/) can be used to build the
project.

```sh
$ git clone git@github.com:arun-mani-j/libmars.git
$ cd libmars
$ meson setup _build/ # Setup build directory
$ meson compile -C _build/ # Compile the project
```

## `MarsChunker`

Chunks the incoming audio stream by silence and duration.

### Example

Suppose you want to chunk a file in [`data/sample.wav`](data/sample.wav) and
save its output in `/output` directory, then you can use the following command.

```sh
$ mkdir -p output # Create the directory if it does not exist
$ _build/examples/chunker -i "data/sample.wav" -o "output/%02d.wav" -m "wavenc"
```

Pass `"mic"` to input, if you want to read from the default [mic](https://gstreamer.freedesktop.org/documentation/pulseaudio/pulsesrc.html?gi-language=c).

By default, the chunking happens if the audio segment size crosses `7` seconds.

### Customization

Chunker relies on GStreamer to do all the heavy-lifting. To improve the output,
you should try tweaking the properties of
[`removesilence`](https://gstreamer.freedesktop.org/documentation/removesilence/index.html?gi-language=c)
and
[`splitmuxsink`](https://gstreamer.freedesktop.org/documentation/multifile/splitmuxsink.html?gi-language=c)
elements.

You can do the customization by modifying the properties of
[`MarsChunker`](/src/chunker.c).

## `MarsCallbackSink`

A sink that calls a given callback for every buffer it gets. Similarly, it can
aggregate the buffers and call them when the stream ends.

### Example

The following example prints the number of buffers the sink received.

```sh
$ _build/examples/callback-sink -i "data/sample.wav" -m "wavenc"

```

Pass `"mic"` to input, if you want to read from the default [mic](https://gstreamer.freedesktop.org/documentation/pulseaudio/pulsesrc.html?gi-language=c).

## Library

You can use `libmars.so` in your application. See [`examples/`](examples/) for a demonstration.

## G-I Support

Mars supports [GNOME
Introspection](https://gi.readthedocs.io/en/latest/index.html). So you can use
it in all the languages supported by G-I which includes Python, Rust, Vala etc.

See [`examples/chunker.py`](/examples/chunker.py) for an example.

## References

1. [`removesilence`](https://gstreamer.freedesktop.org/documentation/removesilence/index.html?gi-language=c) - GStreamer element to detect and remove silence.
2. [`splitmuxsink`](https://gstreamer.freedesktop.org/documentation/multifile/splitmuxsink.html?gi-language=c) - GStreamer element to split the files.
