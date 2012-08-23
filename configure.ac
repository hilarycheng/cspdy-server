AC_PREREQ([2.68])
AC_INIT([cspdyserver], [0.1], [hilarycheng@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_PROG_CXX

AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h sys/ioctl.h sys/socket.h unistd.h])

AC_CHECK_LIB([ev], [ev_default_loop])
AC_CHECK_LIB([pthread], [pthread_create])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([socket])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
