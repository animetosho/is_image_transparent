Is Image Transparent
====================

This is a simple and dirty tool to check whether a given PNG or WebP image is
*completely* transparent. A completely transparent image is an image which
contains alpha data and where all pixels have an alpha (or opacity) value of 0.

This tool may be useful for optimising cases where empty images can be removed
(e.g. determining whether an overlay image can be safely discarded).

Compiling
---------

The tool requires libwebp and libpng (\>=1.6) development libraries to be
available. On Debian, you can use `apt-get install libpng-dev libwebp-dev` to
get these.

Compiling can simply be done via the command: `gcc -Wall -O2 -lwebp -lpng
image_empty_check.c -o is_image_transparent`

If you don’t want WebP or PNG support, this can be disabled by omitting the
relevant linker flag (`-l*`) and adding `-DNO_WEBP` or `-DNO_PNG` to the compile
command.

Usage
-----

`./is_image_transparent [file]`

*[file]* can be a PNG or WebP file.

The result is in the exit/return code, which can be:

-   0: image is completely transparent

-   1: image has non-transparent pixels (is always the case if image contains no
    alpha information)

-   2: error occurred. Error message is written to stderr

Nothing is ever written to stdout, and only fatal error messages written to
stderr.

Limitations
-----------

-   The alpha channel is rendered in 8-bit, which means that if you’re using a
    higher bit depth (e.g. 16-bit alpha channel), there may be some accuracy
    loss

-   By default, it will refuse to read in files larger than 32MB, or decoded
    images (32-bit ARGB format) larger than 128MB in size. You can adjust these
    limits by supplying values for the defines `MAX_SIZE` and/or
    `MAX_IMAGE_SIZE` during compile, for example, to set the max input size to
    1MB, append to the compile `-DMAX_SIZE=1048576`
