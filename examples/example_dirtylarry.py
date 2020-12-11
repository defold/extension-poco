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


# load collection
print("Loading Dirty Larry example")
poco("bn_dirtylarry/larrybutton").click()

# SLIDER TEST
slider = poco("slider/larryslider")

print("slider: anchorPoint", slider.attr('anchorPoint'))

poco("slider/larryslider").click(focus='center')
slider_value = poco("slider/larryvalue").get_text()
print("slider value:", slider_value)

poco("slider/larryslider").click(focus=[0.5,0.5])
slider_value = poco("slider/larryvalue").get_text()
print("slider value:", slider_value)


poco("slider/larryslider").click(focus=[0.0,0.0])
slider_value = poco("slider/larryvalue").get_text()
print("slider value:", slider_value)

poco("slider/larryslider").click(focus=[1.0,0.0])
slider_value = poco("slider/larryvalue").get_text()
print("slider value:", slider_value)

# RADIO TEST
radio_value = poco("radio_value").get_text()
print("radio value:", radio_value)

poco("radio_1/larryradio").click()
radio_value = poco("radio_value").get_text()
print("radio value:", radio_value)

# CHECKBOX TEST
checkbox_value = poco("checkbox_value").get_text()
print("checkbox value:", checkbox_value)

poco("checkbox/larrycheckbox").click()
checkbox_value = poco("checkbox_value").get_text()
print("checkbox value:", checkbox_value)


# INPUT TEST
poco("input_email/bg").click()

poco.agent.input.keyevent('test@mail.coo{KEY_BACKSPACE}m')

poco("bn_login/larrybutton").click()

login_status = poco("login_status").get_text()

print("login_status:", login_status)


# EXIT
print("Sleeping before exiting")
time.sleep(1)
poco("bn_back/larrybutton").click()

print("Done")


"""

MANUAL CLICK
    LEFT:
        DEBUG:SCRIPT: TOUCH:    hash: [touch]   190.25  658.75  380 1318
    RIGHT:
        DEBUG:SCRIPT: TOUCH:    hash: [touch]   484.75  657.25  969 1315

"""



