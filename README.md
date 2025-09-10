# Gucharmap

# CI status
---------

[![pipeline status](https://gitlab.gnome.org/GNOME/gucharmap/badges/master/pipeline.svg)](https://gitlab.gnome.org/GNOME/gucharmap/-/commits/master)

[![coverage report](https://gitlab.gnome.org/GNOME/gucharmap/badges/master/coverage.svg)](https://gitlab.gnome.org/GNOME/gucharmap/-/commits/master)

# Releases

[![Latest Release](https://gitlab.gnome.org/GNOME/gucharmap/-/badges/release.svg)](https://gitlab.gnome.org/GNOME/gucharmap/-/releases)

Tarballs for newer releases are available from the
[package registry](https://gitlab.gnome.org/GNOME/gucharmap/-/packages)
and new and old release are also available on
[download.gnome.org](https://download.gnome.org/sources/gucharmap/).

# Source code

To get the source code, use
```
$ git clone https://gitlab.gnome.org/GNOME/gucharmap
```

# Unicode data

Gucharmap ships with a copy of the unicode data files it needs to
build, but you can also provide an external (absolute) path to the
unicode data files to the -Ducd_path=... argument when running
meson setup.
