#include "jerryscript-mbed-library-registry/wrap_tools.h"
#include "jerryscript-mbed-event-loop/EventLoop.h"
#include "repl/repl-js.h"
#include "repl/repl.h"

DECLARE_GLOBAL_FUNCTION(repl_start) {
    Repl *repl = new Repl();
    // uintptr_t native_ptr = (uintptr_t)repl;

    return jerry_create_boolean(true);
}
