import asyncio
import logging
import re
import sys


from dbus_next import Message, MessageType
from dbus_next.aio import MessageBus
from dbus_next.constants import BusType

from dbus_next.service import (ServiceInterface,
                               method, dbus_property, signal)
from dbus_next import Variant, DBusError

import os

os.environ['DBUS_SESSION_BUS_ADDRESS'] = "unix:abstract=/dbus-session-address"

CLIENT_BUS_NAME = "fm.name.bus.client"
SERVER_BUS_NAME = "fm.name.bus"
INTERFACE_NAME = "fm.name.interface"
OBJECTPATH = "/fm/name/object/path"

class SignalEmitInterface(ServiceInterface):
    '''
    use to emit signal
    '''
    def __init__(self):
        super().__init__(INTERFACE_NAME)  # interface name
    
    @signal()
    def signal(self, value)->'u':
        print('send test signal %s' %value)
        return value
    
async def sendSignal():
    bus = await MessageBus().connect()
    interface = SignalEmitInterface()
    bus.export(OBJECTPATH, interface)  # path
    await bus.request_name(CLIENT_BUS_NAME)  # sender's name

    interface.signal(10)

    # await bus.wait_for_disconnect()
    
    
class DBUS:
    '''
    Util class for dbus.
    '''
    def __init__(self):
        self._bus = None
        self._proxy_objects = {}
        self._interfaces = {}

    async def connect(self):
        await self.get_bus()
        
    def disconnect(self):
        if self._bus is not None:
            self._bus.disconnect()
    async def get_bus(self):
        if self._bus is None:
            self._bus = await MessageBus(bus_type=BusType.SESSION).connect()
        return self._bus
    async def get_proxy_object(self, destination, path, api=None):
        key = (destination, path)
        if key not in self._proxy_objects:
            bus = await self.get_bus()
            if api is None:
                api = await bus.introspect(destination, path)
            self._proxy_objects[key] = bus.get_proxy_object(
                destination, path, api)
            
        return self._proxy_objects[key]
    
    async def get_interface(self, destination, path, iface, api=None):
        key = (destination, path, iface)
        if key not in self._interfaces:
            proxy_object = await self.get_proxy_object(destination, path, api=api)
            self._interfaces[key] = proxy_object.get_interface(iface)
        return self._interfaces[key]
    
class LogManager:
    lock = asyncio.Lock()
    bus = DBUS()
        
    @classmethod
    async def value_set(cls, destination, path, iface, key, value):
        '''
        send dbus to set value
        '''
        async with cls.lock:
            log_manager = await cls.bus.get_interface(
                destination = destination,
                path = path,
                iface = iface
            )
            ret = await log_manager.call_set_value(key, value)
            
            logging.error('value_set ret--- %s' %ret)
            
            return ret 
    
    @classmethod
    async def value_get(cls, destination, path, iface, key):
        '''
        send a dbus message to get value 
        '''
        async with cls.lock:
            log_manager = await cls.bus.get_interface(
                destination = destination,
                path = path,
                iface = iface
            )

            return await log_manager.call_get_value(key)
 
    @classmethod
    async def wait_signal_test(cls, destination, path, iface):
        '''
        send a dbus message to get debug level 
        '''
        
        def hello_notify(x):
            print('get signal with param %s' %x)
        
        async with cls.lock:
            log_manager = await cls.bus.get_interface(
                destination = destination,
                path = path,
                iface = iface
            )
            # print(dir(log_manager))
            # return await log_manager.call_debug_get(submodule)
            log_manager.on_test(hello_notify)
            # log_manager.off_test(hello_notify)
            
            # log_manager.call_function_one()
            # await cls.bus.wait_for_disconnect()
            

    @classmethod
    async def getListActivatableNames(cls):
        '''
        get all dubs connection in session bus
        '''
        async with cls.lock:
            mm = await cls.bus.get_interface(
                destination = 'org.freedesktop.DBus',
                path = '/',
                iface = 'org.freedesktop.DBus'
            )
            
            names = await mm.call_list_activatable_names()
            for name in names:
                print(name)
            
async def test_set_method():
    jobs = []
    jobs.append(LogManager.value_set(SERVER_BUS_NAME, OBJECTPATH, INTERFACE_NAME, 'aaa', 100))
    jobs.append(LogManager.value_set(SERVER_BUS_NAME, OBJECTPATH, INTERFACE_NAME, 'bbb', 200))
    group = asyncio.gather(*jobs, return_exceptions=True);
    ret = await group
    for i in ret:
        if i is None:
            print("get None ")
            continue
        if isinstance(i, Exception):
            print(i)
            continue
        # if i[0] == 0:
        #     print("call sucess %s" %i)
        # else:
        #     print("call fail %s" %i)
        print(i)
        
async def test_get_method():
    jobs = []
    jobs.append(LogManager.value_get(SERVER_BUS_NAME, OBJECTPATH, INTERFACE_NAME, 'all'))
    jobs.append(LogManager.value_get(SERVER_BUS_NAME, OBJECTPATH, INTERFACE_NAME, 'bbb'))
    group = asyncio.gather(*jobs, return_exceptions=True);
    ret = await group
    for i in ret:
        if i is None:
            print("get None ")
            continue
        if isinstance(i, Exception):
            print(i)
            continue
        # if i[0] == 0:
        #     print("call sucess %s" %i)
        # else:
        #     print("call fail %s" %i)
        print(i)
        
async def test_send_signal():
    await sendSignal()
    return
        
async def getListNames():
    '''
    get dubs name
    '''
    ret = await LogManager.getListActivatableNames()
    

async def main():
    param_cnt = len(sys.argv)
    print("Has " + str(param_cnt) + " param")
    if (param_cnt != 2):
        print("Usage: python3 log_analyd {list|signal|get}.")
        sys.exit()

    ptype = sys.argv[1]

    print("Type is " + ptype + ".")
    if ptype == 'list':
        await getListNames()
    elif ptype == "signal":
        await test_send_signal()
    elif ptype == "get":
        await test_get_method()
    elif ptype == "set":
        await test_set_method()
loop = asyncio.get_event_loop()
loop.run_until_complete(main())