import sys, json, time
import util

from poco.utils.device import VirtualDevice
from poco.drivers.std import StdPoco

options, args = util.parse_arguments(sys.argv)

# ************************************************************************************************

device = None # need None for the android fallback path
if options.address != util.DEFAULT_ADDRESS:
    device = VirtualDevice(options.address)

poco = StdPoco(addr=(options.address, options.port), device=device, use_airtest_input=False)

# ************************************************************************************************

# Calls the custom function that the game has registered with the server

cbk = poco.agent.rpc.call("TestNewFunction", 1, 2, 3)
cbk.wait()
print("TestNewFunction: %s" % cbk.result)

print("Done")
