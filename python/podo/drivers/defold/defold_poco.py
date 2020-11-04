# -*- coding: utf-8 -*-

from poco.agent import PocoAgent
from poco.exceptions import InvalidOperationException
from poco.freezeui.hierarchy import FrozenUIHierarchy, FrozenUIDumper
from poco.drivers.std.attributor import StdAttributor
from poco.drivers.std.dumper import StdDumper
from poco.drivers.std.screen import StdScreen
from poco.drivers.std.inputs import StdInput
from poco.pocofw import Poco
from poco.utils.airtest import AirtestInput, AirtestScreen
from poco.utils.simplerpc.utils import sync_wrapper

from poco.utils.simplerpc.rpcclient import RpcClient
from poco.utils.simplerpc.transport.tcp.main import TcpClient

from poco.utils import six
if six.PY3:
    from urllib.parse import urlparse
else:
    from urlparse import urlparse

from poco.drivers.std import StdPocoAgent

DEFAULT_PORT = 15004
DEFAULT_ADDR = ('localhost', DEFAULT_PORT)

# class DefoldPocoAgent(PocoAgent):
#     def __init__(self, addr):
#         # looking more and more like StdPocoAgent
#         self.conn = TcpClient(addr)
#         self.c = RpcClient(self.conn)
#         self.c.connect()

#         hierarchy = FrozenUIHierarchy(StdDumper(self.c), StdAttributor(self.c))
#         screen = StdScreen(self.c)
#         inputs = StdInput(self.c)

#         super(DefoldPocoAgent, self).__init__(hierarchy, inputs, screen, None)

#     def on_bind_driver(self, driver):
#         super(DefoldPocoAgent, self).on_bind_driver(driver)
#         if isinstance(self.input, AirtestInput):
#             self.input.add_preaction_cb(driver)


class DefoldPoco(Poco):
    def __init__(self, addr=DEFAULT_ADDR, device=None, use_airtest_input=True, **kwargs):

        if not isinstance(addr, (tuple, list, six.string_types)):
            raise TypeError('Argument "addr" should be `tuple[2]`, `list[2]` or `string` only. Got {}'.format(type(addr)))

        try:
            if isinstance(addr, (list, tuple)):
                ip, port = addr
            else:
                port = urlparse(addr).port
                if not port:
                    raise ValueError
        except ValueError:
            raise ValueError('Argument "addr" should be a tuple[2] or string format. e.g. '
                             '["localhost", 15004] or "{}". Got {}'.format(DEFAULT_ADDR, repr(addr)))

        #agent = DefoldPocoAgent(addr)
        agent = StdPocoAgent(addr, use_airtest_input)
        kwargs['reevaluate_volatile_attributes'] = True
        super(DefoldPoco, self).__init__(agent, **kwargs)


