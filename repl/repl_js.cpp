#include "jerryscript-mbed-library-registry/wrap_tools.h"
#include "jerryscript-mbed-event-loop/EventLoop.h"
#include "repl/repl-js.h"
#include "repl/repl.h"

using namespace mbed::js;

DECLARE_GLOBAL_FUNCTION(repl_start) {
    Repl *repl = new Repl(EventLoop::getInstance().getQueue());
    // uintptr_t native_ptr = (uintptr_t)repl;

    return jerry_create_boolean(true);
}
