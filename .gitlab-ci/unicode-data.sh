#!/bin/bash
#
# Copyright © 2025 Christian Persch
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this library.  If not, see <https://www.gnu.org/licenses/>.

UNICODE_DATA_GIT_REPO=https://gitlab.gnome.org/chpe/unicode-data.git
UNICODE_DATA_PREFIX=/usr

set -e
set -x

git clone --depth=1 ${UNICODE_DATA_GIT_REPO}
pushd unicode-data

mkdir _build
meson setup --prefix=${UNICODE_DATA_PREFIX} _build
meson compile -C _build
meson install -C _build

popd
rm -rf unicode-data

exit 0
