import sys
from optparse import OptionParser

DEFAULT_PORT = 15004
DEFAULT_ADDRESS = "0.0.0.0"

def validIP(address):
    parts = address.split(".")
    if len(parts) != 4:
        return False
    for item in parts:
        if not 0 <= int(item) <= 255:
            return False
    return True

def parse_arguments(args):
    name = sys.argv[0]
    usage = "usage: python %s [options]" % name

    parser = OptionParser(usage)
    parser.add_option("-a", "--address", dest="address", default = DEFAULT_ADDRESS, help="ip address to connect to", metavar="ADDRESS")
    parser.add_option("", "--port", dest="port", default = DEFAULT_PORT, help="port to connect to", metavar="PORT")
    parser.add_option("-p", "--platform", dest="platform", help="platform to connect to",
            default = "host",
            choices = ['android', 'host', 'macos', 'win32', 'linux'], metavar="PLATFORM")

    (options, args) = parser.parse_args()

    if options.address != DEFAULT_ADDRESS:
        options.platform = "unknown"

    elif options.platform == "host":
        options.address = "127.0.0.1"

    if not validIP(options.address):
        print("Invalid ip address format: %s" % options.address)
        sys.exit(1)

    return (options, args)
