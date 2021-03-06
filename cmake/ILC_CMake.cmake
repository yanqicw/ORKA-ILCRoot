# -*- mode: cmake -*-

# IlcRoot Build System Utility Macro and Function definitions
#
# Author: Johny Jose (johny.jose@cern.ch)
#         Port of previous Makefile build to cmake

cmake_minimum_required(VERSION 2.8.4 FATAL_ERROR)

macro(ILC_DevFlagsOutput)

  if(ILCDEV STREQUAL "YES")
    message(STATUS "FFLAGS    : ${FFLAGS}")
    message(STATUS "CXXFLAGS  : ${CXXFLAGS}")
    message(STATUS "CFLAGS    : ${CFLAGS}")
    message(STATUS "SOFLAGS   : ${SOFLAGS}")
    message(STATUS "LDFLAGS   : ${LDFLAGS}")
    message(STATUS "SHLIB     : ${SHLIB}")
    message(STATUS "SYSLIBS   : ${SYSLIBS}")
    message(STATUS "CINTFLAGS : ${CINTFLAGS}")
  endif(ILCDEV STREQUAL "YES")

endmacro(ILC_DevFlagsOutput)

function(ILC_CleanOutput _output input)

  string(REGEX REPLACE "\n" " " clean "${input}")
  set(${_output} ${clean} PARENT_SCOPE)

endfunction(ILC_CleanOutput)

function(ILC_CorrectPaths _output value )

  set(corrected)
  foreach(path ${value})
    set(external)
    string(REGEX MATCH "^/" external "${path}")
    if(NOT external)
      list(APPEND corrected "${CMAKE_SOURCE_DIR}/${path}" )
    else()
      list(APPEND corrected "${path}")
    endif(NOT external)
  endforeach(path)
  set(${_output} ${corrected} PARENT_SCOPE)

endfunction(ILC_CorrectPaths)

function(ILC_Format _output prefix suffix input)

# DevNotes - input should be put in quotes or the complete list does not get passed to the function
  set(format)
  foreach(arg ${input})
    set(item ${arg})
    if(prefix)
      string(REGEX MATCH "^${prefix}" pre "${arg}")
    endif(prefix)
    if(suffix)
      string(REGEX MATCH "${suffix}$" suf "${arg}")
    endif(suffix)
    if(NOT pre)
      set(item "${prefix}${item}")
    endif(NOT pre)
    if(NOT suf)
      set(item "${item}${suffix}")
    endif(NOT suf)
    list(APPEND format ${item})
  endforeach(arg)
  set(${_output} ${format} PARENT_SCOPE)

endfunction(ILC_Format)

function(ILC_CheckLibraries _output input)

  set(format)
  foreach(arg ${input})
    set(item ${arg})
    string(REGEX MATCH "^(/|-)" preformatted "${item}")
    if(NOT preformatted)
      set(item "-l${item}")
    endif(NOT preformatted)
    list(APPEND format ${item})
  endforeach(arg)
  set(${_output} ${format} PARENT_SCOPE)
  
endfunction(ILC_CheckLibraries)

function(ILC_RootConfig _output parameters)

  if(ROOT_CONFIG)
    execute_process(COMMAND ${ROOT_CONFIG} ${parameters} OUTPUT_VARIABLE result ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
      message(FATAL_ERROR "root-config ERROR : ${error}")
    endif(error)
    ILC_CleanOutput(result "${result}")
    set(${_output} "${result}" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "root-config not found")
  endif(ROOT_CONFIG)

endfunction(ILC_RootConfig)

macro(ILC_CheckOutOfSourceBuild)
  
  #Check if previous in-source build failed
  if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeCache.txt OR EXISTS ${CMAKE_SOURCE_DIR}/CMakeFiles)
    message(FATAL_ERROR "CMakeCache.txt or CMakeFiles exists in source directory! Please remove them before running cmake $ILC_ROOT")
  endif(EXISTS ${CMAKE_SOURCE_DIR}/CMakeCache.txt OR EXISTS ${CMAKE_SOURCE_DIR}/CMakeFiles)
  
  #Get Real Paths of the source and binary directories
  get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
  get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)
  
  #Check for in-source builds
  if(${srcdir} STREQUAL ${bindir})
    message(FATAL_ERROR "IlcRoot cannot be built in-source! Please run cmake $ILC_ROOT outside the source directory")
  endif(${srcdir} STREQUAL ${bindir})

endmacro(ILC_CheckOutOfSourceBuild)

macro(ILC_ConfigureCompiler)

  if(ILCPROFILE STREQUAL "YES")
    set(ILC_TARGET ${ILC_TARGET}PROF)
    set(FFLAGS "-pg ${FFLAGS}")
    set(CXXFLAGS "-pg ${CXXFLAGS}")
    set(CFLAGS "-pg ${CFLAGS}")
    set(SOFLAGS "-pg ${SOFLAGS}")
    set(LDFLAGS "-pg ${LDFLAGS}")
  endif(ILCPROFILE STREQUAL "YES")

  ILC_RootConfig(RCFLAGS --auxcflags)
  ILC_RootConfig(RLFLAGS --ldflags)

  set(CXXFLAGS "${CXXFLAGS} ${RCFLAGS}")
  set(CFLAGS  "${CFLAGS} ${RCFLAGS}")
  set(SOFLAGS "${SOFLAGS} ${RLFLAGS}")
  set(LDFLAGS "${LDFLAGS} ${RLFLAGS}")

  if(NOT ILCCXXWARN STREQUAL "NO")
    set(CXXFLAGS "${CXXFLAGS} ${CXXWARN}")
  endif(NOT ILCCXXWARN STREQUAL "NO")

  set(CXXFLAGS "${DATEFLAGS} ${CXXFLAGS}")
  set(CFLAGS "${DATEFLAGS} ${CFLAGS}")
  set(CINTFLAGS "${DATEFLAGS}")

endmacro(ILC_ConfigureCompiler)

macro(ILC_ConfigurePlatform)
  
  ILC_GetTarget()
  include("CMake${ILC_TARGET}")
  
endmacro(ILC_ConfigurePlatform)

macro(ILC_GetTarget)
  
  #Set ILC_TARGET if it has not been set yet
  if(NOT ILC_TARGET)  
    set(ILC_TARGET $ENV{ILC_TARGET})
    #If ILC_TARGET is not defined assign default values
    if(NOT ILC_TARGET)
      message(WARNING "Environment variable ILC_TARGET is not set ! Setting to default value.")
      ILC_RootConfig(ILC_TARGET --arch)
      message("ILC_TARGET set to ${ILC_TARGET}")
    endif(NOT ILC_TARGET)
  endif(NOT ILC_TARGET)

endmacro(ILC_GetTarget) 

macro(ILC_ResetPackage)

  set(MODDIR ${MODULE})
  set(MODDIRI ${PROJECT_SOURCE_DIR}/${MODULE})

  set(SRCS)
  set(HDRS)
  set(FSRCS)
  set(DHDR)
  set(CSRCS)
  set(CHDRS)
  set(EINCLUDE)
  set(EDEFINE)
  set(ELIBS)
  set(ELIBSDIR)
  set(PACKFFLAGS)
  set(PACKCXXFLAGS)
  set(PACKCFLAGS)
  set(PACKDYFLAGS)
  set(PACKSOFLAGS)
  set(PACKLDFLAGS)
  set(PACKBLIBS)
  set(EXPORT)
  set(EHDRS)
  set(CINTHDRS)
  set(CINTAUTOLINK)
  set(ARLIBS)
  set(SHLIBS)

  #HLT Variables
  set(MODULE_HDRS)
  set(MODULE_SRCS)
  set(MODULE_DHDR)
  set(CLASS_HDRS)
  set(LIBRARY_DEP)
  set(HLTDEFS)
  set(HLTSOFLAGS) 


  set(PSRCS)
  set(PHDRS)
  set(PFSRCS)
  set(PDHDR)
  set(PCSRCS)
  set(PCHDRS)
  set(PEINCLUDE)
  set(PEDEFINE)
  set(PELIBS)
  set(PELIBSDIR)
  set(PPACKFFLAGS)
  set(PPACKCXXFLAGS)
  set(PPACKCFLAGS)
  set(PPACKDYFLAGS)
  set(PPACKSOFLAGS)
  set(PPACKLDFLAGS)
  set(PPACKBLIBS)
  set(PEXPORT)
  set(PEHDRS)
  set(PCINTHDRS)
  set(PCINTAUTOLINK)
  set(PARLIBS)
  set(PSHLIBS)
  set(PDS)

endmacro(ILC_ResetPackage)

function(ILC_SetPackageVariable _var ext setvalue unsetvalue )

#DevNotes - setvalue and unsetcalue should both be put in quotes or cmake throws an error if they have undefined values
  if(setvalue)
    set(${_var} ${setvalue} PARENT_SCOPE)
    set(${PACKAGE}${ext} ${setvalue} PARENT_SCOPE)
  else()
    set(${_var} ${unsetvalue} PARENT_SCOPE)
    set(${PACKAGE}${ext} ${unsetvalue} PARENT_SCOPE)
  endif(setvalue)

endfunction(ILC_SetPackageVariable)

macro(ILC_BuildPackage)
 
  list(APPEND EINCLUDE STEER) 

  ILC_SetPackageVariable(PFFLAGS "FFLAGS" "${PACKFFLAGS}" "${FFLAGS}")
  ILC_SetPackageVariable(PCFLAGS "CFLAGS" "${PACKCFLAGS}" "${CFLAGS}")
  ILC_SetPackageVariable(PCXXFLAGS "CXXFLAGS" "${PACKCXXFLAGS}" "${CXXFLAGS}")
  ILC_SetPackageVariable(PSOFLAGS "SOFLAGS" "${PACKSOFLAGS}" "${SOFLAGS}")
  ILC_SetPackageVariable(PLDFLAGS "LDFLAGS" "${PACKLDFLAGS}" "${LDFLAGS}")
  ILC_SetPackageVariable(PDCXXFLAGS "DCXXFLAGS" "${PACKDCXXFLAGS}" "${CXXFLAGSNO}")
  ILC_SetPackageVariable(PBLIBS "BLIBS" "${PACKBLIBS}" "${LIBS}")

  set(WITHDICT)
  if(DHDR OR CINTAUTOLINK)
    set(WITHDICT TRUE)
  endif(DHDR OR CINTAUTOLINK)

  ILC_SetPackageVariable(PEDEFINE "EDEFINE" "${EDEFINE}" "${EDEFINE}")
  ILC_SetPackageVariable(PEXPORT "EXPORT" "${EXPORT}" "${EXPORT}")
#  message(${CMAKE_INCLUDE_EXPORT_DIRECTORY})
  ILC_SetPackageVariable(PEXPORTDEST "EXPORTDEST" "${CMAKE_INCLUDE_EXPORT_DIRECTORY}" "${CMAKE_INCLUDE_EXPORT_DIRECTORY}")
  ILC_SetPackageVariable(PINC "INC" "${EINCLUDE};${MODULE}" "${EINCLUDE};${MODULE}")
  ILC_SetPackageVariable(PELIBS "ELIBS" "${ELIBS}" "${ELIBS}")
  ILC_SetPackageVariable(PELIBSDIR "ELIBSDIR" "${ELIBSDIR}" "${ELIBSDIR}")
  
  ILC_SetPackageVariable(PCS "CS" "${CSRCS}" "${CSRCS}")
  ILC_SetPackageVariable(PCH "CH" "${CHDRS}" "${CHDRS}")
  
  ILC_SetPackageVariable(PFS "FS" "${FSRCS}" "${FSRCS}")

  ILC_SetPackageVariable(PS "S" "${SRCS}" "${SRCS}")
  ILC_SetPackageVariable(PH "H" "${HDRS} ${EHDRS}" "${HDRS} ${EHDRS}")
  ILC_SetPackageVariable(PCINTHDRS "CINTHDRS" "${CINTHDRS}" "${PH}")
  
  string( REPLACE ".h" "" PCINTCLASSES "${PCINTHDRS}")
  set ( ${PACKAGE}CINTCLASSES ${PCINTCLASSES})

  ILC_SetPackageVariable(PDH "DH" "${DHDR}" "")

  if(CINTAUTOLINK)
    ILC_SetPackageVariable(PDAL "DAL" "${CMAKE_CURRENT_BINARY_DIR}/G__${PACKAGE}AutoLinkDef.h" "${CMAKE_CURRENT_BINARY_DIR}/G__${PACKAGE}AutoLinkDef.h")
    ILC_SetPackageVariable(PDH "DH" "${PDAL}" "${PDAL}")
    ILC_GenerateLinkDef()
  endif(CINTAUTOLINK)

  if(lib)
    list(APPEND PMLIBS ${PACKAGE})
    list(APPEND ALLLIBS ${PACKAGE})
    list(APPEND PMALIBS ${PACKAGE}-static)
    list(APPEND ALLALIBS ${PACKAGE}-static)
    list(APPEND BINLIBS ${PACKAGE})
  else()
    list(APPEND ALLEXECS ${PACKAGE})
  endif(lib)
  set(${MODULE}ALIBS "${PMALIBS}" PARENT_SCOPE)
  set(${MODULE}LIBS "${PMLIBS}" PARENT_SCOPE)
  set(${MODULE}INC "${EINCLUDE}" PARENT_SCOPE)
  list(APPEND INCLUDEFILES ${PEXPORTDEST})
  if(WITHDICT)  
    ILC_SetPackageVariable(PDS "DS" "G__${PACKAGE}.cxx" "G__${PACKAGE}.cxx")
    ILC_GenerateDictionary()
  else()
    if(lib)
      message(STATUS "No dictionary generated for ${PACKAGE}")  
    endif(lib)
  endif(WITHDICT)
  if(PS OR PCS OR PFS)
    if(lib)
      ILC_BuildLibrary()
    elseif(bin)
      ILC_BuildExecutable()
    endif(lib)
  endif(PS OR PCS OR PFS)
  ILC_CopyHeaders()
  ILC_BuildPAR()

endmacro(ILC_BuildPackage)


macro(ILC_BuildModule)

  add_definitions(-D_MODULE_="${MODULE}")
  foreach(PACKAGEFILE ${PACKAGES})
      set(lib)
      set(bin)
      string(REGEX MATCH "CMakelib" lib "${PACKAGEFILE}")
      string(REGEX MATCH "CMakebin" bin "${PACKAGEFILE}")
      get_filename_component(PACKAGE ${PACKAGEFILE} NAME)
      string(REGEX REPLACE "^CMake(lib|bin)(.*)\\.pkg" "\\2" PACKAGE "${PACKAGE}")
      if(ILCDEV)
        message("Adding package ${PACKAGE} in ${MODULE}")
      endif(ILCDEV)
      ILC_ResetPackage()
      include(${PACKAGEFILE})
      ILC_BuildPackage()
     
      get_property(EXCLUDEPACKAGE TARGET ${PACKAGE} PROPERTY EXCLUDE_FROM_ALL) 
      if(NOT EXCLUDEPACKAGE)
        install(TARGETS ${PACKAGE} 
                RUNTIME DESTINATION bin/tgt_${ILC_TARGET}
                LIBRARY DESTINATION lib/tgt_${ILC_TARGET}
                ARCHIVE DESTINATION bin/tgt_${ILC_TARGET})
      endif(NOT EXCLUDEPACKAGE)
  endforeach(PACKAGEFILE)
  ILC_CheckModule()
endmacro(ILC_BuildModule)

macro(ILC_GenerateDictionary)
 
  if(ILCDEV STREQUAL "YES")
    message("Generating Dictionary rule for ${PACKAGE}")
    message("${ROOTCINT} -f ${PDS} -c ${PEDEFINE} ${CINTFLAGS} ${PINC} ${PCINTHDRS} ${PDH}")
  endif(ILCDEV STREQUAL "YES")
 
  # Split up all arguments

  set(DEDEFINE ${PEDEFINE})
  set(DCINTFLAGS ${CINTFLAGS})
  set(DINC ${PINC})
  set(DCINTHDRS ${PCINTHDRS})
  set(DDH ${PDH})
  separate_arguments(DEDEFINE)
  separate_arguments(DCINTFLAGS)
  separate_arguments(DINC)
  separate_arguments(DCINTHDRS)
  separate_arguments(DDH)
  # Format neccesary arguments
  ILC_Format(DINC "-I" "" "${DINC};${CMAKE_INCLUDE_EXPORT_DIRECTORY}")
  set_source_files_properties(${PDS} PROPERTIES GENERATED TRUE)
  set_source_files_properties(${PDS} PROPERTIES COMPILE_FLAGS "-w")
  add_custom_command(OUTPUT  ${PDS}
                     COMMAND cd ${PROJECT_SOURCE_DIR} && ${ROOTCINT} -f ${CMAKE_CURRENT_BINARY_DIR}/${PDS} -c -D_MODULE=\\\"${MODULE}\\\" ${DEDEFINE} ${DCINTFLAGS} ${DINC} ${DCINTHDRS} ${DDH}
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                     DEPENDS ${DCINTHDRS} ${DDH}
                     )
  add_custom_target(G${PACKAGE})
  add_dependencies(G${PACKAGE} ${PDS})

endmacro(ILC_GenerateDictionary)

macro(ILC_BuildLibrary)

  ILC_DevFlagsOutput()
  set(CMAKE_CXX_FLAGS "${PEDEFINE} ${PCXXFLAGS}")
  set(CMAKE_C_FLAGS "${PEDEFINE} ${PCFLAGS}")
  set(CMAKE_Fortran_FLAGS "${PEDEFINE} ${PFFLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS ${PSOFLAGS}) 
  set(CMAKE_MODULE_LINKER_FLAGS ${PLDFLAGS})
  
  
  separate_arguments(PINC)
  separate_arguments(EINCLUDE)
  separate_arguments(PELIBSDIR)
  separate_arguments(PBLIBS)
  separate_arguments(PELIBS)
  separate_arguments(SHLIB)

  ILC_Format(PELIBSDIR "-L" "" "${PELIBSDIR}")
  ILC_CheckLibraries(PBLIBS "${PBLIBS}")

  ILC_CorrectPaths(EINCLUDE "${EINCLUDE}")
  ILC_CorrectPaths(PINC "${PINC}")

  include_directories(SYSTEM ${ROOTINCDIR})
  include_directories(${PINC})  
  include_directories(${EINCLUDE})
  include_directories(${CMAKE_INCLUDE_EXPORT_DIRECTORY})
  
  add_library(${PACKAGE} SHARED ${PCS} ${PFS} ${PS} ${PDS})
  set_target_properties(${PACKAGE} PROPERTIES SUFFIX .so)  
  

  if(PELIBS OR SHLIB)
    target_link_libraries(${PACKAGE} ${PELIBSDIR} ${PELIBS} ${SHLIB})
  endif(PELIBS OR SHLIB)
  
  add_dependencies(${MODULE}-all ${PACKAGE})
     
  add_library(${PACKAGE}-static STATIC EXCLUDE_FROM_ALL ${PCS} ${PFS} ${PS} ${PDS})
  set_target_properties(${PACKAGE}-static PROPERTIES OUTPUT_NAME ${PACKAGE})
  if(PELIBS OR ALLIB)
    target_link_libraries(${PACKAGE}-static ${PELIBSDIR} ${PELIBS} ${ALLIB})
  endif(PELIBS OR ALLIB)

  add_dependencies(${MODULE}-all-static ${PACKAGE}-static)
  if(ILCPROFILE STREQUAL "YES")
    add_dependencies(${MODULE}-all ${MODULE}-static)
  endif(ILCPROFILE STREQUAL "YES")

  list(FIND EXCLUDEMODULES ${MODULE} RESULT)
  if(NOT RESULT STREQUAL "-1")
    message(STATUS "${MODULE} will not be built by default. Type make ${MODULE}-all to build.")
    set_property(TARGET ${PACKAGE} PROPERTY EXCLUDE_FROM_ALL TRUE)
  endif(NOT RESULT STREQUAL "-1")

  set_target_properties(${PACKAGE} PROPERTIES INSTALL_NAME_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

endmacro(ILC_BuildLibrary)

macro(ILC_BuildExecutable)

  ILC_DevFlagsOutput()
  set(CMAKE_CXX_FLAGS "${PEDEFINE} ${PCXXFLAGS} ${EXEFLAGS}")
  set(CMAKE_C_FLAGS "${PEDEFINE} ${PCFLAGS} ${EXEFLAGS}")
  set(CMAKE_Fortran_FLAGS "${PEDEFINE} ${PFFLAGS} ${EXEFLAGS}")
  set(CMAKE_SHARED_LINKER_FLAGS ${PSOFLAGS}) 
  set(CMAKE_MODULE_LINKER_FLAGS ${PLDFLAGS})
  
  separate_arguments(PINC)
  separate_arguments(EINCLUDE)
  separate_arguments(PELIBSDIR)
  separate_arguments(PBLIBS)
  separate_arguments(PELIBS)
  separate_arguments(SHLIB)
  
  ILC_Format(PELIBSDIR "-L" "" "${PELIBSDIR}")
  ILC_CheckLibraries(PBLIBS "${PBLIBS}")

  ILC_CorrectPaths(EINCLUDE "${EINCLUDE}")
  ILC_CorrectPaths(PINC "${PINC}")

  include_directories(SYSTEM ${ROOTINCDIR})
  include_directories(${PINC})  
  include_directories(${EINCLUDE})
  include_directories(${CMAKE_INCLUDE_EXPORT_DIRECTORY})

  if(ILCPROFILE STREQUAL "YES")
    add_executable(${PACKAGE} ${PFS} ${PCS} ${PS} ${PDS})
    if(ARLIBS OR SHLIBS OR PBLIBS OR EXEFLAGS)
      target_link_libraries(${PACKAGE} ${ARLIBS} ${SHLIBS} ${PBLIBS} ${EXEFLAGS})
    endif(ARLIBS OR SHLIBS OR PBLIBS OR EXEFLAGS)
    add_dependencies(${MODULE}-all ${PACKAGE})
  else()
    add_executable(${PACKAGE} ${PFS} ${PCS} ${PS} ${PDS})
    if(PELIBS OR PBLIBS OR EXEFLAGS)
      target_link_libraries(${PACKAGE} ${BINLIBDIRS} ${PELIBSDIR} ${PELIBS} ${PBLIBS} ${EXEFLAGS})
    endif(PELIBS OR PBLIBS OR EXEFLAGS)
    add_dependencies(${MODULE}-all ${PACKAGE})
  endif(ILCPROFILE STREQUAL "YES")

# IlcMDC
# ------------------------------
  if(PACKAGE STREQUAL "ilcmdc" AND GENERATEPACKAGES)

    add_executable(${PACKAGE}-static ${PFS} ${PCS} ${PS} ${PDS})
    set_target_properties(${PACKAGE}-static PROPERTIES OUTPUT_NAME ilcmdca)
    add_custom_target( libIlcMDC
                        COMMAND rm -rf ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libIlcMDC.a
			COMMAND rm -rf junkmdc
			COMMAND mkdir junkmdc && cd junkmdc && ar x ../libRAWDatabase.a && ar x ../libMDC.a && ar x ../libESD.a && ar x ../libSTEERBase.a && ar r ../libIlcMDC.a *.o && cd .. && rm -rf junkmdc
			WORKING_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    add_custom_target( root-static-libs 
                       COMMAND make static
		       WORKING_DIRECTORY ${ROOTSYS} )
		       
    add_dependencies(libIlcMDC RAWDatabase-static STEERBase-static MDC-static ESD-static)
    add_dependencies(${PACKAGE}-static libIlcMDC root-static-libs)
    target_link_libraries(${PACKAGE}-static ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libIlcMDC.a ${ROOTLIBDIR}/libRoot.a ${ROOTLIBDIR}/libfreetype.a ${ROOTLIBDIR}/libpcre.a -pthread -ldl -lcurses)

    install(TARGETS ${PACKAGE}-static RUNTIME DESTINATION bin/tgt_${ILC_TARGET} COMPONENT MDC)
    install(FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libIlcMDC.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libIlcMDC.a ${ROOTLIBDIR}/libRoot.a ${ROOTLIBDIR}/libfreetype.a ${ROOTLIBDIR}/libpcre.a DESTINATION lib/tgt_${ILC_TARGET} COMPONENT MDC)
    install(FILES ${PROJECT_SOURCE_DIR}/RAW/mdc.h DESTINATION include COMPONENT MDC)
    set(CPACK_RPM_PACKAGE_SUMMARY "IlcMDC static libraries")
    set(CPACK_RPM_PACKAGE_NAME "ilcmdc")
    set(CPACK_RPM_PACKAGE_VERSION "${ILCMDCVERSION}")
    set(CPACK_RPM_PACKAGE_RELEASE "${ILCMDCRELEASE}")
    set(CPACK_RPM_PACKAGE_LICENSE "License: CERN ILC Off-line Group")
    set(CPACK_RPM_PACKAGE_VENDOR "ILC Core Off-line Group")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IlcMDC static libraries")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Ilc")
    set(CPACK_RPM_PACKAGE_REQUIRES "glibc")
    set(CPACK_PACKAGE_VERSION "1.0.0")
    set(CPACK_PACKAGE_VERSION_MAJOR "1")
    set(CPACK_PACKAGE_VERSION_MINOR "0")
    set(CPACK_PACKAGE_VERSION_PATCH "0")
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/")
    add_component_package(MDC ilcmdc-rpm)

  endif(PACKAGE STREQUAL "ilcmdc" AND GENERATEPACKAGES)

  list(FIND EXCLUDEMODULES ${MODULE} RESULT)
  if(NOT RESULT STREQUAL "-1")
    set_property(TARGET ${PACKAGE} PROPERTY EXCLUDE_FROM_ALL TRUE)
  endif(NOT RESULT STREQUAL "-1")

endmacro(ILC_BuildExecutable)

macro(ILC_CopyHeaders)
  
  if(EXPORT )
    set(HEADERS ${EXPORT})
    set(_headersdep)
    foreach(header ${HEADERS})
      get_filename_component( header_name ${header} NAME )
      add_custom_command(OUTPUT ${PEXPORTDEST}/${header_name}
                         COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${MODULE}/${header} ${PEXPORTDEST}/${header_name}
			 DEPENDS ${CMAKE_SOURCE_DIR}/${MODULE}/${header})
      list(APPEND _headersdep ${PEXPORTDEST}/${header_name})
      install(FILES ${header} DESTINATION include)
    endforeach(header)
    add_custom_target(${PACKAGE}-headers DEPENDS ${_headersdep})
    add_dependencies(${PACKAGE} ${PACKAGE}-headers)
  endif(EXPORT)

endmacro(ILC_CopyHeaders)

macro(ILC_GenerateLinkDef)
  set(PCLASSES)
  foreach (class ${PCINTCLASSES})
    get_filename_component(classname ${class} NAME)
    # set(PCLASSES ${PCLASSES} "\\n#pragma link C++ class ${classname}+;")
    set(PCLASSES ${PCLASSES} "${classname}")
  endforeach(class)

  add_custom_command(OUTPUT ${PDAL}
    COMMAND sh ${CMAKE_SOURCE_DIR}/cmake/GenerateLinkDef.sh ${PCLASSES} > ${PDAL} 
    DEPENDS ${PCINTHDRS} ${CMAKE_SOURCE_DIR}/cmake/GenerateLinkDef.sh)
endmacro(ILC_GenerateLinkDef)

macro(ILC_BuildPAR)
  
  if(EXISTS ${CMAKE_SOURCE_DIR}/${MODULE}/PROOF-INF.${PACKAGE})
    set(PARSRCS)
    foreach(file ${SRCS} ${HDRS} ${FSRCS} ${DHDR})
      get_filename_component(srcdir ${file} PATH)
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/${file}-par                         
                         COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/${srcdir}
                         COMMAND cp -pR ${file} ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/${file}
                         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
      list(APPEND PARSRCS ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/${file}-par)
    endforeach(file ${SRCS} ${HDRS} ${FSRCS} ${DHDR})
    
    add_custom_target(${PACKAGE}.par
                      COMMAND sed -e 's/include .\(ROOTSYS\)\\/\\(etc\\|test\\)\\/Makefile.arch/include Makefile.arch/\; s/PACKAGE = .*/PACKAGE = ${PACKAGE}/\; s,SRCS *=.*,SRCS = ${SRCS},\;' < Makefile | sed -e 's,HDRS *=.*,HDRS = ${HDRS},\; s,FSRCS *=.*,FSRCS = ${FSRCS},\; s,DHDR *=.*,DHDR = ${DHDR},' > ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/Makefile
                      COMMAND cp -pR ${ROOTSYS}/etc/Makefile.arch ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/Makefile.arch
                      COMMAND cp -pR PROOF-INF.${PACKAGE} ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}/PROOF-INF
#                      COMMAND cp -pR lib${PACKAGE}.pkg ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}
                      COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_BINARY_DIR} tar --exclude=.svn -czhf ${CMAKE_BINARY_DIR}/${PACKAGE}.par ${PACKAGE}
                      COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE}
                      COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red --bold "${PACKAGE}.par has been created in ${CMAKE_BINARY_DIR}"
                      DEPENDS ${PARSRCS} 
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )


    add_dependencies(${PACKAGE}.par ${SRCS} ${FSRCS} ${HDRS} ${DHDR})
    add_dependencies(par-all ${PACKAGE}.par)
    add_dependencies(${MODULE}-par-all ${PACKAGE}.par)

    add_custom_target(test-${PACKAGE}.par
                      COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --red --bold "The file ${PACKAGE}.par is now being tested, in case of an error check in par-tmp/${PACKAGE}."
                      COMMAND ${CMAKE_COMMAND} -E make_directory par-tmp
                      COMMAND ${CMAKE_COMMAND} -E chdir par-tmp tar xfz ../${PACKAGE}.par && ${CMAKE_COMMAND} -E chdir par-tmp/${PACKAGE} PROOF-INF/BUILD.sh
                      COMMAND ${CMAKE_COMMAND} -E remove_directory par-tmp
                      COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --green --bold "${PACKAGE}.par testing succeeded"
                      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    add_dependencies(test-${PACKAGE}.par ${PACKAGE}.par)
    add_dependencies(test-par-all test-${PACKAGE}.par)
    add_dependencies(test-${MODULE}-par-all test-${PACKAGE}.par)

  endif(EXISTS ${CMAKE_SOURCE_DIR}/${MODULE}/PROOF-INF.${PACKAGE})
  # endif(EXISTS ${ILC_ROOT}/${MODULE}/PROOF-INF.${PACKAGE})

endmacro(ILC_BuildPAR)

