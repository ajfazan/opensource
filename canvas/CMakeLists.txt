PROJECT( CANVAS )
CMAKE_MINIMUM_REQUIRED( VERSION 2.8.4 )

SET( HEADERS image.hpp image8.hpp image16.hpp image32.hpp )
SET( SOURCES image.cpp image8.cpp image16.cpp image32.cpp )

SET( CMAKE_INSTALL_PREFIX $ENV{WS_INSTALL} )

ADD_LIBRARY( canvas SHARED ${SOURCES} )

GET_FILENAME_COMPONENT( PARENT_DIR ${CMAKE_SOURCE_DIR} PATH )

INCLUDE_DIRECTORIES( ${PARENT_DIR} $ENV{WS_INSTALL}/include )

SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frounding-math" )

FIND_LIBRARY( GDAL gdal )
IF( GDAL )
  TARGET_LINK_LIBRARIES( canvas ${GDAL} )
ENDIF( GDAL )

FIND_LIBRARY( CGAL CGAL )
IF( CGAL )
  TARGET_LINK_LIBRARIES( canvas ${CGAL} )
ENDIF( CGAL )

FIND_LIBRARY( CGAL_Core CGAL_Core )
IF( CGAL_Core )
  TARGET_LINK_LIBRARIES( canvas ${CGAL_Core} )
ENDIF( CGAL_Core )

FIND_LIBRARY( GMP gmp )
IF( GMP )
  TARGET_LINK_LIBRARIES( canvas ${GMP} )
ENDIF( GMP )

FIND_LIBRARY( MPFR mpfr )
IF( MPFR )
  TARGET_LINK_LIBRARIES( canvas ${MPRF} )
ENDIF( MPFR )

FIND_PACKAGE( Boost REQUIRED COMPONENTS filesystem system thread )
IF( Boost_FOUND )
  INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
  TARGET_LINK_LIBRARIES( canvas ${Boost_LIBRARIES} )
ENDIF( Boost_FOUND )

INSTALL( FILES ${HEADERS} DESTINATION include/canvas )

IF( CYGWIN )
  INSTALL( FILES cygcanvas.dll DESTINATION bin )
  INSTALL( FILES libcanvas.dll.a DESTINATION lib )
ELSE( CYGWIN )
  INSTALL( TARGETS canvas DESTINATION lib )
ENDIF( CYGWIN )
