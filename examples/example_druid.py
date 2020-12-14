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


print(poco.agent.input, type(poco.agent.input))

# load collection
print("Loading Druid example")
poco("bn_druid/larrybutton").click()


poco("scroll_view").swipe([-0.8, 0.0], duration=0.5)
time.sleep(1)
poco("scroll_view").swipe([0.4, 0.0], duration=0.5)
time.sleep(1)


# EXIT
print("Sleeping before exiting")
time.sleep(1)
poco("bn_back/larrybutton").click()

print("Done")

