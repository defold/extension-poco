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

# Executed in the context where the server is currrently run
# In our case it happens to be the main.script

# ui = poco.agent.hierarchy.dump()
# print(json.dumps(ui, indent=4))


#cbk = poco.agent.rpc.call("Execute", "TestNewFunction(1,2,3)")

cbk = poco.agent.rpc.call("Execute", "return gui.is_enabled(gui.get_node('box1'))")
cbk.wait()
print("Execute: result: '%s'" % cbk.result)


cbk = poco.agent.rpc.call("Execute", "return gui.is_enabled(gui.get_node('box2'))")
cbk.wait()
print("Execute: result: '%s'" % cbk.result)


#cbk = poco.agent.rpc.call("Execute", "return gui.is_enabled('box1')")
# cbk = poco.agent.rpc.call("GuiIsEnabled", "box1")
# cbk.wait()
# print("Execute: result: '%s'" % cbk.result)

# cbk = poco.agent.rpc.call("GuiIsEnabled", "box2")
# cbk.wait()
# print("Execute: result: '%s'" % cbk.result)

# time.sleep(1)

# cbk = poco.agent.rpc.call("Execute", "go.set('/bg#background', 'tint', vmath.vector4(1,1,1,1))")
# cbk.wait()
# print("Execute: result: '%s'" % cbk.result)

print("Done")
