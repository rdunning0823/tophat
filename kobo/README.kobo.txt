Top Hat README.kobo.txt
=============

This documentation describes aspects of the Kobo configuration for Top Hat.  Please
read this in conjunction with the README document in the root folder of the Top Hat
source code repository.

---------------------
Kobo build of Top Hat:

This is different from the XCSoar build for the Kobo!

The KoboRoot.tgz installation file can be build can optionally the KOBO_UIMAGE=y flag
which will update the Kobo's operating system kernel to support both USB Host mode
and PC Connect mode.

To build the Kobo installation with the Kobo kernel update to support
both USB Host mode and PC Connect mode:

  make TARGET=KOBO output/KOBO/KoboRoot.tgz KOBO_UIMAGE=y

To build the Kobo installation that only updates Top Hat and does not
make any changes to the Kobo's kernel:

  make TARGET=KOBO output/KOBO/KoboRoot.tgz

The Kobo kernel source code is a submodule of the Top Hat Git repository and is
downloaded automatically by Git.  To update the Kobo kernel submodule, type:

  git submodule update

The toolchain for building the Kobo kernel (backup your machine first in case
you break something) can be installed on your Linux machine as follows:

------------
(The following is extracted from the more general document "kobo/uimage/README.md" in the
Top Hat repository.)

	## Kernel

	Unfortunately the Kobo Kernel must be compiled with an older version of GCC (4.4) and XCSoar requires 4.6 or later.  To resolve this you will need to build chains - one for the XCSoar packages and one for the kernel.

	### Setup

	* These steps have been followed on a new Ubuntu 12.04 Virtual Machine and on a Linux Mint 2.6.39-2-amd64.

	* Install codesourcery GCC toolchain to /usr/arm-none-linux-gnueabi by untaring the downloaded archive into the /usr (ditching the arm2010-blah directory) directory
	> wget http://sources.buildroot.net/arm-2010q1-202-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2

	> bunzip2 arm-2010q1-202-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2

	> sudo tar -C /usr -xvf arm-2010q1-202-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar --strip 1

	* Changed the ownership of this directory to me (chown ...)
	> sudo chown ${USER}:${USER} /usr/arm-none-linux-gnueabi

	* Install the uboot-mkimage tool
	> sudo apt-get install uboot-mkimage ncurses-dev

	### Setup - G++ - Hardware Float

	* Install g++ toolchain, this should go into /usr/arm-linux-gnueabihf directory

	> sudo apt-get install  gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

	* Change ownership so you don't need to compile as root (as above)

------------
Source code for Kobo uimage

Starting with Top Hat 1.3.4d_beta, we are no longer archiving the source code for the Kobo uImage
within the tarball file posted on the website.  However, it is included in the Top Hat git 
repository.  Also, if you want to download it directly, look at Top Hat version 1.3.4c_beta which
includes the current version of the uImage source in the tar archive.