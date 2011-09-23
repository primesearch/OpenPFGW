Install third-party libraries in this directory.

For Windows, use MPIR.  It is a Windows specific fork of GMP.  You need to configure it with these
flags:
	--enable-static --cpu-none --ABI32
or:
	--enable-static --cpu-none --ABI64
depending upon the version of Windows you are building for.