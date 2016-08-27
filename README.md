# README

This is a C library for using the [Browscap](https://browscap.org/) user agent database,
which is a descendant of [PHP's `get_browser()` function](https://secure.php.net/manual/en/function.get-browser.php) and probably of the [IIS Browser Capabilities component](https://msdn.microsoft.com/en-us/library/ms531077(v=vs.85).aspx#unknown_112) before that.

Or it will be someday. Right now it's just an unoptimized prototype.

## Building

Requires the [`iniparser`](https://github.com/ndevilla/iniparser) library,
[`strlcpy` and `strlcat`](https://www.sudo.ws/todd/papers/strlcpy.html)
(available from [`libbsd`](https://libbsd.freedesktop.org/wiki/) on Linux)
and `fnmatch` with the nonstandard `FNM_CASEFOLD` flag (available on Mac OS X, FreeBSD, and glibc).

Assuming you've installed `iniparser` through Homebrew, you can build the library
and its demo program with debug info like this on Mac OS X:

```bash
clang -std=c11 \
    -Wall -g \
    -I$(brew --prefix)/include \
    -L$(brew --prefix)/lib \
    -liniparser \
    -dynamiclib \
    browscap.c \
    -o libbrowscap.dylib

clang -std=c11 \
    -Wall -g \
    -L. \
    -lbrowscap \
    browscap_demo.c \
    -o browscap_demo
```

## Running

Requires [one of Browscap's `php_browscap.ini` files](https://browscap.org/) for data.
It supports regular, lite, and full versions, although it's slow with anything but lite.

The demo program takes a data file path followed by user agent strings on the command line:

```bash
./browscap_demo lite_php_browscap.ini \
    "Mozilla/5.0 (Linux; Android 5.1.1; SM-T337T Build/LMY47X) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.89 Safari/537.36" \
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.116 Safari/537.36"
```

Prints each user agent string followed by its properties:

```
Mozilla/5.0 (Linux; Android 5.1.1; SM-T337T Build/LMY47X) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/50.0.2661.89 Safari/537.36
    Comment = Chrome 50.0
    Browser = Chrome
    Version = 50.0
    Platform = Android
    isMobileDevice = true
    isTablet = false
    Device_Type = Mobile Phone

Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.116 Safari/537.36
    Comment = Chrome 52.0
    Browser = Chrome
    Version = 52.0
    Platform = MacOSX
    isMobileDevice = false
    isTablet = false
    Device_Type = Desktop

```
