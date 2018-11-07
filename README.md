# WonderBrush 2.1.2
WonderBrush is a vector/bitmap graphics application for the Haiku operating system. See http://yellowbites.com/wonderbrush.html for more details. To learn more about Haiku, visit https://haiku-os.org.

This is the last known version of the code for WonderBrush as it is included in Haiku, restored from a backup and stripped
off the SVN stuff and most code which should not be necessary to build WonderBrush.

The code as-is does not build on Haiku. It should in theory build at least on x86_gcc2. Currently it gets stuck on finding 
the freetype headers.

If someone wants to get this building or port to a new architecture, I am looking forward to pull requests, and I am fine 
with replacing the buildsystem, for example with the JamfileEngine.
