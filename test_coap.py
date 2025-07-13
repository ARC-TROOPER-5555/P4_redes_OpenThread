import logging
import asyncio

from aiocoap import *

# Dirección IP del Border Router
board_IP = "[xxxxxxxxx]"

# Descomenta el método que quieres probar:
METHOD = "GET"
#METHOD = "PUT"
#METHOD = "DELETE"

# Descomenta el método que quieres probar:
URI = "sensor"
#URI = "led"
#URI = "nombre"
PAYLOAD = b"1"

logging.basicConfig(level=logging.INFO)

async def get(ip, uri):
    protocol = await Context.create_client_context()
    request = Message(code=GET, uri='coap://' + ip + '/' + uri)
    try:
        response = await protocol.request(request).response
    except Exception as e:
        print('Failed to fetch resource:')
        print(e)
    else:
        print('Result: %s\n%r' % (response.code, response.payload.decode()))

async def put(ip, uri, payload):
    context = await Context.create_client_context()
    await asyncio.sleep(2)
    request = Message(code=PUT, payload=payload, uri='coap://' + ip + '/' + uri)
    response = await context.request(request).response
    print('Result: %s\n%r' % (response.code, response.payload.decode()))

async def delete(ip, uri):
    context = await Context.create_client_context()
    await asyncio.sleep(2)
    request = Message(code=DELETE, uri='coap://' + ip + '/' + uri)
    response = await context.request(request).response
    print('Result: %s\n%r' % (response.code, response.payload.decode()))

if __name__ == "__main__":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())

    if METHOD == "GET":
        print("*** GET ***")
        asyncio.run(get(board_IP, URI))
    elif METHOD == "PUT":
        print("*** PUT ***")
        asyncio.run(put(board_IP, URI, PAYLOAD))
        print("*** GET ***")
        asyncio.run(get(board_IP, URI))
    elif METHOD == "DELETE":
        print("*** DELETE ***")
        asyncio.run(delete(board_IP, URI))
        print("*** GET ***")
        asyncio.run(get(board_IP, URI))
