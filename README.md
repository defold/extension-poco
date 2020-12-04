# Poco test framework integration

## About

`extension-poco` is a native extension for [Defold](https://www.defold.com) to enable test automation from a computer to a test device (desktop or mobile).

This extension implements the [Poco](https://poco-chinese.readthedocs.io/en/latest/#) api, using the [Cocos2Dx-lua](https://poco-chinese.readthedocs.io/en/latest/source/doc/integration.html?highlight=cocos2d#cocos2dx-lua) Lua server, with some small cusomization on top (see `defold-poco.lua`)

## Installation

### Defold
To use this library in your Defold project, add the following URL to your `game.project` dependencies:

https://github.com/defold/extension-poco/archive/master.zip

We recommend using a link to a zip file of a [specific release](https://github.com/defold/extension-poco/releases).

### Python

The Poco api is compatible with both Python 2.7 and Python 3.3-3.6.

### Poco library

Install the Poco library (from [documentation](https://poco-chinese.readthedocs.io/en/latest/source/README.html#installation)):

    pip install pocoui

## Usage - Game side

### Server

See [main.script](./main/main.script) for an example.

Note that you can also use this in the `gui` script context as well.

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

### Custom function

You can add a custom function that you can call from the client scripts:

Server:

    poco:setDispatchFn("TestNewFunction", function(x, y, z)
            print("INSIDE NEW CLIENT TEST FUNCTION!", x, y, z)
            return {message= "" .. (x + y + z)}
        end)

Client:

    cbk = poco.agent.rpc.call("TestNewFunction", 1, 2, 3)
    cbk.wait()


## Custom hook

You can also add a hook to an existing function, in order to manipulate the result before it's returned to the client.

    poco:setDispatchCallbackFn("Dump", function(scene)
        pprint("INSIDE NEW CALLBACK FUNCTION!", scene)
        return scene
    end)


## Usage - Client side (i.e. the test script)

### Examples
[Examples](https://github.com/defold/extension-poco/tree/master/python)

This extension also contains a test project, which we can test against.

Launch the test project on your device and then run one of the [test scripts](https://github.com/defold/extension-poco/tree/master/python).

How to test against a local build on the host computer:

    $ python2 ./examples/example_dump.py

Connect to a certain ip (any device type):

    $ python2 ./examples/example_dump.py -a <ip address>

Connect to an Android device via `adb` (if you don't know its ipaddress)

    $ python2 ./examples/example_dump.py -p android

For list of all arguments:

    $ python2 ./examples/example_dump.py -h

### Poco Documentation

There are several resources on how to write a test script here:

* [AirTestProject/Poco](https://github.com/AirtestProject/Poco/blob/master/README.rst)

* [Poco - Examples and Tutorials](https://poco.readthedocs.io/en/latest/source/doc/poco-example/index.html#tutorial)


### Caveats

While the Defold scene graph inspection api strives to reconstruct the scene as it once was in the Defold Editor, it is not possible to accurately do so to 100%. Some of the data is not avaiable once in runtime format.

Each component type also has to implement its own hooks into the inspection api, so if the component type you're looking for isn't seen in the output graph, this might be the reason.

