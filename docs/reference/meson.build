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

libgucharmap_private_headers = [
  'config.h',
  'gucharmap-chartable-accessible.h',
  'gucharmap-chartable-cell-accessible.h',
  'gucharmap-font-codepoint-list.h',
  'gucharmap-mini-fontsel.h',
  'gucharmap-print-operation.h',
  'gucharmap-private.h',
  'gucharmap-search-dialog.h',
  'gucharmap-settings.h',
  'gucharmap-type-builtins.h',
  'gucharmap-version.h',
  'gucharmap-window.h',
  'gucharmap.h',
  'unicode-blocks.h',
  'unicode-categories.h',
  'unicode-names.h',
  'unicode-nameslist.h',
  'unicode-scripts.h',
  'unicode-unihan.h',
  'unicode-versions.h',
]

atk_dep   = dependency('atk')
cairo_dep = dependency('cairo')
pango_dep = dependency('pango')

atk_prefix  = glib_dep.get_pkgconfig_variable('prefix')
cairo_prefix = cairo_dep.get_pkgconfig_variable('prefix')
glib_prefix  = glib_dep.get_pkgconfig_variable('prefix')
pango_prefix = gtk3_dep.get_pkgconfig_variable('prefix')

fixxrefs = [
  '--extra-dir=' + atk_prefix    / gnome.gtkdoc_html_dir('atk'),
  '--extra-dir=' + cairo_prefix  / gnome.gtkdoc_html_dir('cairo'),
  '--extra-dir=' + glib_prefix   / gnome.gtkdoc_html_dir('glib'),
  '--extra-dir=' + glib_prefix   / gnome.gtkdoc_html_dir('gobject'),
  '--extra-dir=' + pango_prefix  / gnome.gtkdoc_html_dir('pango'),
]

if get_option('gtk3')

  gtk3_prefix = gtk3_dep.get_pkgconfig_variable('prefix')
  fixxrefs_gtk3 = fixxrefs + [
    '--extra-dir=' + gtk3_prefix / gnome.gtkdoc_html_dir('gtk'),
  ]

  gnome.gtkdoc(
    'gucharmap',
    c_args: ['-DGUCHARMAP_COMPILATION', '-Wno-error'],
    dependencies: libgucharmap_gtk3_dep,
    fixxref_args: fixxrefs_gtk3,
    ignore_headers: libgucharmap_private_headers,
    include_directories: [top_inc, src_inc],
    install: true,
    main_sgml: 'gucharmap-docs.sgml',
    mkdb_args: ['--sgml-mode', '--output-format=xml',],
    module_version: libgucharmap_gtk3_api_version,
    namespace: 'gucharmap',
    scan_args: ['--rebuild-sections', '--rebuild-types',],
    src_dir: [top_inc, src_inc,],
  )
endif
