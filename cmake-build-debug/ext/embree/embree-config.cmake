## Copyright 2009-2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

# use default install config
INCLUDE(${CMAKE_CURRENT_LIST_DIR}/embree-config-install.cmake)

# and override path variables to match for build directory
SET(EMBREE_INCLUDE_DIRS /Users/sze/Desktop/CS5625/Starter22/ext/embree/include)
SET(EMBREE_LIBRARY /Users/sze/Desktop/CS5625/Starter22/cmake-build-debug/ext/embree/libembree3.3.13.2.dylib)
SET(EMBREE_LIBRARIES ${EMBREE_LIBRARY})
