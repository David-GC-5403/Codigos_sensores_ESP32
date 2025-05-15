import serial
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


# Indica al programa que pines serán salida
#gpio_1 = gpio.OutputDevice(PIN_1, initial_value=False)
#gpio_2 = gpio.OutputDevice(PIN_2, initial_value=False)
#gpio.OutputDevice(PIN_3, initial_value=False)


# Puertos para la comunicación
puerto_1 = serial.Serial("/dev/ttyUSB2", 9600, timeout=1)
puerto_2 = serial.Serial("/dev/ttyUSB1", 9600, timeout=1)
#puerto_3 = serial.Serial("/dev/ttyUSB2", 9600, timeout=1)
puerto_1.close()  # Cierra los puertos inicialmente
puerto_2.close()  

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

def writeData(*args):
    # Segun el mensaje recibido, se almacenara la media en una columna u otra del csv
    # Se inicializa el buffer con NaN para evitar errores al escribir en el csv
    row = ["NaN", "NaN", "NaN", "NaN"]

    
    with open("datos.csv", "a") as file:
        writer = csv.writer(file)
        for param in args:
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

        row.append(time.strftime("%H:%M:%S"))  # Añade la hora al buffer
        writer.writerow(row)  # Escribe el buffer en el csv
        print(row)  # Imprime el buffer en la consola para ver que se ha escrito correctamente


#---------------------------------- Loop principal -------------------------------------#
while True:

    # Enciende los ESP32 y lee el puerto antes que nada
    #gpio_1.on
    # Mientras el puerto no reciba nada, espera
    puerto_1.open()  # Abre el puerto
    while not puerto_1.in_waiting > 0:
        pass

    # Ya ha recibido algo, apaga el pin de encendido y lee el puerto
    #gpio_1.off
    mensaje = puerto_1.readline().decode('utf-8').strip()  # Lee el puerto
    puerto_1.write(b"off")   # El esp32 espera un mensaje para dormir, en este caso "off"
    puerto_1.close()  # Cierra el puerto  

    # El mensaje recibido sera del tipo "xA;Medida;Valor;xZ"
    split_1 = mensaje.split(";")    # Separa el mensaje
      
    #gpio_2.on
    # Mientras el puerto no reciba nada, espera
    puerto_2.open()  # Abre el puerto
    while not puerto_2.in_waiting > 0:
        pass

    # Ya ha recibido algo, apaga el pin de encendido y lee el puerto
    #gpio_2.off

    mensaje = puerto_2.readline().decode('utf-8').strip()  # Lee el puerto
    puerto_2.write(b"off")   # El esp32 espera un mensaje para dormir, en este caso "off"
    puerto_2.close()  # Cierra el puerto 

    # El mensaje recibido sera del tipo "xA;Medida;Valor;xZ"
    split_2 = mensaje.split(";")
     

    """""
    gpio.OutputDevice.on(PIN_3)
    mensaje = readSerial(puerto_3)
    gpio.OutputDevice.off(PIN_3)
    split_3 = mensaje.split(";")

    # En este punto los ESP32 estan dormidos, podemos tratar los datos con tranquilidad
    """

    writeData(split_1,split_2)
    #writeData(split_2)
    # writeData(split_3)

    time.sleep(1)



