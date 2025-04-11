

####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was SlangConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

if (NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  include("${CMAKE_CURRENT_LIST_DIR}/slangTargets.cmake")
  check_required_components("slang")
endif()

if(ON)

  find_program(SLANGC_EXECUTABLE "slangc" HINTS ENV PATH "${PACKAGE_PREFIX_DIR}/bin")

  if (NOT SLANGC_EXECUTABLE)
      message(STATUS "slangc executable not found; ensure it is available in your PATH.")
  endif()
    
  set(SLANG_EXECUTABLE ${SLANGC_EXECUTABLE} CACHE STRING "Path to the slangc executable")

endif()

