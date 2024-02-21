# chunker
A simple chunker of audio stream by silence using GStreamer.

## Getting Started
You need [GStreamer](https://gstreamer.freedesktop.org/documentation/index.html)
for the usage. [Meson](https://mesonbuild.com/) can be used to build the
project.

``` sh
$ git clone git@github.com:arun-mani-j/chunker.git
$ cd chunker
$ meson setup _build/ # Setup build directory
$ meson compile -C _build/ # Compile the project
```

## Usage
Suppose you want to chunk a file named `hello.wav` file and save its
output in `/output` directory, then you can use the following command.

``` sh
$ mkdir -p output # Create the directory if it does not exist
$ _build/chunker -i "hello.wav" -o "output/%02d.wav" -m "muxer"
```

You can use [`data/sample.wav`](data/sample.wav) to test the chunking process.

## Customization
Chunker relies on GStreamer to do all the heavy-lifting. To improve the output,
you should try tweaking the properties of
[`removesilence`](https://gstreamer.freedesktop.org/documentation/removesilence/index.html?gi-language=c)
element.

You can do the customization by modifying the constants in [`chunker.h`](/chunker.h).

## References
1. [`removesilence`](https://gstreamer.freedesktop.org/documentation/removesilence/index.html?gi-language=c) - GStreamer element to detect and remove silence.
2. [`splitmuxsink`](https://gstreamer.freedesktop.org/documentation/multifile/splitmuxsink.html?gi-language=c) - GStreamer element to split the files. 
