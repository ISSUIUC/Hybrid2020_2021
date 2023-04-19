import asyncio
from aiohttp import web

class TelemetryServer:
    def __init__(self):
        self.data = "{}";
        self.on_open = lambda: print("opened")
        self.on_close = lambda: print("closed")

    def add_open_listener(self, func):
        self.on_open = func

    def add_close_listener(self, func):
        self.on_close = func

    async def start(self, port):
        app = web.Application()
        app.add_routes([web.get('/', self.handle),
                        web.get('/{path}', self.handle),
                        web.get('/{path}/{code}', self.handle)])

        runner = web.AppRunner(app)
        await runner.setup()
        site = web.TCPSite(runner, 'localhost', port)
        print(f"starting server on localhost:{port}");
        await site.start()
    
    # data as json blob
    def update_data(self, data):
        self.data = data
            
    
    async def handle (self, request):
        path = request.match_info.get("path", "");
        code = request.match_info.get("code", "");

        if path == "data":
            return web.Response(text=self.data, content_type="text/json")

        elif path == "main.js":
            return web.Response(text=self.get_file_contents("./public/main.js"), content_type="text/javascript")

        elif path == "open" and code == "boomboom":
            self.on_open()
            return web.Response()

        elif path == "close" and code == "boomboom":
            self.on_close()
            return web.Response()

        else:
            return web.Response(text=self.get_file_contents("./public/a.html"), content_type="text/html")

    def get_file_contents(self, filename):
        with open(filename) as file:
            return file.read()
