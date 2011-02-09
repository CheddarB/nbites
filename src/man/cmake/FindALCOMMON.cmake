SET( ALCOMMON_DEFINITIONS "" )

##
# Includes
##

IF(WEBOTS_BACKEND)
  SET( ALCOMMON_INCLUDE_DIR ${AL_DIR}/include/alcommon
    ${AL_DIR}/lib
    ${AL_DIR}/include/alproxies
    )
ELSE(WEBOTS_BACKEND)
  IF(OE_CROSS_BUILD)
  SET( ALCOMMON_INCLUDE_DIR ${OE_SYSROOT}/usr/include/alcommon/include
    ${OE_SYSROOT}/usr/include/alproxies
    ${OE_SYSROOT}/usr/include/alcommon
    ${OE_SYSROOT}/usr/include/alcore
    ${OE_SYSROOT}/usr/include/libthread
    ${OE_SYSROOT}/usr/include/alvalue
    ${OE_SYSROOT}/usr/include/altools
    ${OE_SYSROOT}/usr/include/alfactory
    ${OE_SYSROOT}/usr/include
    )
  ELSE(OE_CROSS_BUILD)
  SET( ALCOMMON_INCLUDE_DIR ${AL_DIR}/include/alcommon/include
    ${AL_DIR}/include/alproxies
    ${AL_DIR}/include/alcommon
    ${AL_DIR}/include/alcore
    ${AL_DIR}/include/libthread
    ${AL_DIR}/include/alvalue
    ${AL_DIR}/include/altools
    ${AL_DIR}/include/alfactory
    ${AL_DIR}/include
    )
  ENDIF(OE_CROSS_BUILD)
ENDIF(WEBOTS_BACKEND)

    IF( OE_CROSS_BUILD )
        SET( ALCOMMON_LIBRARIES
          ${OE_SYSROOT}/usr/lib/libalcommon.so )
    ELSE(OE_CROSS_BUILD )
      IF( WIN32 )
        SET( ALCOMMON_LIBRARIES ${AL_DIR}/lib/alcommon.lib)
      ELSE( WIN32 )
        IF (APPLE)
          SET( ALCOMMON_LIBRARIES
            ${AL_DIR}/lib/libalcommon.a
            )
        ELSE(APPLE)
          SET( ALCOMMON_LIBRARIES
	    ${AL_DIR}/lib/libalcommon.so
            )
        ENDIF(APPLE)
      ENDIF( WIN32 )
    ENDIF( OE_CROSS_BUILD )

IF( ALCOMMON_LIBRARIES AND EXISTS ${ALCOMMON_LIBRARIES} )
    SET( ALCOMMON_FOUND TRUE )
ENDIF( ALCOMMON_LIBRARIES AND EXISTS ${ALCOMMON_LIBRARIES} )

IF( NOT ALCOMMON_FOUND_TRUE AND ALCOMMON_FIND_REQUIRED )
    IF( NOT ALCOMMON_INCLUDE_DIR )
        MESSAGE( STATUS "Required include not found" )
        MESSAGE( FATAL_ERROR "Could not find ALCOMMON include!")
    ENDIF( NOT ALCOMMON_INCLUDE_DIR )
    IF( NOT ALCOMMON_LIBRARIES )
        MESSAGE( STATUS "Required libraries not found" )
        MESSAGE( FATAL_ERROR "Could not find ALCOMMON libraries!")
    ENDIF( NOT ALCOMMON_LIBRARIES )
ENDIF( NOT ALCOMMON_FOUND_TRUE AND ALCOMMON_FIND_REQUIRED )

##
# Finally, display informations if not in quiet mode
##

IF( NOT ALCOMMON_FIND_QUIETLY )
  MESSAGE( STATUS "ALCOMMON found " )
  MESSAGE( STATUS "  includes   : ${ALCOMMON_INCLUDE_DIR}" )
  MESSAGE( STATUS "  libraries  : ${ALCOMMON_LIBRARIES}" )
  MESSAGE( STATUS "  definitions: ${ALCOMMON_DEFINITIONS}" )
ENDIF( NOT ALCOMMON_FIND_QUIETLY )



MARK_AS_ADVANCED(
  ALCOMMON_DEFINITIONS
  ALCOMMON_INCLUDE_DIR
  ALCOMMON_LIBRARIES
)
