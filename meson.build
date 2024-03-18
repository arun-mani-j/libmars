project('mars', 'c',
  version : '0.5',
  default_options : ['warning_level=3'])

add_project_arguments(['-Wno-unused-parameter'],
  language: 'c')

root_inc = include_directories('.')

subdir('src')
subdir('examples')
