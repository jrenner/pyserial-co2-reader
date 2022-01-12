import serial
import time
from logzero import logger
from model import Reading, save_reading

def process_input(inp):
    text = str(inp, encoding='utf-8').strip()
    pieces = text.strip().split(",")
    res = {}
    for p in pieces:
        label, value = p.split(":")
        value = int(value)
        res[label] = value
    return res

def run():
    """

    ser = serial.Serial('/dev/ttyACM0',
            115200,
            timeout=2,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False) 
            """
    ser = serial.Serial("/dev/ttyACM0", 115200, timeout=10)
    ser.flushInput()
    ser.flushOutput()
    while True:
        data_raw = ser.readline()
        reading = process_input(data_raw)
        logger.info(reading)
        save_reading(reading)
        time.sleep(60)


if __name__ == "__main__":
    run()
