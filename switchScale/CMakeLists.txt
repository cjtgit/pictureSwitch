project( showim )
find_package( OpenCV REQUIRED )
add_executable( showim showim )
target_link_libraries( showim ${OpenCV_LIBS} )

