# S2OPC Node wrapper

## Restrictions

In order for the package to work, you should have the `libclient_subscription` library in your LD_LIBRARY_PATH.

## Install

`npm install`

## Examples

Examples are available in the `examples` folder and cover the basic services offered by the wrapper.
They can be launched using `node examples/<example>.js` or via `npm run <example>`.

## Running the tests

First, you should have the `toolkit_test_server` (from S2OPC) running.
Then launch `npm test`.
