# WonderBrush 2.1.2

WonderBrush is a vector/bitmap graphics application for the Haiku operating system.
See http://yellowbites.com/wonderbrush.html for more details.
To learn more about Haiku, visit https://haiku-os.org.

This is the last known version of the code for WonderBrush as it is included in Haiku.
It was restored from a backup and cleaned up.

A new buildsystem (based on the Jamfile engine) allows to compile WonderBrush from these sources.

If someone wants to port to a new architecture, I am looking forward to pull requests.
Consider contributing to https://github.com/stippi/WonderBrush-v3 for new features and other improvements, as this version 2 is now in maintenance mode.

## Building

The development versions these librareis are needed:
- `libexpat`
- `liblayout`
- `libfreetype`

Install the dependencies via:

```shell
pkgman install freetype_devel liblayout_devel expat_devel
```

Of course you also need `gcc` and `jam`, but these come included with Haiku.

Build via:

```shell
TARGET_PLATFORM=haiku64 jam -j<number of your CPU cores>
```

or

```shell
TARGET_PLATFORM=haiku jam -j<number of your CPU cores>
```

depending on your Haiku system.
