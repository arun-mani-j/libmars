project('chunker', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

add_project_arguments(['-Wno-unused-parameter'],
  language: 'c')

gst = dependency('gstreamer-1.0')

files = [
  'chunker.c',
  'chunker.h'
]

exe = executable('chunker', files,
  dependencies: gst,
  install : true)
