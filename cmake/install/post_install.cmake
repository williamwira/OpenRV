#
# Copyright (C) 2022  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# This module can be used to verify the contents of the installation folder. as a mean to double check previous operations.
#
# The first part replicates to some extents what's being done in the 'pre_install.cmake' script.
#

SET(QT_GPL_COMPONENTS
    charts
    datavisualization
    designer
    linguist
    designercomponent
    designerconfig
    networkauthorization
    networkauth
    virtualkeyboard
    quickwebgl
    webgl
    platforminputcontext
    design
)

IF(CMAKE_INSTALL_PREFIX)
  #
  # Create a list of all installed files
  FILE(
    GLOB_RECURSE _all_installed_files
    LIST_DIRECTORIES FALSE
    RELATIVE ${CMAKE_INSTALL_PREFIX}
    ${CMAKE_INSTALL_PREFIX}/*
  )

  #
  # Loop through all components in all all installed files
  MESSAGE(STATUS "Double checking installed files...")
  FOREACH(
    QT_GPL_COMPONENT
    ${QT_GPL_COMPONENTS}
  )
    FOREACH(
      _file
      ${_all_installed_files}
    )
      STRING(TOLOWER "${_file}" _file_lower)
      IF(APPLE)
        # MacOS: Split paths with "_codesignature" folder, as that folder's string yielded a false positive match for the design component in QT_GPL_COMPONENTS.
        STRING(
          REGEX
          REPLACE "_codesignature.*" "" _file_lower "${_file_lower}"
        )
      ENDIF()
      IF("${_file_lower}" MATCHES "${QT_GPL_COMPONENT}")
        MESSAGE(FATAL_ERROR "Was not expecting the installation of the following file: '${_file}', which matches Qt component '${QT_GPL_COMPONENT}'")
      ENDIF()
    ENDFOREACH()
  ENDFOREACH()
ELSE()
  MESSAGE(FATAL_ERROR "An install prefix was not set: '${CMAKE_INSTALL_PREFIX}'")
ENDIF()
