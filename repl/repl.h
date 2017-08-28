#ifndef _MBED_JS_REPL_
#define _MBED_JS_REPL_

#include <string>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>

#include "mbed.h"
#include "Callback.h"

#include "us_ticker_api.h"

using namespace std;

#ifndef JSMBED_USE_RAW_SERIAL
#error "Macro 'JSMBED_USE_RAW_SERIAL' not defined, required by mbed-js-repl"
#else
extern RawSerial pc;
#endif

class ReplBuffer {
public:
    ReplBuffer() {}

    void clear() {
        buffer.clear();
        position = 0;
    }

    void add(string s) {
        for(string::iterator it = s.begin(); it != s.end(); ++it) {
            buffer.insert(buffer.begin() + position, *it);
            position++;
        }
    }

    void add(char c) {
        buffer.insert(buffer.begin() + position, c);
        position++;
    }

    vector<char>::iterator begin() {
        return buffer.begin();
    }

    vector<char>::iterator end() {
        return buffer.end();
    }

    size_t getPosition() {
        return position;
    }

    void setPosition(size_t pos) {
        position = pos;
    }

    size_t size() {
        return buffer.size();
    }

private:
    vector<char> buffer;
    size_t position = 0;
};

class IRepl {
public:
    virtual void printJustHappened() = 0;
};

static IRepl* replInstance = NULL;

class Repl : public IRepl {
public:
    Repl() : historyPosition(0) {
        pc.printf("\r\nJavaScript REPL running...\r\n> ");

        replInstance = this;

        pc.attach(Callback<void()>(this, &Repl::callback));
    }

    void printJustHappened() {
        string s(buffer.begin(), buffer.end());
        pc.printf("> %s", s.c_str());
    }

private:
    void callback() {
        while (pc.readable()) {
            char c = pc.getc();

            // control characters start with 0x1b and end with a-zA-Z
            if (inControlChar) {

                controlSequence.push_back(c);

                // if a-zA-Z then it's the last one in the control char...
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                    inControlChar = false;

                    // up
                    if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x41) {
                        pc.printf("\033[u"); // restore current position

                        if (historyPosition == 0) {
                            // cannot do...
                        }
                        else {
                            historyPosition--;
                            // reset cursor to 0, do \r, then write the new command...
                            pc.printf("\33[2K\r> %s", history[historyPosition].c_str());

                            buffer.clear();
                            buffer.add(history[historyPosition]);
                        }
                    }
                    // down
                    else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x42) {
                        pc.printf("\033[u"); // restore current position

                        if (historyPosition == history.size()) {
                            // no-op
                        }
                        else if (historyPosition == history.size() - 1) {
                            historyPosition++;

                            // put empty
                            // reset cursor to 0, do \r, then write the new command...
                            pc.printf("\33[2K\r> ");

                            buffer.clear();
                        }
                        else {
                            historyPosition++;
                            // reset cursor to 0, do \r, then write the new command...
                            pc.printf("\33[2K\r> %s", history[historyPosition].c_str());

                            buffer.clear();
                            buffer.add(history[historyPosition]);
                        }
                    }
                    // left
                    else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x44) {
                        size_t curr = buffer.getPosition();

                        // at pos0? prevent moving to the left
                        if (curr == 0) {
                            pc.printf("\033[u"); // restore current position
                        }
                        // otherwise it's OK, move the cursor back
                        else {
                            buffer.setPosition(curr - 1);

                            pc.putc('\033');
                            for (size_t ix = 0; ix < controlSequence.size(); ix++) {
                                pc.putc(controlSequence[ix]);
                            }
                        }
                    }
                    // right
                    else if (controlSequence.size() == 2 && controlSequence.at(0) == 0x5b && controlSequence.at(1) == 0x43) {
                        size_t curr = buffer.getPosition();
                        size_t size = buffer.size();

                        // already at the end?
                        if (curr == size) {
                            pc.printf("\033[u"); // restore current position
                        }
                        else {
                            buffer.setPosition(curr + 1);

                            pc.putc('\033');
                            for (size_t ix = 0; ix < controlSequence.size(); ix++) {
                                pc.putc(controlSequence[ix]);
                            }
                        }
                    }
                    else {
                        // not up/down? Execute original control sequence
                        pc.putc('\033');
                        for (size_t ix = 0; ix < controlSequence.size(); ix++) {
                            pc.putc(controlSequence[ix]);
                        }
                    }

                    controlSequence.clear();
                }

                continue;
            }

            switch (c) {
                case '\r': /* want to run the buffer */
                    pc.putc(c);
                    pc.putc('\n');
                    js::EventLoop::getInstance().nativeCallback(Callback<void()>(this, &Repl::runBuffer));
                    break;
                case 0x08: /* backspace */
                case 0x7f: /* also backspace on some terminals */
                    js::EventLoop::getInstance().nativeCallback(Callback<void()>(this, &Repl::handleBackspace));
                    break;
                case 0x1b: /* control character */
                    // wait until next a-zA-Z
                    inControlChar = true;

                    pc.printf("\033[s"); // save current position

                    break; /* break out of the callback (ignore all other characters) */
                default:
                    size_t curr_pos = buffer.getPosition();
                    size_t buffer_size = buffer.size();

                    if (curr_pos == buffer_size) {
                        buffer.add(c);
                        pc.putc(c);
                    }
                    else {
                        // super inefficient...
                        string v(buffer.begin(), buffer.end());
                        v.insert(curr_pos, 1, c);

                        buffer.clear();
                        buffer.add(v);

                        buffer.setPosition(curr_pos + 1);

                        pc.printf("\r> %s\033[%dG", v.c_str(), int(curr_pos) + 4);
                    }
                    break;
            }
        }
    }

    void handleBackspace() {
        size_t curr_pos = buffer.getPosition();

        string v(buffer.begin(), buffer.end());

        if (v.size() == 0 || curr_pos == 0) return;

        bool endOfLine = curr_pos == v.size();

        v.erase(curr_pos - 1, 1);

        buffer.clear();
        buffer.add(v);

        buffer.setPosition(curr_pos - 1);

        if (endOfLine) {
            pc.printf("\b \b");
        }
        else {
            // carriage return, new text, set cursor, empty until end of line
            pc.printf("\r\033[K> %s\033[%dG", v.c_str(), curr_pos + 2);
        }
    }

    void runBuffer() {
        string rawCode(buffer.begin(), buffer.end());

        // pc.printf("Running: %s\r\n", rawCode.c_str());

        history.push_back(rawCode);
        historyPosition = history.size();

        // pc.printf("Executing (%s): ", rawCode.c_str());
        // for (size_t ix = 0; ix < rawCode.size(); ix++) {
        //     pc.printf(" %02x ", rawCode.at(ix));
        // }
        // pc.printf("\r\n");

        const jerry_char_t* code = reinterpret_cast<const jerry_char_t*>(rawCode.c_str());
        const size_t length = rawCode.length();

        jerry_value_t parsed_code = jerry_parse(code, length, false);

        // @todo, how do we get the error message? :-o

        if (jerry_value_has_error_flag(parsed_code)) {
            LOG_PRINT_ALWAYS("Syntax error while parsing code... (%s)\r\n", rawCode.c_str());
        }
        else {
            jerry_value_t returned_value = jerry_run(parsed_code);

            if (jerry_value_has_error_flag(returned_value)) {
                LOG_PRINT_ALWAYS("Running failed...\r\n");
            }
            else {
                jerry_value_t str_value = jerry_value_to_string(returned_value);

                jerry_size_t size = jerry_get_string_size(str_value);
                jerry_char_t ret_buffer[size + 1] = { 0 };

                jerry_string_to_char_buffer(str_value, ret_buffer, size);

                // jerry_string_to_char_buffer is not guaranteed to end with \0
                ret_buffer[size] = '\0';

                // reset terminal position to column 0...
                pc.printf("\33[2K\r");
                pc.printf("\33[36m"); // color to cyan

                if (jerry_value_is_string(returned_value)) {
                    pc.printf("\"%s\"", ret_buffer);
                }
                else if (jerry_value_is_array(returned_value)) {
                    pc.printf("[%s]", ret_buffer);
                }
                else {
                    pc.printf("%s", ret_buffer);
                }

                // pc.printf("\r\n");
                pc.printf("\33[0m"); // color back to normal
                pc.printf("\r\n");

                jerry_release_value(str_value);
            }

            jerry_release_value(returned_value);
        }

        jerry_release_value(parsed_code);

        buffer.clear();

        pc.printf("> ");
    }

    ReplBuffer buffer;
    bool inControlChar = false;
    vector<char> controlSequence;
    vector<string> history;
    size_t historyPosition;
};

static bool jerry_port_console_printing = false;

void jerry_port_console (const char *format, /**< format string */
                         ...) /**< parameters */
{
    if (strlen(format) == 1 && format[0] == 0x0a) { // line feed (\n)
        printf("\r"); // add CR for proper display in serial monitors

        jerry_port_console_printing = false; // not printing anymore...
    }

    if (!jerry_port_console_printing) {
        pc.printf("\33[100D\33[2K");
        jerry_port_console_printing = true;
    }

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);

    if (strlen(format) == 1 && format[0] == 0x0a && replInstance) {
        replInstance->printJustHappened();
    }
} /* jerry_port_console */


#endif // _MBED_JS_REPL_
