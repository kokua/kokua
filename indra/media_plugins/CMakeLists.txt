# -*- cmake -*-

add_subdirectory(base)

add_subdirectory(webkit)

if (NOT DARWIN)
   add_subdirectory(gstreamer010)
endif (NOT DARWIN)

if (WINDOWS)
    add_subdirectory(quicktime)
endif (WINDOWS)

if (WINDOWS)
    add_subdirectory(winmmshim)
endif (WINDOWS)

add_subdirectory(example)
