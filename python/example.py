import sys, json, time
from poco.utils.device import VirtualDevice
from poco.drivers.std import StdPoco

platform = None # defaults to Android
if len(sys.argv) > 1:
    platform = sys.argv[1]


DEFAULT_ADDRESS = ("0.0.0.0", 15004)

# if target is macOS, use a virtual device
device = None
if platform in ('macOS',):
    device = VirtualDevice(DEFAULT_ADDRESS[0])

poco = StdPoco(addr=DEFAULT_ADDRESS, device=device, use_airtest_input=False)

#ui = poco.agent.hierarchy.dump()
#print(json.dumps(ui, indent=4))

# For UIObjectProxy documentation, see https://poco-chinese.readthedocs.io/en/latest/source/poco.proxy.html

# Poco coords: Origin is top left
# Gui coords: Origin is top left
# Screen coords: Origin is bottom left
# Hid coords: Origin is bottom left (macOS)

# load level 1
print "Loading level 1"
poco("bn_level1/larrybutton").click()

# iterate over the animals
# Since we're actually removing them in this example, we need to collect the names first
# then find them again individually
animal_names = []
for animal in poco("animals").offspring(type="gui_node_box"):
    animal_names.append(animal.attr('name'))

for animal in animal_names:
    print "Drag", animal
    poco(animal).drag_to(poco("goal"), duration=0.75)
    time.sleep(1)

score = poco("score").get_text()

print "SCORE", score
assert score == "300"


poco("bn_back/larrybutton").click()


print "Done"

