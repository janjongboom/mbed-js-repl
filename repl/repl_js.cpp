/* Copyright (c) 2016 ARM Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "jerry-core/jerry-api.h"
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
