project('mars', 'c',
  version : '0.2',
  default_options : ['warning_level=3'])

add_project_arguments(['-Wno-unused-parameter'],
  language: 'c')

subdir('src')

files = [
 'src/main.c',
]

exe = executable('mars', files,
  dependencies: mars_dep,
  install : true)
