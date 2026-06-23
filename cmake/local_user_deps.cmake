# Project-local dependency hints used when system development packages are not
# available to the current user.
set(RMDB_LOCAL_DEPS_ROOT "$ENV{HOME}/.local/rmdb-deps")

if(EXISTS "${RMDB_LOCAL_DEPS_ROOT}/usr/include")
    include_directories(SYSTEM "${RMDB_LOCAL_DEPS_ROOT}/usr/include")
endif()

if(EXISTS "${RMDB_LOCAL_DEPS_ROOT}/usr/lib/x86_64-linux-gnu")
    link_directories("${RMDB_LOCAL_DEPS_ROOT}/usr/lib/x86_64-linux-gnu")
endif()

if(EXISTS "${RMDB_LOCAL_DEPS_ROOT}/usr/lib/x86_64-linux-gnu/pkgconfig")
    list(PREPEND CMAKE_PREFIX_PATH "${RMDB_LOCAL_DEPS_ROOT}/usr")
    set(ENV{PKG_CONFIG_PATH}
        "${RMDB_LOCAL_DEPS_ROOT}/usr/lib/x86_64-linux-gnu/pkgconfig:$ENV{PKG_CONFIG_PATH}")
endif()
