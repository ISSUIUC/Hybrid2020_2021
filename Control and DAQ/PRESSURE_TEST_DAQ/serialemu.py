import math
from random import random


class SerialEMU:
    def __init__(self):
        self.val = 0

    def flush(self):
        pass

    def write(self, bytes):
        print(bytes.decode("utf8"));

    def readline(self):
        self.val += 0.1;
        return f"{self.val}\t{math.sin(self.val)}\t3\t4\r\n".encode("utf8")