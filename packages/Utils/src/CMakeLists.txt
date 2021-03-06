INCLUDE(TribitsLibraryMacros)

#
# A) Package-specific configuration options
#

TRIBITS_CONFIGURE_FILE(${PACKAGE_NAME}_config.hpp)

#
# B) Define the header and source files (and directories)
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.hpp
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

APPEND_SET(HEADERS
  DTK_CommIndexer.hpp
  DTK_CommTools.hpp
  DTK_DBC.hpp
  DTK_PredicateComposition.hpp
  DTK_PredicateComposition_impl.hpp
  DTK_SearchTreeFactory.hpp
  DTK_StaticSearchTree.hpp
  DTK_StaticSearchTree_impl.hpp
  ) 

APPEND_SET(SOURCES
  DTK_CommIndexer.cpp
  DTK_CommTools.cpp
  DTK_DBC.cpp
  DTK_SearchTreeFactory.cpp
  )

#
# C) Define the targets for package's library(s)
#

TRIBITS_ADD_LIBRARY(
  dtk_utils
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )
