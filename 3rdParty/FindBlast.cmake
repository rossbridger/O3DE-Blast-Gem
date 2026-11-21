# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root
# of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

set(MY_NAME "Blast")
set(TARGET_WITH_NAMESPACE "3rdParty::${MY_NAME}")
if (TARGET ${TARGET_WITH_NAMESPACE})
    return()
endif()

set(_PACKAGE_DIR ${CMAKE_CURRENT_LIST_DIR}/Blast)
# The configuration is only known at generation time, so pick the library folder with a generator expression.
# Profile builds use the release libraries.
set(${MY_NAME}_LIBRARY_DIR ${_PACKAGE_DIR}/$<IF:$<CONFIG:debug>,debug,release>/bin)

set(${MY_NAME}_INCLUDE_DIR ${_PACKAGE_DIR}/include/extensions/assetutils
                           ${_PACKAGE_DIR}/include/extensions/authoring
                           ${_PACKAGE_DIR}/include/extensions/authoringCommon
                           ${_PACKAGE_DIR}/include/extensions/serialization
                           ${_PACKAGE_DIR}/include/extensions/shaders
                           ${_PACKAGE_DIR}/include/extensions/stress
                           ${_PACKAGE_DIR}/include/globals
                           ${_PACKAGE_DIR}/include/lowlevel
                           ${_PACKAGE_DIR}/include/shared/NvFoundation
                           ${_PACKAGE_DIR}/include/toolkit)

set(IMPORTED_BLAST_LIBS_SUFFIX
    NvBlastExtAssetUtils
    NvBlastExtAuthoring
    NvBlastExtSerialization
    NvBlastExtShaders
    NvBlastExtStress
    NvBlastExtTkSerialization
    NvBlastGlobals
    NvBlast
    NvBlastTk
)

set(${MY_NAME}_LIBRARIES "")
set(${MY_NAME}_RUNTIME_DEPENDENCIES "")
foreach(BLAST_LIB ${IMPORTED_BLAST_LIBS_SUFFIX})
    set(_shared_lib ${${MY_NAME}_LIBRARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${BLAST_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
    # Platforms with import libraries (Windows) link against the .lib and load the .dll at runtime.
    if(CMAKE_IMPORT_LIBRARY_SUFFIX)
        list(APPEND ${MY_NAME}_LIBRARIES
            ${${MY_NAME}_LIBRARY_DIR}/${CMAKE_IMPORT_LIBRARY_PREFIX}${BLAST_LIB}${CMAKE_IMPORT_LIBRARY_SUFFIX})
    else()
        list(APPEND ${MY_NAME}_LIBRARIES ${_shared_lib})
    endif()
    list(APPEND ${MY_NAME}_RUNTIME_DEPENDENCIES ${_shared_lib})
endforeach()

add_library(${TARGET_WITH_NAMESPACE} INTERFACE IMPORTED GLOBAL)
ly_target_include_system_directories(TARGET ${TARGET_WITH_NAMESPACE} INTERFACE ${${MY_NAME}_INCLUDE_DIR})
target_link_libraries(${TARGET_WITH_NAMESPACE} INTERFACE ${${MY_NAME}_LIBRARIES})
target_compile_definitions(${TARGET_WITH_NAMESPACE} INTERFACE ${${MY_NAME}_COMPILE_DEFINITIONS})

# Copy the shared libraries next to the executables that load the gem.
ly_add_target_files(TARGETS ${TARGET_WITH_NAMESPACE} FILES ${${MY_NAME}_RUNTIME_DEPENDENCIES})

    # install the libraries making sure to use different directories for debug/release/etc
set(BASE_LIBRARY_FOLDER "lib/${PAL_PLATFORM_NAME}")
foreach(conf IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER ${conf} UCONF)
    string(TOLOWER ${conf} LCONF)
    if(LCONF STREQUAL "debug")
        set(_conf_dir debug)
    else()
        set(_conf_dir release)
    endif()
    set(_conf_libraries "")
    foreach(BLAST_LIB ${IMPORTED_BLAST_LIBS_SUFFIX})
        list(APPEND _conf_libraries ${_PACKAGE_DIR}/${_conf_dir}/bin/${CMAKE_SHARED_LIBRARY_PREFIX}${BLAST_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endforeach()
    ly_install(FILES ${_conf_libraries}
            DESTINATION "${BASE_LIBRARY_FOLDER}/${conf}"
            COMPONENT ${LY_INSTALL_PERMUTATION_COMPONENT}_${UCONF}
            CONFIGURATIONS ${conf}
    )
endforeach()

set(${MY_NAME}_FOUND True)
