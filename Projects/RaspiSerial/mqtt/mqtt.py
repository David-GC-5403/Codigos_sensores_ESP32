import csv
import os
import datetime

# Configuracion del archivo donde se guardara la info
file_exists = os.path.exists("datos.csv")
if file_exists == False:
    archivo = open("datos.csv", "w")
    archivo.close()
    

    
fields = ["Temperatura", "Hora"]

with open("datos.csv", "r") as file:
    csvRead = csv.reader(file)
    
    first_line = next(csvRead, None)  # Read the first line if it exists
with open("datos.csv", "a") as file:
    if first_line is None or first_line != fields:
        csvwriter = csv.writer(file)
        csvwriter.writerow(fields)  # Write the header

# Callback cuando se recibe un mensaje
def on_message(client, userdata, msg):
    print(f"Mensaje recibido en {msg.topic}: {msg.payload.decode()}")
    payload = msg.payload.decode()
    payload_lista = payload.split(" ")

    temperatura = float(payload_lista[0])
    hora = datetime.datetime.strptime(payload_lista[1], "%H:%M:%S").time()
    
    guardar = [temperatura, hora]
    with open("datos.csv", "a") as file:
        csvwriter = csv.writer(file)
        csvwriter.writerow(guardar)

