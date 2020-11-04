import sys
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

print("poco.py:", "get_screen_size", poco.get_screen_size())

# For UIObjectProxy documentation, see https://poco-chinese.readthedocs.io/en/latest/source/poco.proxy.html

node_name = 'OctopusArea'

poco(node_name).click(sleep_interval=0.001)

poco(node_name).drag_to((0.5, 0.5), duration=0.4)

print("get_name", poco(node_name).get_name())
print("get_text", poco(node_name).get_text())
print("get_bounds", poco(node_name).get_bounds())

children = poco(node_name).children()
print("children", children)

print("exists", node_name, poco(node_name).exists())
print("exists", "not_exist", poco("not_exist").exists())

print("poco.py:", "done.")