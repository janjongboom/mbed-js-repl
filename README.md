# mbed-js-repl

JS REPL library for mbed OS 5. For an example program that uses the REPL, see [mbed-js-repl-example](https://github.com/janjongboom/mbed-js-repl-example).

## Usage

1. Start a new JavaScript on mbed project:

    ```
    $ git clone git@github.com:armmbed/mbed-js-example my-js-repl
    $ cd my-js-repl
    $ npm install
    ```

2. Add this library to your dependencies:

    ```
    $ npm install mbed-js-repl --save
    ```

3. Add the following line to your JS app:

    ```js
    var repl = new JSRepl();
    ```

4. Build your application:

    ```
    # replace K64F with your development board
    $ gulp --target=K64F
    ```

5. Run your program and connect over serial (baud rate 115,200) to your board to use the REPL.
