from serial import Serial as serial
import gpiozero as gpio
import os
import time
import csv
import io

#-------------------------------------- Setup ------------------------------------------#

# Declara los pines para desperar a los ESP32
PIN_1 = 17
PIN_2 = 27
PIN_3 = 22

# Esto es una prueba


# Indica al programa que pines serán salida
gpio_1 = gpio.OutputDevice(PIN_1, initial_value=False)
#gpio.OutputDevice(PIN_2, initial_value=False)
#gpio.OutputDevice(PIN_3, initial_value=False)


# Puertos para la comunicación
puerto_1 = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)
#puerto_2 = serial.Serial("/dev/ttyUSB1", 9600, timeout=1)
#puerto_3 = serial.Serial("/dev/ttyUSB2", 9600, timeout=1)

# Los puertos se abren nada mas declararlos, pero los quiero cerrados inicialmente.
puerto_1.close()
#puerto_2.close()
#puerto_3.close()

# Configuracion del archivo donde se guardara la info
file_exists = os.path.exists("data.csv")
if file_exists == False:
    archivo = open("datos.csv", "w")
    archivo.close()

fields = ["Temperatura", "TDS", "PH", "Turbidez", "Hora"]

with open("datos.csv", "r") as file:
    csvRead = csv.reader(file)
    first_line = next(csvRead, None)  # Read the first line if it exists
with open("datos.csv", "a") as file:
    if first_line is None or first_line != fields:
        csvwriter = csv.writer(file)
        csvwriter.writerow(fields)  # Write the header



#-------------------------------------- Funciones --------------------------------------#

def readSerial(puerto):
    try:
        puerto.open()
        payload = puerto.readline().decode('utf-8').strip()
        puerto.close()

        return payload
    except:
        print("Error al abrir el puerto")
        return


def writeData(msg_dividido):
    # Segun el mensaje recibido, se almacenara la media en una columna u otra del csv
    # Se inicializa el buffer con NaN para evitar errores al escribir en el csv
    buffer = ["NaN", "NaN", "NaN", "NaN"]

    # Abre el archivo para escribir
    with open("datos.csv", "a") as file:
        writer = csv.writer(file)

        if msg_dividido[0] == "Temperatura":
            Temp = msg_dividido[1]
            buffer[0] = Temp
        
        elif msg_dividido[0] == "TDS":
            TDS = msg_dividido[1]
            buffer[1] = TDS
        
        elif msg_dividido[0] == "PH":
            PH = msg_dividido[1]
            buffer[2] = PH
        
        elif msg_dividido[0] == "Turbidez":
            Turbidez = msg_dividido[1]
            buffer[3] = Turbidez
        
        else:
            print("Medida no reconocida")


        buffer.append(time.strftime("%H:%M:%S"))  # Añade la hora al buffer
        writer.writerow(buffer)  # Escribe el buffer en el csv
        print(buffer)  # Imprime el buffer en la consola para ver que se ha escrito correctamente
        # Cierra el archivo
        file.close()

#---------------------------------- Loop principal -------------------------------------#
while True:

    # Enciende los ESP32 y lee el puerto antes que nada
    gpio.OutputDevice.on(gpio_1)

    # Mientras el puerto no reciba nada, espera
    while serial.readline() == 0:
        pass

    # Ya ha recibido algo, apaga el pin de encendido y lee el puerto
    gpio.OutputDevice.off(PIN_1)
    mensaje = readSerial(puerto_1)

    serial.write(puerto_1, "off")   # El esp32 espera un mensaje para dormir, en este caso "off"

    split_1 = mensaje.split(";")    # Separa el mensaje

    """""
    gpio.OutputDevice.on(PIN_2)
    mensaje = readSerial(puerto_2)
    gpio.OutputDevice.off(PIN_2)
    split_2 = mensaje.split(";")


    gpio.OutputDevice.on(PIN_3)
    mensaje = readSerial(puerto_3)
    gpio.OutputDevice.off(PIN_3)
    split_3 = mensaje.split(";")

    # En este punto los ESP32 estan dormidos, podemos tratar los datos con tranquilidad
    """

    writeData(split_1)
    # writeData(split_2)
    # writeData(split_3)

    time.sleep(10)

    

