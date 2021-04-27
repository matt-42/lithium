

find_path(Wepoll_INCLUDE_DIR "wepoll.h")
find_library(Wepoll_LIBRARY "wepoll")

message("Found wepoll library " ${Wepoll_LIBRARY})
message("Found wepoll header " ${Wepoll_INCLUDE_DIR})
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Wepoll DEFAULT_MSG
		Wepoll_LIBRARY Wepoll_INCLUDE_DIR)