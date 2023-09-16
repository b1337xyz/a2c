#!/usr/bin/env python3
from xmlrpc.client import ServerProxy
from json import dumps
s = ServerProxy(f'http://127.0.0.1:6800/rpc').aria2
print(dumps(s.tellWaiting(0, 10)[0], indent=2))
