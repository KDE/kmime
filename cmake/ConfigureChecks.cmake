include (CheckCXXSourceCompiles)

check_cxx_source_compiles("
#include <string.h>
int main() { strcasestr(\"\", \"\"); }
"
    HAVE_STRCASESTR)
