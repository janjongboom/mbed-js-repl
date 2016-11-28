#ifndef _JERRYSCRIPT_MBED_LIB_REPL_H
#define _JERRYSCRIPT_MBED_LIB_REPL_H

#include "jerryscript-mbed-library-registry/wrap_tools.h"
#include "repl-js.h"

DECLARE_JS_WRAPPER_REGISTRATION (repl)
{
    REGISTER_CLASS_CONSTRUCTOR(JSRepl);
}

#endif // _JERRYSCRIPT_MBED_LIB_REPL_H
