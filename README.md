# Poco test framework integration

## Usage - Game side (i.e. server)

Add a require to one of your Defold scripts

    local poco = require('poco.lua.defold-poco')

Create a PocoManager instance at game startup:

    function init(self)
        poco:init_server(15004) -- default port for Poco
    end

Update the PocoManager instance continuously:

    function update(self, dt)
        poco:server_loop()
    end

## Usage - Client side (i.e. the test script)

This extension also contains a test project, which we can test against.
Launch the test project on your device and then run the [test script](./python/test.py)

Note: the script uses python2

For macOS:

    $ python python/test.py macOS


### Further reading

There are several resourcs on how to write a test script here:

* [AirTestProject/Poco](https://github.com/AirtestProject/Poco/blob/master/README.rst)

* [Poco - Examples and Tutorials](https://poco.readthedocs.io/en/latest/source/doc/poco-example/index.html#tutorial)