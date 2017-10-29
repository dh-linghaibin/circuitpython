import time
import array
import layer
import board
import digitalio
import busio
import gamepad
import ustruct


K_X = 0x01
K_DOWN = 0x02
K_LEFT = 0x04
K_RIGHT = 0x08
K_UP = 0x10
K_O = 0x20


def color565(r, g, b):
    return (r & 0xf8) << 8 | (g & 0xfc) << 3 | b >> 3



class ST7735R:
    def __init__(self, spi, dc):
        self.spi = spi
        spi.try_lock()
        spi.configure(baudrate=24000000, polarity=0, phase=0)
        self.dc = dc
        self.dc.switch_to_output(value=0)
        for command, data in (
            (b'\x01', b''),
            (b'\x11', b''),
            (b'\x36', b'\xc8'),
            (b'\x3a', b'\x05'),
            (b'\xb4', b'\x07'),
            (b'\xb1', b'\x01\x2c\x2d'),
            (b'\xb2', b'\x01\x2c\x2d'),
            (b'\xb3', b'\x01\x2c\x2d\x01\x2c\x2d'),
            (b'\xc0', b'\x02\x02\x84'),
            (b'\xc1', b'\xc5'),
            (b'\xc2', b'\x0a\x00'),
            (b'\xc3', b'\x8a\x2a'),
            (b'\xc4', b'\x8a\xee'),
            (b'\xc5', b'\x0e'),
            (b'\x20', b''),
            (b'\xe0', b'\x02\x1c\x07\x12\x37\x32\x29\x2d'
             b'\x29\x25\x2B\x39\x00\x01\x03\x10'),
            (b'\xe1', b'\x03\x1d\x07\x06\x2E\x2C\x29\x2D'
             b'\x2E\x2E\x37\x3F\x00\x00\x02\x10'),
            (b'\x13', b''),
            (b'\x29', b''),
        ):
            self._write(command, data)
        self.dc.value = 0

    def block(self, x0, y0, x1, y1, data=None):
        xpos = ustruct.pack('>HH', x0 + 2, x1 + 2)
        ypos = ustruct.pack('>HH', y0 + 3, y1 + 3)
        self._write(b'\x2a', xpos)
        self._write(b'\x2b', ypos)
        self._write(b'\x2c', data)

    def _write(self, command=None, data=None):
        if command is not None:
            self.dc.value = 0
            self.spi.write(command)
        if data:
            self.dc.value = 1
            self.spi.write(data)

    def fill(self, color):
        self.block(0, 0, 127, 127, b'')
        pixel = color.to_bytes(2, 'big')
        data = pixel * 256
        for count in range(64):
            self._write(None, data)

class BMP:
    def __init__(self, filename):
        self.filename = filename
        self.colors = 0

    def read_header(self):
        if self.colors:
            return
        with open(self.filename, 'rb') as f:
            f.seek(10)
            self.data = int.from_bytes(f.read(4), 'little')
            f.seek(18)
            self.width = int.from_bytes(f.read(4), 'little')
            self.height = int.from_bytes(f.read(4), 'little')
            f.seek(46)
            self.colors = int.from_bytes(f.read(4), 'little')

    def read_palette(self):
        palette = array.array('H', 0 for i in range(16))
        with open(self.filename, 'rb') as f:
            f.seek(self.data - self.colors * 4)
            for color in range(self.colors):
                buffer = f.read(4)
                c = color565(buffer[2], buffer[1], buffer[0])
                palette[color] = (c << 8) | (c >> 8)
        return palette

    def read_data(self, offset=0, size=-1):
        with open(self.filename, 'rb') as f:
            f.seek(self.data + offset)
            return f.read(size)


class Bank:
    def __init__(self, buffer=None, palette=None):
        self.buffer = buffer
        self.palette = palette

    @classmethod
    def from_bmp(cls, filename):
        bmp = BMP(filename)
        bmp.read_header()
        if bmp.width != 16 or bmp.height != 256:
            raise ValueError("Not 16x256!")
        palette = bmp.read_palette()
        buffer = bmp.read_data(0, 2048)
        return cls(buffer, palette)

    def pixel(self, tile, x, y):
        b = self.buffer[(tile << 7) + (x << 3) + (y >> 1)]
        c = b & 0x0f if y & 0x01 else b >> 4
        return self.palette[c]


class Layer:
    def __init__(self, bank, width=8, height=8):
        self.buffer = bytearray((width * height) >> 1)
        self.x = 0
        self.y = 0
        self.z = 0
        self.width = width
        self.height = height
        self.bank = bank
        self.layer = layer.Layer(width, height, self.bank.buffer,
                                 self.bank.palette, self.buffer)

    def tile(self, x, y, tile=None):
        if not 0 <= x < self.width or not 0 <= y < self.height:
            return 0
        b = self.buffer[(x * self.width + y) >> 1]
        if tile is None:
            return b & 0x0f if y & 0x01 else b >> 4
        if y & 0x01:
            b = b & 0xf0 | tile
        else:
            b = b & 0x0f | (tile << 4)
        self.buffer[(x * self.width + y) >> 1] = b

    def move(self, x, y):
        self.x = x
        self.y = y
        self.layer.move(x, y)


class Sprite:
    def __init__(self, bank, frame, x, y, z=0, mirror=False):
        self.bank = bank
        self.frame = frame
        self.x = x
        self.y = y
        self.z = z
        self.mirror = mirror
        self.layer = layer.Layer(1, 1, self.bank.buffer, self.bank.palette)
        self.layer.move(x, y)
        self.layer.frame(frame, mirror)
        self.px = x
        self.py = y

    def move(self, x, y):
        self.px = self.x
        self.py = self.y
        self.x = x
        self.y = y
        self.layer.move(x, y)

    def set_frame(self, frame=None, mirror=False):
        if frame is not None:
            self.frame = frame
        self.mirror = mirror
        self.layer.frame(self.frame, mirror)

class Game:
    def __init__(self, fps=6):
        self.layers = []
        dc = digitalio.DigitalInOut(board.DC)
        spi = busio.SPI(clock=board.SCK, MOSI=board.MOSI)
        self.display = ST7735R(spi, dc)
        self.buffer = array.array('H', 0 for i in range(256))
        self.gamepad = gamepad.GamePad(
            digitalio.DigitalInOut(board.X),
            digitalio.DigitalInOut(board.DOWN),
            digitalio.DigitalInOut(board.LEFT),
            digitalio.DigitalInOut(board.RIGHT),
            digitalio.DigitalInOut(board.UP),
            digitalio.DigitalInOut(board.O),
        )
        self.last_tick = time.monotonic()
        self.tick_delay = 1/fps

    def tick(self):
        self.last_tick += self.tick_delay
        wait = max(0, self.last_tick - time.monotonic())
        if wait:
            time.sleep(wait)
        else:
            self.last_tick = time.monotonic()

    def keys(self):
        return self.gamepad.get_pressed()

    def render(self, x0, y0, x1, y1):
        layers = [l.layer for l in self.layers]
        self.display.block(x0, y0, x1 - 1, y1 - 1, b'')
        self.display.dc.value = 1
        layer.render(x0, y0, x1, y1, layers, self.buffer, self.display.spi)
