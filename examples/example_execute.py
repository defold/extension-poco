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

cbk = poco.agent.rpc.call("Execute", "go.set('/bg#background', 'tint', vmath.vector4(1,0,0,1))")
cbk.wait()
print("Execute: result: '%s'" % cbk.result)

time.sleep(1)

cbk = poco.agent.rpc.call("Execute", "go.set('/bg#background', 'tint', vmath.vector4(1,1,1,1))")
cbk.wait()
print("Execute: result: '%s'" % cbk.result)

print("Done")
