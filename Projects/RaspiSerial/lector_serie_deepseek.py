import serial  # Correct import (not "from serial import Serial as serial")
import gpiozero as gpio
import os
import time
import csv

#-------------------------------------- Setup ------------------------------------------#
PIN_1 = 17
gpio_1 = gpio.OutputDevice(PIN_1, initial_value=False)

# Open the serial port (automatically opens when initialized)
puerto_1 = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)
puerto_1.close()  # Close it initially

# CSV setup (same as before)
file_exists = os.path.exists("data.csv")
if not file_exists:
    with open("datos.csv", "w") as archivo:
        pass  # Just create the file

fields = ["Temperatura", "TDS", "PH", "Turbidez", "Hora"]
with open("datos.csv", "r") as file:
    csvRead = csv.reader(file)
    first_line = next(csvRead, None)
with open("datos.csv", "a") as file:
    if first_line is None or first_line != fields:
        csv.writer(file).writerow(fields)

#-------------------------------------- Functions --------------------------------------#
def readSerial(puerto):
    try:
        puerto.open()
        payload = puerto.readline().decode('utf-8').strip()  # Read from the port instance
        puerto.close()
        return payload
    except Exception as e:
        print(f"Error al abrir el puerto: {e}")
        return None

def writeData(msg_dividido):
    buffer = ["NaN"] * 4
    with open("datos.csv", "a") as file:
        writer = csv.writer(file)
        
        if msg_dividido[0] == "Temperatura":
            buffer[0] = msg_dividido[1]
        elif msg_dividido[0] == "TDS":
            buffer[1] = msg_dividido[1]
        elif msg_dividido[0] == "PH":
            buffer[2] = msg_dividido[1]
        elif msg_dividido[0] == "Turbidez":
            buffer[3] = msg_dividido[1]
        else:
            print("Medida no reconocida")

        buffer.append(time.strftime("%H:%M:%S"))
        writer.writerow(buffer)
        print(buffer)

#---------------------------------- Loop principal -------------------------------------#
while True:
    gpio_1.on()  # Turn on ESP32

    # Wait for data (check if bytes are available)
    while puerto_1.in_waiting == 0:  # Correct way to check for data
        pass

    gpio_1.off()  # Turn off ESP32
    mensaje = readSerial(puerto_1)

    if mensaje:
        puerto_1.write(b"off")  # Send "off" to ESP32 (must be bytes)
        split_1 = mensaje.split(";")
        writeData(split_1)

    time.sleep(10)