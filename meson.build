project('libst7036', 'c')

libsources = [
  'src/gpio.c',
  'src/libst7036.c'
]

libst7036 = shared_library('st7036', libsources,  version : '0.1', soversion : '0', install : true)

executable('st7036-example', 'src/example.c', link_with: libst7036, install : true)
