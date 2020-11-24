import sys, json
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

ui = poco.agent.hierarchy.dump()
print(json.dumps(ui, indent=4))

#print("poco.py:", "get_screen_size", poco.get_screen_size())

# For UIObjectProxy documentation, see https://poco-chinese.readthedocs.io/en/latest/source/poco.proxy.html


#node_name = 'OctopusArea'
#node_name = 'child1'
node_name = '/bg'

# print("exists", "not_exist", poco("not_exist").exists())
# print("exists", node_name, poco(node_name).exists())

#print poco(node_name).child('child1').attr('name')

print "Children"
for child in poco("main").children():
    print "  ", child.attr('name'), child.attr('type'), child.attr('text')

poco("/go").poco("logo").click(sleep_interval=0.001)

# poco(node_name).drag_to((0.5, 0.5), duration=0.4)

# print("get_name", poco(node_name).get_name())
# print("get_text", poco(node_name).get_text())
# print("get_bounds", poco(node_name).get_bounds())

# children = poco(node_name).children()
# print("children", children)

# Executed in the context of the main.script, where the server is currrently run
#cbk = poco.agent.rpc.call("Execute", "go.animate('/go', 'euler.z', go.PLAYBACK_LOOP_FORWARD, 360, go.EASING_LINEAR, 2)")
#cbk.wait()
#print("Execute: %s" % cbk.result)

# testing the new client defined function from main.script
# cbk = poco.agent.rpc.call("TestNewFunction", 1, 2, 3)
# cbk.wait()
#print("TestNewFunction: %s" % cbk.result)

print("poco.py:", "done.")


