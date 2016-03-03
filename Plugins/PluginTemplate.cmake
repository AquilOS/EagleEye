
GET_PROPERTY(INCLUDE_DIRS TARGET ${PROJECT_NAME} PROPERTY INCLUDE_DIRECTORIES)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Plugins)
FILE(READ "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.txt" temp)
SET(PROJECT_ID)
IF(temp)
  STRING(FIND "${temp}" "\n" len)
  STRING(SUBSTRING "${temp}" 0 ${len} temp)
  SET(PROJECT_ID ${temp})
ELSE(temp)
  SET(PROJECT_ID "1")
ENDIF(temp)
LIST(REMOVE_DUPLICATES INCLUDE_DIRS)
LIST(REMOVE_DUPLICATES LINK_DIRS_RELEASE)
LIST(REMOVE_DUPLICATES LINK_DIRS_DEBUG)
if(WIN32)
string(REGEX REPLACE "-D" "/D" WIN_DEFS "${DEFS}")
string(REGEX REPLACE ";" " " WIN_DEFS "${WIN_DEFS}")
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.txt" "${PROJECT_ID}\n${INCLUDE_DIRS};\n${LINK_DIRS_DEBUG};\n${LINK_DIRS_RELEASE};\n/DPROJECT_BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\" ${WIN_DEFS} /DPLUGIN_NAME=${PROJECT_NAME} /FI\"EagleLib/Project_defs.hpp\"")
else(WIN32)
FILE(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.txt" "${PROJECT_ID}\n${INCLUDE_DIRS};\n${LINK_DIRS_DEBUG};\n${LINK_DIRS_RELEASE};\n-DPROJECT_BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\" ${DEFS} -DPLUGIN_NAME=${PROJECT_NAME} -include \"EagleLib/Project_defs.hpp\"")
endif(WIN32)
ADD_DEFINITIONS(-DPROJECT_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")
ADD_DEFINITIONS(-DPROJECT_CONFIG_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.txt\")
ADD_DEFINITIONS(-DPLUGIN_NAME=${PROJECT_NAME})
if(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI\"EagleLib/Project_defs.hpp\"")
else(WIN32)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include \"EagleLib/Project_defs.hpp\"")

endif(WIN32)

LINK_DIRECTORIES(${LINK_DIRS_DEBUG})
LINK_DIRECTORIES(${LINK_DIRS_RELEASE})
LINK_DIRECTORIES(${LINK_DIRS})
INSTALL(TARGETS ${PROJECT_NAME}
	LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
        
MESSAGE(STATUS 
"====== ${PROJECT_NAME} ======

 Project ID: ${PROJECT_ID}
 
 Config file: ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_config.txt
 
 Headers: 
 ${hdr}
 
 Source: 
 ${src}
 
 Cuda kernels: 
 ${knl}
 
 QT MOC: 
 ${MOC}
 
 Definitions:
 ${DEFS}
 
 C++ flags:
  ${CMAKE_CXX_FLAGS}
 Debug:
  ${CMAKE_CXX_FLAGS_DEBUG}
 Release:
  ${CMAKE_CXX_FLAGS_RELEASE}
 C Flags
  ${CMAKE_C_FLAGS}
  Include Dirs: ${INCLUDE_DIRS}
  
  Link Dirs Debug: ${LINK_DIRS_DEBUG}
  
  Link Dirs Release: ${LINK_DIRS_RELEASE}
 ")
