set(BOOST_ADDS_SRC_PATH "${PROJECT_SOURCE_DIR}/src")
file(GLOB_RECURSE BOOST_ADDS_SRC_FILES "*.cpp" "*.hpp" )

# external libs
include(UseMvpPlayerBoost)

# Create 'boostAdds' static lib
add_library( boostAdds-static STATIC ${BOOST_ADDS_SRC_FILES} )
set_target_properties( boostAdds-static PROPERTIES LINKER_LANGUAGE CXX )
set_target_properties( boostAdds-static PROPERTIES OUTPUT_NAME boostAdds )
target_include_directories( boostAdds-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( boostAdds-static PUBLIC ${Boost_INCLUDE_DIRS} )
target_link_libraries( boostAdds-static ${Boost_LIBRARIES} )

# Create 'boostAdds' shared lib
add_library( boostAdds-shared SHARED ${BOOST_ADDS_SRC_FILES} )
set_target_properties( boostAdds-shared PROPERTIES LINKER_LANGUAGE CXX )
set_target_properties( boostAdds-shared PROPERTIES OUTPUT_NAME boostAdds )
set_target_properties( boostAdds-shared PROPERTIES SOVERSION ${BOOST_ADDS_VERSION_MAJOR} )
set_target_properties( boostAdds-shared PROPERTIES VERSION ${BOOST_ADDS_VERSION} )
target_include_directories( boostAdds-shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_include_directories( boostAdds-shared PUBLIC ${Boost_INCLUDE_DIRS} )
target_link_libraries( boostAdds-shared ${Boost_LIBRARIES} )

### Install boostAdds libs and include
install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	DESTINATION "include"
	FILES_MATCHING
	PATTERN "*.hpp"
	PATTERN "*.tcc"
)

install(TARGETS boostAdds-static DESTINATION lib/ OPTIONAL)
install(TARGETS boostAdds-shared DESTINATION lib/ OPTIONAL)