# mbed-js-repl

JS REPL library for mbed OS 5. For an example program that uses the REPL, see [mbed-js-repl-example](https://github.com/janjongboom/mbed-js-repl-example).

## Usage

1. Start a new JavaScript on mbed project (f.e. from mbed-js-example).
2. Add this library to your dependencies list in package.json.
3. Add the following line to your JS app:

    ```js
    var repl = new JSRepl();
    ```
    
4. Run your program and connect over serial to your board to use the REPL.
