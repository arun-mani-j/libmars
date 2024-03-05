gst = dependency('gstreamer-1.0')
gst_base = dependency('gstreamer-base-1.0')
deps = [gst, gst_base]

files = [
  'callback-sink.c',
  'callback-sink.h',
  'chunker.c',
  'chunker.h',
]

mars_inc = include_directories('.')
mars_lib_inc = [mars_inc, root_inc]

mars_lib = library('mars', files, dependencies: deps)
mars_dep = declare_dependency(
  link_with: mars_lib,
  dependencies: gst
)
