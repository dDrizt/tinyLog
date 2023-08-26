#include "tinylog.h"

int main() {
    tinylog::log_info("the answer is {}", 42);

    #define _FUNCTION(name) tinylog::log_##name(#name);
        TINYLOG_FOREACH_LOG_LEVEL(_FUNCTION)
    #undef _FUNCTION
    

    return 0;
}
