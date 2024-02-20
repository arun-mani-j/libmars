project('chunker', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

add_project_arguments(['-Wno-unused-parameter'],
  language: 'c')

gst = dependency('gstreamer-1.0')

exe = executable('chunker', 'chunker.c',
  dependencies: gst,
  install : true)
