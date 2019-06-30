Gucharmap
=========

Unicode data
------------

Note that to build gucharmap, you need to provide the (absolute) path
to the unicode data files to the -Ducd_path=... argument when running
meson. These data files are not included in gucharmap; you need to
download them separately, or use the package provided by your
distribution. E.g. on fedora, install the unicode-ucd package.
