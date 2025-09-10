# Copyright © 2019 Christian Persch
#
# This programme is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This programme is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this programme.  If not, see <https://www.gnu.org/licenses/>.

project(
  'gucharmap',
  ['c',],
  default_options: [
    'buildtype=release',
    'c_std=gnu11',
    'warning_level=0',
    'b_ndebug=false',
  ],
  license: ['GPL-3.0-or-later',],
  meson_version: '>= 0.62.0',
  version: '17.0.0',
)

# Naming

gucharmap_name = 'gucharmap'

# Requirements

freetype2_req_version = '1.0'
gio_req_version       = '2.32.0'
glib_req_version      = '2.32.0'
gtk3_req_version      = '3.4.0'
pcre2_req_version     = '10.21'

# NOTE! This is an exact requirement. Each gucharmap version (major.minor) is
# designed to work with, and only with, this unicode version. Upgrading to
# a newer unicode version does not simply mean bumping this requirement, but
# also requires more or less (depending on the unicode version) extensive code
# changes to gucharmap. Therefore, you should NOT upgrade the unicode version
# of a released gucharmap, but upgrade to the gucharmap version corresponding
# to the desired unicode version.
unicode_req_version   = '17.0.0'

# API

gucharmap_api_major_version = 2
gucharmap_api_minor_version = 90

libgucharmap_gtk3_api_version = '@0@.@1@'.format(gucharmap_api_major_version, gucharmap_api_minor_version)
libgucharmap_gtk3_api_name = gucharmap_name + '-' + libgucharmap_gtk3_api_version
libgucharmap_gtk3_api_path = libgucharmap_gtk3_api_name / 'gucharmap'

# Versioning

gucharmap_version = meson.project_version()

version_split = meson.project_version().split('.')
gucharmap_major_version = version_split[0].to_int()
gucharmap_minor_version = version_split[1].to_int()
gucharmap_micro_version = version_split[2].to_int()

# Library versioning

lt_current  = 7
lt_revision = 0
lt_age      = 0

libgucharmap_gtk3_soversion = '@0@.@1@.@2@'.format(lt_current, lt_revision, lt_age)

libgucharmap_gtk4_soversion = '0'

# i18n

gucharmap_gettext_domain = gucharmap_name

# Directories

gucharmap_datadir    = get_option('datadir')
gucharmap_includedir = get_option('includedir')
gucharmap_libdir     = get_option('libdir')
gucharmap_libexecdir = get_option('libexecdir')
gucharmap_localedir  = get_option('localedir')
gucharmap_prefix     = get_option('prefix')

gucharmap_pkgdatadir = gucharmap_datadir / gucharmap_name

# Debug

enable_debug = get_option('dbg') or get_option('debug') or get_option('buildtype').contains('debug')

# Meson modules

gnome = import('gnome')
i18n  = import('i18n')
pkg   = import('pkgconfig')

# Compilers

cc = meson.get_compiler('c')

# Meson has a misfeature where it allows the user to override the -std option
# for the C/C++ compiler. Disallow that.

assert(get_option('c_std') == 'gnu11', 'cannot override C std version')

# Asserts must not be disabled

assert(get_option('b_ndebug') == 'false', 'assertions may not be disabled')

# Start config.h

config_h = configuration_data()

config_h.set_quoted('GETTEXT_PACKAGE', gucharmap_gettext_domain)
config_h.set_quoted('VERSION', gucharmap_version)
config_h.set('ENABLE_DEBUG', enable_debug)

# FIXME AC_USE_SYSTEM_EXTENSIONS also supported non-gnu systems
config_h.set10('_GNU_SOURCE', true)

# Check for functions

libdl_dep = cc.find_library('dl')

check_dl_functions_required = [
  'dlsym',
]

foreach func: check_dl_functions_required
  assert(cc.has_function(func, dependencies: libdl_dep), func + ' not found')
endforeach

# Asserts must not be disabled

assert(get_option('b_ndebug') == 'false', 'assertions may not be disabled')

# LTO very much NOT supported

assert(get_option('b_lto') == false, 'LTO not supported')

# Compiler flags

compiler_flags_common = [
  '-Waggregate-return',
  '-Wall',
  '-Wcast-align',
  '-Werror=implicit-function-declaration',
  '-Wextra',
  '-Wformat-signedness',
  '-Wimplicit',
  '-Winit-self',
  '-Wmissing-declarations',
  '-Wmissing-format-attribute',
  '-Wmissing-prototypes',
  '-Wnested-externs',
  '-Wno-missing-field-initializers',
  '-Wno-cast-function-type',
  '-Wno-deprecated-declarations',
  '-Wno-switch-enum',
  '-Wno-unused-parameter',
  '-Wold-style-definition',
  '-Wpacked',
  '-Wpointer-arith',
  '-Wshadow',
  '-Wsign-compare',
  '-Wstrict-aliasing=2',
  '-Wstrict-prototypes',
  '-Wuninitialized',
  '-Wunsafe-loop-optimizations',
  '-Wwrite-strings',
  '-fno-common',
]

if enable_debug
  compiler_flags_common += [
    '-ggdb3',
  ]
endif

# These are currently needed but the code should be fixed instead
compiler_flags_common_undesirable = [
  '-fno-strict-aliasing'
]

compiler_flags_cc_required = [
  '-fvisibility=hidden',
]

global_cflags = cc.get_supported_arguments(compiler_flags_common +
                                           compiler_flags_common_undesirable +
                                           compiler_flags_cc_required)

foreach flag: compiler_flags_cc_required
  assert(cc.has_argument(flag), flag + ' is required but not supported')
endforeach

# These flags have to be tested together

compiler_flags_common_multi = [
  # FIXME: these flags break the gtkdoc-scangobj run
  ## These only work together with -Wformat
  #[
  #  '-Werror=format=2',
  #  '-Werror=format-nonliteral',
  #  '-Werror=format-security',
  #],
]

foreach flags : compiler_flags_common_multi
  if cc.has_multi_arguments(flags)
    global_cflags += flags
  endif
  if cxx.has_multi_arguments(flags)
    global_cxxflags += flags
  endif
endforeach

# ... and now make these flags the default

add_project_arguments(global_cflags, language: 'c',)

# Linker flags

linker_flags = [
  '-Wl,-Bsymbolic-functions'
]

foreach flag: linker_flags
  assert(cc.has_link_argument(flag), flag + ' is required but not supported')
  add_project_link_arguments(flag, language: 'c',)
endforeach

# Dependencies

freetype2_dep = dependency('freetype2',  version: '>=' + freetype2_req_version,)
gio_dep       = dependency('gio-2.0',    version: '>=' + gio_req_version,)
glib_dep      = dependency('glib-2.0',   version: '>=' + glib_req_version,)
pcre2_dep     = dependency('libpcre2-8', version: '>=' + pcre2_req_version)

if get_option('gtk3')
  gtk3_dep = dependency('gtk+-3.0',  version: '>=' + gtk3_req_version,)
else
  gtk3_dep = dependency('', required: false,)
endif

# Write config.h

configure_file(
  output: 'config.h',
  configuration: config_h,
)

# Subdirs

top_inc = include_directories('.')

subdir('po')
subdir('data')
subdir('gucharmap')

if get_option('docs')
  subdir('docs' / 'reference')
endif

if get_option('docs') and get_option('charmap')
  subdir('help')
endif

# Dist

meson.add_dist_script(
  find_program('meson_changelog.sh'),
)

# Simple compat Makefile

makefile_conf = configuration_data()
makefile_conf.set('srcdir', meson.current_source_dir())
makefile_conf.set('builddir', meson.current_build_dir())

configure_file(
  input: 'Makefile.meson',
  output: '@BASENAME@',
  configuration: makefile_conf,
)

# .gitignore everything in the build directory

configure_file(
  output: '.gitignore',
  command: ['echo', '**/**',],
  capture: true,
  install: false,
)

# Summary

output = '\n'
output += 'Configuration for Gucharmap:\n\n'
output += '  Version:                ' + gucharmap_version + '\n'
output += '  Unicode version:        ' + unicode_req_version + '\n'
output += '\n'
output += '  C compiler:             ' + cc.get_id() + '\n\n'
output += '  Coverage:               ' + get_option('b_coverage').to_string() + '\n'
output += '  Charmap:                ' + get_option('charmap').to_string() + '\n'
output += '  Documentation:          ' + get_option('docs').to_string() + '\n'
output += '  Debug:                  ' + enable_debug.to_string() + '\n'
output += '  GIR:                    ' + get_option('gir').to_string() + '\n'
output += '  GTK+ 3.0:               ' + get_option('gtk3').to_string() + '\n'
output += '  Vala:                   ' + get_option('vapi').to_string() + '\n'
output += '\n'
output += '  Prefix:                 ' + get_option('prefix') + '\n'
output += '  Desktop data dir:       ' + desktopdatadir + '\n'
output += '  Schemas dir:            ' + schemadir + '\n'
message(output)

# Done
