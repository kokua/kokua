#!/bin/bash

## Here are some configuration options for Linux Client Testers.
## These options are for self-assisted troubleshooting during this beta
## testing phase; you should not usually need to touch them.

## - Switches the OpenAL sound effect driver off.
## That means: No sound effects (UI-, environment-, gesture-, uploaded-, prim- sounds).
#export LL_BAD_OPENAL_DRIVER=x

## NOTE: the FMOD audio driver isn't supported in Kokua, any "export LL_BAD_FMOD" have *no* effect.

## - Avoids the optional OpenGL extensions which have proven most problematic
##   on some hardware.  Disabling this option may cause BETTER PERFORMANCE but
##   may also cause CRASHES and hangs on some unstable combinations of drivers
##   and hardware.
## NOTE: This is now disabled by default.
#export LL_GL_BASICEXT=x

## - Avoids *all* optional OpenGL extensions.  This is the safest and least-
##   exciting option.  Enable this if you experience stability issues, and
##   report whether it helps in the Linux Client Testers forum.
#export LL_GL_NOEXT=x

## - For advanced troubleshooters, this lets you disable specific GL
##   extensions, each of which is represented by a letter a-o.  If you can
##   narrow down a stability problem on your system to just one or two
##   extensions then please post details of your hardware (and drivers) to
##   the Linux Client Testers forum along with the minimal
##   LL_GL_BLACKLIST which solves your problems.
#export LL_GL_BLACKLIST=abcdefghijklmno

## - Some ATI/Radeon users report random X server crashes when the mouse
##   cursor changes shape.  If you suspect that you are a victim of this
##   driver bug, try enabling this option and report whether it helps:
#export LL_ATI_MOUSE_CURSOR_BUG=x


## Everything below this line is just for advanced troubleshooters.
##-------------------------------------------------------------------

## - For advanced debugging cases, you can run the viewer under the
##   control of another program, such as strace, gdb, or valgrind.  If
##   you're building your own viewer, bear in mind that the executable
##   in the bin directory will be stripped: you should replace it with
##   an unstripped binary before you run.
#export LL_WRAPPER='gdb --args'
#export LL_WRAPPER='valgrind --smc-check=all --error-limit=no --log-file=secondlife.vg --leak-check=full --suppressions=/usr/lib/valgrind/glibc-2.5.supp --suppressions=secondlife-i686.supp'

## - Avoids an often-buggy X feature that doesn't really benefit us anyway.
export SDL_VIDEO_X11_DGAMOUSE=0

## - Works around a problem with misconfigured 64-bit systems not finding GL
export LIBGL_DRIVERS_PATH="${LIBGL_DRIVERS_PATH}":/usr/lib64/dri:/usr/lib32/dri:/usr/lib/dri

## - The 'scim' GTK IM module widely crashes the viewer.  Avoid it.
if [ "$GTK_IM_MODULE" = "scim" ]; then
    export GTK_IM_MODULE=xim
fi

## - Automatically work around the ATI mouse cursor crash bug:
## (this workaround is disabled as most fglrx users do not see the bug)
#if lsmod | grep fglrx &>/dev/null ; then
#	export LL_ATI_MOUSE_CURSOR_BUG=x
#fi


## Nothing worth editing below this line.
##-------------------------------------------------------------------

SCRIPTSRC=`readlink -f "$0" || echo "$0"`
RUN_PATH=`dirname "${SCRIPTSRC}" || echo .`
echo "Running from ${RUN_PATH}"
cd "${RUN_PATH}"

# Re-register  hop:// and secondlife:// protocol handler every launch, for now.
./etc/register_hopprotocol.sh
./etc/register_secondlifeprotocol.sh

# Re-register the application with the desktop system every launch, for now.
./etc/refresh_desktop_app_entry.sh

## Before we mess with LD_LIBRARY_PATH, save the old one to restore for
##  subprocesses that care.
export SAVED_LD_LIBRARY_PATH="${LD_LIBRARY_PATH}"

if [ -n "$LL_TCMALLOC" ]; then
    tcmalloc_libs='/usr/lib/libtcmalloc.so.0 /usr/lib/libstacktrace.so.0 /lib/libpthread.so.0'
    all=1
    for f in $tcmalloc_libs; do
        if [ ! -f $f ]; then
	    all=0
	fi
    done
    if [ $all != 1 ]; then
        echo 'Cannot use tcmalloc libraries: components missing' 1>&2
    else
	export LD_PRELOAD=$(echo $tcmalloc_libs | tr ' ' :)
	if [ -z "$HEAPCHECK" -a -z "$HEAPPROFILE" ]; then
	    export HEAPCHECK=${HEAPCHECK:-normal}
	fi
    fi
fi

BINARY_TYPE=$(expr match "$(file -b bin/do-not-directly-run-kokua-bin)" '\(.*executable\)')
if [ "${BINARY_TYPE}" == "ELF 32-bit LSB executable" ]; then

	export SL_ENV='LD_LIBRARY_PATH="`pwd`"/lib:"${LD_LIBRARY_PATH}"'
else
	export SL_ENV='LD_LIBRARY_PATH="`pwd`"/lib64:"`pwd`"/lib32:"${LD_LIBRARY_PATH}"'
fi

export SL_CMD='$LL_WRAPPER bin/do-not-directly-run-kokua-bin'
export SL_OPT="`cat etc/gridargs.dat` $@"


# Run the program
eval ${SL_ENV} ${SL_CMD} ${SL_OPT} || LL_RUN_ERR=runerr

# Handle any resulting errors
if [ -n "$LL_RUN_ERR" ]; then
	LL_RUN_ERR_MSG=""
	if [ "$LL_RUN_ERR" = "runerr" ]; then
		# generic error running the binary
		echo '*** Bad shutdown. ***'
	fi
fi
	

echo
echo '*******************************************************'
echo 'This is a ALPHA release of the Kokua Linux client.'
echo 'Thank you for testing!'
echo 'Please see README-linux.txt before reporting problems.'
echo
