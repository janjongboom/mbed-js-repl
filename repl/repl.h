#ifndef _MBED_JS_REPL_
#define _MBED_JS_REPL_

#include <string>
#include <sstream>
#include "mbed.h"
#include "Callback.h"

using namespace std;

extern RawSerial pc;

class Repl {
public:
    Repl() {
        pc.printf("JavaScript REPL running...\r\n> ");

        pc.attach(Callback<void()>(this, &Repl::callback));
    }

private:
    void callback() {
        while (pc.readable()) {
            char c = pc.getc();

            // pc.printf(" %02x ", c);

            switch (c) {
                case '\r': /* no-op */
                    pc.putc(c);
                    pc.putc('\n');
                    js::EventLoop::getInstance().nativeCallback(Callback<void()>(this, &Repl::runBuffer));
                    break;
                case 0x7f: /* backspace */
                    js::EventLoop::getInstance().nativeCallback(Callback<void()>(this, &Repl::handleBackspace));
                    break;
                case 0x1b: /* control character */
                    pc.putc(c);

                    // forward everything to PC, but do not put them in buffer
                    while (pc.readable()) {
                        pc.putc(pc.getc());
                    }

                    return; /* break out of the callback (ignore all other characters) */
                default:
                    buffer << c;
                    pc.putc(c);
                    break;
            }
        }
    }

    void handleBackspace() {
        string v = buffer.str();

        if (v.size() == 0) return;

        v.resize(v.size () - 1);

        buffer.str("");
        buffer << v;

        pc.printf("\b \b");
    }

    void runBuffer() {
        // pc.printf("Running: %s\r\n", buffer.str().c_str());

        string codez = buffer.str();

        // pc.printf("Executing: ");
        // for (size_t ix = 0; ix < codez.size(); ix++) {
        //     pc.printf(" %02x ", codez.at(ix));
        // }
        // pc.printf("\r\n");

        const jerry_char_t* code = reinterpret_cast<const jerry_char_t*>(codez.c_str());
        const size_t length = codez.length();

        jerry_value_t parsed_code = jerry_parse(code, length, false);

        // @todo, how do we get the error message? :-o

        if (jerry_value_has_error_flag(parsed_code)) {
            LOG_PRINT_ALWAYS("Syntax error while parsing code...\r\n");
        }
        else {
            jerry_value_t returned_value = jerry_run(parsed_code);

            if (jerry_value_has_error_flag(returned_value)) {
                LOG_PRINT_ALWAYS("Running failed...\r\n");
            }
            else {
                jerry_value_t str_value = jerry_value_to_string(returned_value);

                jerry_size_t size = jerry_get_string_size(str_value);
                jerry_char_t buffer[size];

                jerry_string_to_char_buffer(str_value, buffer, size);

                // reset terminal position to column 0...
                pc.printf("\33[2K\r");

                if (jerry_value_is_string(returned_value)) {
                    pc.printf("\"%s\"\r\n", buffer);
                }
                else if (jerry_value_is_array(returned_value)) {
                    pc.printf("[%s]\r\n", buffer);
                }
                else {
                    pc.printf("%s\r\n", buffer);
                }

                jerry_release_value(str_value);
            }

            jerry_release_value(returned_value);
        }

        jerry_release_value(parsed_code);

        buffer.str("");

        pc.printf("> ");
    }

    stringstream buffer;
};

#endif // _MBED_JS_REPL_
