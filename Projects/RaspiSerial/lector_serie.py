import serial
import gpiozero as gpio
import os
import time
import csv

#-------------------------------------- Funciones --------------------------------------#
def setup_pin():
    # Setup GPIO and Serial ports
    gpio_1 = gpio_2 = gpio_3 = None
    

    try:
        gpio_1 = gpio.OutputDevice(PIN_1, initial_value=False)
    except Exception as e:
        print(f"Failed to initialize GPIO 1: {e}")

    try:
        gpio_2 = gpio.OutputDevice(PIN_2, initial_value=False)
    except Exception as e:
        print(f"Failed to initialize GPIO 2: {e}")

    try:
        gpio_3 = gpio.OutputDevice(PIN_3, initial_value=False)
    except Exception as e:
        print(f"Failed to initialize GPIO 3: {e}")

    return gpio_1, gpio_2, gpio_3

def setup_serial():
    puerto_1 = puerto_2 = puerto_3 = None

    try:
        puerto_1 = serial.Serial("/dev/ttyUSB0", 9600, timeout=1)
        puerto_1.close()
    except Exception as e:
        print(f"Failed to initialize Serial Port 1: {e}")

    try:
        puerto_2 = serial.Serial("/dev/ttyUSB1", 9600, timeout=1)
        puerto_2.close()
    except Exception as e:
        print(f"Failed to initialize Serial Port 2: {e}")

    try:
        puerto_3 = serial.Serial("/dev/ttyUSB2", 9600, timeout=1)
        puerto_3.close()
    except Exception as e:
        print(f"Failed to initialize Serial Port 3: {e}")

    return puerto_1, puerto_2, puerto_3 

def setup_datafile(header, name):
    # Crea el archivo si este no existe
    file_exists = os.path.exists(name)
    if file_exists == False:
        archivo = open(name, "w")
        archivo.close()

    # Escribe la cabecera si no existe
    with open(name, "r") as file:
        csvRead = csv.reader(file)
        first_line = next(csvRead, None)  # Lee la primera linea si existe
    with open(name, "a") as file:
        if first_line is None or first_line != header:
            csvwriter = csv.writer(file)
            csvwriter.writerow(header)  # Escribe la cabecera si la linea leida no es la que toca

def writeData(*args):
    # Segun el mensaje recibido, se almacenara la media en una columna u otra del csv
    # Se inicializa el buffer con NaN para evitar errores al escribir en el csv
    row = ["NaN", "NaN", "NaN", "NaN"]
    
    with open("datos.csv", "a") as file:
        writer = csv.writer(file)
        for param in args:
            if len(param) <= 4:  # Si el mensaje tiene 4 o menos valores, no es el sensor que manda medida + temperatura
                if param[1] == "Temperatura":
                    Temp = param[2]
                    row[0] = Temp

                elif param[1] == "TDS":
                    TDS = param[2]
                    row[1] = TDS
                
                elif param[1] == "PH":
                    PH = param[2]
                    row[2] = PH
                
                elif param[1] == "Turbidez":
                    Turbidez = param[2]
                    row[3] = Turbidez
                
                else:
                    print("Medida no reconocida")
                    print(param[1])

            else: # Si el mensaje tiene más de 4 valores, es el sensor que manda medida + temperatura
                # Primero separa el mensaje de forma normal
                if param[1] == "TDS":
                    TDS = param[2]
                    row[1] = TDS
                
                elif param[1] == "PH":
                    PH = param[2]
                    row[2] = PH
                
                elif param[1] == "Turbidez":
                    Turbidez = param[2]
                    row[3] = Turbidez
                
                else:
                    print("Medida no reconocida")
                    print(param[1])
                # Añade al final el valor de la temperatura
                Temp = param[4]
                row[0] = Temp

        row.append(time.strftime("%D:%H:%M:%S"))  # Añade la hora al buffer
        writer.writerow(row)  # Escribe el buffer en el csv
        print(row)  # Imprime el buffer en la consola para ver que se ha escrito correctamente

def read_data(disp, pines, puertos):
    if disp == 1:
        pin_gpio = pines[1]
        serial_port = puertos[1]
    elif disp == 2:
        pin_gpio = gpio_2
        serial_port = puerto_2
    elif disp == 3:
        pin_gpio = gpio_3
        serial_port = puerto_3

    # Enciende el ESP32 y el puerto serie
    pin_gpio.on()
    serial_port.open()

    # Espera a que haya algo en el puerto serie
    while not serial_port.in_waiting > 0:
        pass

    # En este punto ha recibido un mensaje, vamos a leerlo
    pin_gpio.off() # Apaga el pin de encendido, no hace falta ya
    payload = serial_port.readline().decode('utf-8').strip()
    
    # Una vez se ha leido el mensaje, se le indica al controladoe que puede dormirse
    serial_port.write(b"off") # Envía "off" en binario
    serial_port.close() # Cierra el puerto

    return payload


#-------------------------------------- Setup ------------------------------------------#

# Declara los pines para desperar a los ESP32
PIN_1 = 17
PIN_2 = 27
PIN_3 = 22

gpio_1, gpio_2, gpio_3 = setup_pin() # Inicializa los pines
puerto_1, puerto_2, puerto_3 = setup_serial() # Inicializa el puerto serie

fields = ["Temperatura", "TDS", "PH", "Turbidez", "Hora"] # Vector con el header y orden de medidas

setup_datafile(fields, "datos.csv")

pines = [gpio_1, gpio_2, gpio_3]
puertos = [puerto_1, puerto_2, puerto_3]

#---------------------------------- Loop principal -------------------------------------#
while True:

    mensaje_1 = read_data(1, pines, puertos)
    split_1 = mensaje_1.split(";") # El mensaje recibido sera del tipo "xA;Medida;Valor;xZ". Aqui se separa
      

    mensaje_2 = read_data(2)
    split_2 = mensaje_2.split(";")
     
    
    mensaje_3 = read_data(3)
    split_3 = mensaje_3.split(";")

    writeData(split_1,split_2,split_3)

    time.sleep(15*60) # Espera 15 min entre medidas



