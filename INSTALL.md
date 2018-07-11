
Please read the files (especially README.Makefiles) in the
husky-common (huskybse) package FIRST!

Instruction to build smapi
-----------------------------

1. Put the smapi package in the directory where the other packages of fido
   husky reside:

   unix, beos, possible cygwin:
   -------------------
   /usr/src/packages/        -> huskybse/
                             -> huskylib/
                             -> smapi/
                             -> fidoconfig/
                             -> hpt/
                             -> htick/
                             ...some other

   windows, dos, os/2 & etc:
   -------------------------
   d:\husky\                 -> huskylib\
                             -> smapi\
                             -> fidoconf\
                             -> hpt\
                             -> htick\
                             ...some other

2. Determine which makefile you need to use then compile (and install if need)
   smapi using selected makefile, as following examples:
   Example 1 (use common makefile (named 'Makefile'): require huskymak.cfg (see
              huskybse); unixes only):
	$ make
	$ make install
   Example 2 (use platfom-specific makefile):
   unix:
	$ make -f makefile.lnx
	$ make -f makefile.lnx install
   dos:
        d:\husky\smapi>make -f makefile.djg

2.1. Compile with cmake:

  $ mkdir build
  $ cd build
  $ cmake ..
  $ cmake --build .
  $ make install

  make distrib rpm,deb,tgz:
    cpack -G RPM, cpack -G DEB, cpack -G TGZ

3. (For UNIXes only) Ensure /usr/local/lib/ is in /etc/ld.so.conf

4. (For UNIXes only) Execute ldconfig as root

You're ready. Now you can install software which uses smapi.

$Id$
