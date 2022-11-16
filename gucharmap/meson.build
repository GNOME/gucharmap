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

src_inc = include_directories('.')

disable_deprecated_cppflags = [
  '-DATK_DISABLE_DEPRECATION_WARNINGS',
  '-DGDK_DISABLE_DEPRECATION_WARNINGS',
  '-DGDK_PIXBUF_DISABLE_DEPRECATION_WARNINGS',
  '-DGLIB_DISABLE_DEPRECATION_WARNINGS',
  '-DGTK_DISABLE_DEPRECATION_WARNINGS',
  '-DPANGO_DISABLE_DEPRECATION_WARNINGS',
]

# Library

libgucharmap_gtk3_includedir = gucharmap_includedir / libgucharmap_gtk3_api_path

version_conf = {
  'GUCHARMAP_VERSION_MAJOR': gucharmap_major_version.to_string(),
  'GUCHARMAP_VERSION_MINOR': gucharmap_minor_version.to_string(),
  'GUCHARMAP_VERSION_MICRO': gucharmap_micro_version.to_string(),
}

libgucharmap_version_header = configure_file(
  configuration: version_conf,
  input: 'gucharmap-version.h.in',
  install: false,
  output: '@BASENAME@',
)

libgucharmap_enum_headers = files(
  'gucharmap-chapters-model.h',
  'gucharmap-charmap.h',
  'gucharmap-unicode-info.h',
)

libgucharmap_public_headers = libgucharmap_enum_headers + [libgucharmap_version_header,] + files(
  'gucharmap-block-chapters-model.h',
  'gucharmap-block-codepoint-list.h',
  'gucharmap-chapters-view.h',
  'gucharmap-chartable.h',
  'gucharmap-codepoint-list.h',
  'gucharmap-macros.h',
  'gucharmap-script-chapters-model.h',
  'gucharmap-script-codepoint-list.h',
  'gucharmap.h',
)

install_headers(
  libgucharmap_public_headers,
  subdir: libgucharmap_gtk3_api_path,
)

libgucharmap_enum_sources = gnome.mkenums(
  'gucharmap-type-builtins.h',
  c_template: 'gucharmap-type-builtins.c.template',
  h_template: 'gucharmap-type-builtins.h.template',
  install_dir: libgucharmap_gtk3_includedir,
  install_header: true,
  sources: libgucharmap_enum_headers,
)

# Due to a meson bug, this header cannot be installed with the rule above. Instead,
# use the install_header attribute in the mkenums call, and add the header afterwards
# to the list. See https://gitlab.gnome.org/chpe/meson/issues/4
libgucharmap_public_headers += libgucharmap_enum_sources[1]

libgucharmap_sources = libgucharmap_enum_sources + files(
  'gucharmap-block-chapters-model.c',
  'gucharmap-block-chapters-model.h',
  'gucharmap-block-codepoint-list.c',
  'gucharmap-block-codepoint-list.h',
  'gucharmap-chapters-model.c',
  'gucharmap-chapters-model.h',
  'gucharmap-chapters-view.c',
  'gucharmap-chapters-view.h',
  'gucharmap-charmap.c',
  'gucharmap-charmap.h',
  'gucharmap-chartable-accessible.c',
  'gucharmap-chartable-accessible.h',
  'gucharmap-chartable-cell-accessible.c',
  'gucharmap-chartable-cell-accessible.h',
  'gucharmap-chartable.c',
  'gucharmap-chartable.h',
  'gucharmap-codepoint-list.c',
  'gucharmap-codepoint-list.h',
  'gucharmap-init.c',
  'gucharmap-private.h',
  'gucharmap-script-chapters-model.c',
  'gucharmap-script-chapters-model.h',
  'gucharmap-script-codepoint-list.c',
  'gucharmap-script-codepoint-list.h',
  'gucharmap-unicode-info.c',
  'gucharmap-unicode-info.h',
)

libgucharmap_sources += gnome.genmarshal(
  'gucharmap-marshal',
  internal: true,
  prefix: '_gucharmap_marshal',
  sources: 'gucharmap-marshal.list',
)

ucd_path = get_option('ucd_path')
assert(ucd_path != '', 'must specify path to UCD files')

# Meson bug! This should use files('...') (allowed as per docs), but that
# crashes meson; see https://gitlab.gnome.org/chpe/meson/issues/2
generate_unicode_data = find_program('gen-guch-unicode-tables.pl')

unicode_sources = custom_target(
  'unicode-data',
  command: [generate_unicode_data, unicode_req_version, ucd_path, '@OUTDIR@'],
  install: false,
  output: [
    'unicode-blocks.h',
    'unicode-categories.h',
    'unicode-names.h',
    'unicode-nameslist.h',
    'unicode-scripts.h',
    'unicode-unihan.h',
    'unicode-versions.h',
  ],
)

libgucharmap_gtk3_public_deps = [
  gio_dep,
  glib_dep,
  gtk3_dep,
  pcre2_dep,
]

libgucharmap_gtk3_deps = libgucharmap_gtk3_public_deps

libgucharmap_gtk3_cppflags = disable_deprecated_cppflags + [
  '-DGUCHARMAP_COMPILATION',
  '-DG_LOG_DOMAIN="Gucharmap"',
  '-DLOCALEDIR="@0@"'.format(gucharmap_prefix / gucharmap_localedir),
]

libgucharmap_gtk3_sources = libgucharmap_sources
libgucharmap_gtk3_public_headers = libgucharmap_public_headers

# for legacy reasons
libgucharmap_gtk3_libname = 'gucharmap_@0@_@1@'.format(gucharmap_api_major_version, gucharmap_api_minor_version)

libgucharmap_gtk3 = shared_library(
  libgucharmap_gtk3_libname,
  c_args: libgucharmap_gtk3_cppflags,
  dependencies: libgucharmap_gtk3_deps,
  include_directories: [top_inc, src_inc,],
  install: true,
  sources: libgucharmap_gtk3_sources + [unicode_sources,],
  version: libgucharmap_gtk3_soversion,
)

libgucharmap_gtk3_dep = declare_dependency(
  dependencies: libgucharmap_gtk3_deps,
  include_directories: [top_inc, src_inc,],
  link_with: libgucharmap_gtk3,
  sources: libgucharmap_gtk3_public_headers,
)

# pkg-config file

pkg.generate(
  libgucharmap_gtk3,
  description: 'Gucharmap widget for GTK+ 3.0',
  filebase: libgucharmap_gtk3_api_name,
  name: 'gucharmap',
  requires: libgucharmap_gtk3_public_deps,
  subdirs: libgucharmap_gtk3_api_name,
  version: gucharmap_version,
  variables: [
    'exec_prefix=${prefix}',
  ],
)

# Introspection

if get_option('gir')
  libgucharmap_gtk3_gir_includes = [
    'Gtk-3.0',
    'Pango-1.0'
  ]

  libgucharmap_gtk3_gir = gnome.generate_gir(
    libgucharmap_gtk3,
    dependencies: libgucharmap_gtk3_dep,
    export_packages: libgucharmap_gtk3_api_name,
    extra_args: '-DGUCHARMAP_COMPILATION',
    header: 'gucharmap' / 'gucharmap.h',
    includes: libgucharmap_gtk3_gir_includes,
    install: true,
    namespace: 'Gucharmap',
    nsversion: libgucharmap_gtk3_api_version,
    sources: libgucharmap_gtk3_public_headers,
  )
endif

# Vala bindings

if get_option('vapi')
  assert(get_option('gir'), 'gir is required for vala support')
  assert(get_option('gtk3'), 'vala support only available for gtk3')

  add_languages('vala', required: true,)

  valac = meson.get_compiler('vala')
  assert(valac.version().version_compare('>= 0.16'), 'vala >= 0.16 required')

  libgucharmap_gtk3_vapi_deps = [
    'gio-2.0',
    'glib-2.0',
    'gtk+-3.0',
    'pango',
  ]

  libgucharmap_gtk3_vapi = gnome.generate_vapi(
    libgucharmap_gtk3_api_name,
    install: true,
    packages: libgucharmap_gtk3_vapi_deps,
    sources: libgucharmap_gtk3_gir[0],
  )
endif

# Charmap application

gucharmap_sources = files(
  'gucharmap-mini-fontsel.c',
  'gucharmap-mini-fontsel.h',
  'gucharmap-print-operation.c',
  'gucharmap-print-operation.h',
  'gucharmap-search-dialog.c',
  'gucharmap-search-dialog.h',
  'gucharmap-settings.c',
  'gucharmap-settings.h',
  'gucharmap-window.c',
  'gucharmap-window.h',
  'main.c',
)

gucharmap_gtk3_sources = gucharmap_sources

gucharmap_gtk3_sources += gnome.compile_resources(
  'gucharmap-resources',
  'gucharmap.gresource.xml',
  c_name: 'gucharmap',
)

gucharmap_gtk3_cppflags = disable_deprecated_cppflags + [
  '-DLOCALEDIR="@0@"'.format(gucharmap_prefix / gucharmap_localedir),
]

gucharmap_gtk3_deps = [
  freetype2_dep,
  gtk3_dep,
  libdl_dep,
  libgucharmap_gtk3_dep,
]

gucharmap = executable(
  'gucharmap',
  gucharmap_gtk3_sources,
  c_args: gucharmap_gtk3_cppflags,
  dependencies: gucharmap_gtk3_deps,
  include_directories: [top_inc, src_inc,],
  install: get_option('charmap'),
)

# Install settings schemas

schemadir = gucharmap_prefix / gucharmap_datadir / 'glib-2.0' / 'schemas'

install_data(
  'org.gnome.Charmap.gschema.xml',
  install_dir: schemadir,
)

gnome.post_install(
  glib_compile_schemas: true,
)
