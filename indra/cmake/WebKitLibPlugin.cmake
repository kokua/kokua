# -*- cmake -*-
include(Linking)
include(Prebuilt)

if (STANDALONE)
  set(WEBKITLIBPLUGIN OFF CACHE BOOL
      "WEBKITLIBPLUGIN support for the llplugin/llmedia test apps.")
else (STANDALONE)
  if(LINUX AND ${ARCH} STREQUAL "x86_64")
     use_prebuilt_binary(lindenqt) 		# HACK: *ORDER OF DOWNLOAD MATTERS*
						# lindenqt currently also contains a deprecated version
						# of llqtwebkit, that is overridden by the one downloaded by
						# "use_prebuilt_binary(llqtwebkit)", which is actually used.
						# TODO: unhack with the next update of Qt
     use_prebuilt_binary(llqtwebkit-templinux64hack)
  else(LINUX AND ${ARCH} STREQUAL "x86_64")
    use_prebuilt_binary(llqtwebkit)
  endif(LINUX AND ${ARCH} STREQUAL "x86_64")

  set(WEBKITLIBPLUGIN ON CACHE BOOL
      "WEBKITLIBPLUGIN support for the llplugin/llmedia test apps.")
endif (STANDALONE)

if (WINDOWS)
  set(WEBKIT_PLUGIN_LIBRARIES
      debug llqtwebkitd
      debug QtWebKitd4
      debug QtOpenGLd4
      debug QtNetworkd4
      debug QtGuid4
      debug QtCored4
      debug qtmaind
      optimized llqtwebkit
      optimized QtWebKit4
      optimized QtOpenGL4
      optimized QtNetwork4
      optimized QtGui4
      optimized QtCore4
      optimized qtmain
      )
elseif (DARWIN)
  set(WEBKIT_PLUGIN_LIBRARIES
      optimized ${ARCH_PREBUILT_DIRS_RELEASE}/libllqtwebkit.dylib
      debug ${ARCH_PREBUILT_DIRS_RELEASE}/libllqtwebkit.dylib
      )
elseif (LINUX)
  if (STANDALONE) 
    set(WEBKIT_PLUGIN_LIBRARIES llqtwebkit)
  else (STANDALONE)
    set(WEBKIT_PLUGIN_LIBRARIES
        llqtwebkit

        qgif
        qjpeg

        QtWebKit
        QtOpenGL
        QtNetwork
        QtGui
        QtCore

        jpeg
        fontconfig
        X11
        Xrender
        GL
        )
  endif (STANDALONE)
endif (WINDOWS)
