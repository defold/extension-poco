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

def is_outside(pos):
    return pos[0] < 0.0 or pos[0] >= 1.0 or pos[1] < 0.0 or pos[1] > 1.0

def project_to_border(pos):
    # clip the line to the screen border
    playerx, playery = 0.5, 0.5
    diffx, diffy = pos[0] - playerx, pos[1] - playery

    limitx, limity = 1.0, 1.0
    if diffx < 0:
        limitx = 0.0
    if diffy < 0:
        limity = 0.0

    tx = (limitx - playerx) / diffx
    ty = (limity - playery) / diffy
    t = min(tx, ty)

    return (playerx + t * diffx, playery + t * diffy)

# In this test, we will go to each object in a certain sequence
animal_names = ["/parrot4", "/parrot1", "/parrot", "/parrot2", "/parrot3"]
for animal in animal_names:
    print("Go to", animal)
    child = poco(animal).child('sprite')

    pos = child.get_position()

    # If the position is outside of the screen, we approach the target by
    # clicking on the line towards the goal until the target is on screen
    while is_outside(pos):
        clickx, clicky = project_to_border(pos)

        print("click", clickx, clicky)
        poco.agent.input.click(clickx, clicky)
        time.sleep(0.6)

        pos = child.get_position()

    child.click(sleep_interval=0.01)
    time.sleep(0.6)


score = poco("score").get_text()

print("SCORE", score)
assert score == "Score: 500"


poco("bn_back/larrybutton").click()

print("Done")

