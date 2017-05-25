#include "jerryscript-mbed-library-registry/wrap_tools.h"
#include "jerryscript-mbed-event-loop/EventLoop.h"
#include "repl/repl-js.h"
#include "repl/repl.h"

void JSRepl__destructor(uintptr_t native_ptr) {

}

DECLARE_CLASS_CONSTRUCTOR(JSRepl) {
    CHECK_ARGUMENT_COUNT(JSRepl, __constructor, (args_count == 0));

    Repl *repl = new Repl();
    uintptr_t native_ptr = (uintptr_t)repl;

    // create the jerryscript object
    jerry_value_t js_object = jerry_create_object();
    jerry_set_object_native_handle(js_object, native_ptr, JSRepl__destructor);

    return js_object;
}
