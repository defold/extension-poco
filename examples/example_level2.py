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

# For UIObjectProxy documentation, see https://poco-chinese.readthedocs.io/en/latest/source/poco.proxy.html

# load level
print("Loading level 2")
poco("bn_level2/larrybutton").click()

# In this test, we will go to each object in a certain sequence
animal_names = ["/parrot1", "/parrot4", "/parrot", "/parrot2", "/parrot3"]
for animal in animal_names:
    print("Go to", animal)
    child = poco(animal).child('sprite')
    print(animal, child.get_position(), child.get_size())
    child.click(sleep_interval=0.01)
    time.sleep(0.6)


score = poco("score").get_text()

print("SCORE", score)
assert score == "Score: 500"


poco("bn_back/larrybutton").click()

print("Done")

