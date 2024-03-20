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
  dependencies: deps,
)

gnome = import('gnome')
gir_args = [
    '-DMARS_COMPILATION',
    '--quiet',
    '--add-init-section=extern void gst_init(gint*,gchar**);' + 'gst_init(NULL,NULL);'
]
mars_gir = gnome.generate_gir(mars_lib,
  sources: files,
  namespace: 'Mars',
  nsversion: '1.0',
  identifier_prefix: 'Mars',
  symbol_prefix: 'mars',
  export_packages: 'mars1',
  includes: [ 'GLib-2.0', 'GObject-2.0', 'Gst-1.0', 'GstBase-1.0' ],
  install: true,
  dependencies: deps,
  extra_args: gir_args,
)
