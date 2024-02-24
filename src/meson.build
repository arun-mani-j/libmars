gst = dependency('gstreamer-1.0')

files = [
  'chunker.c',
  'chunker.h',
]

mars_lib = library('mars', files, dependencies: gst)
mars_dep = declare_dependency(
  link_with: mars_lib,
  dependencies: gst
  )
