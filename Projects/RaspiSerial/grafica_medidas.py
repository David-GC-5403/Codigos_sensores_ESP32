import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter

# Cargar los datos desde el archivo CSV
data = pd.read_csv('datos.csv')

# Convertir la columna de tiempo a formato datetime
data['Hora'] = pd.to_datetime(data['Hora'], format='%H:%M:%S')

# Crear una figura con subplots para cada variable
plt.figure(figsize=(10, 8))

# Subplot para Temperatura
plt.subplot(4, 1, 1)
plt.plot(data['Hora'], data['Temperatura'], 'b-o')
plt.ylabel('Temperatura (°C)')
plt.title('Datos en función del tiempo')
plt.grid(True)

# Subplot para TDS
plt.subplot(4, 1, 2)
plt.plot(data['Hora'], data['TDS'], 'r-o')
plt.ylabel('TDS')
plt.grid(True)

# Subplot para PH
plt.subplot(4, 1, 3)
plt.plot(data['Hora'], data['PH'], 'g-o')
plt.ylabel('PH')
plt.grid(True)

# Subplot para Turbidez
plt.subplot(4, 1, 4)
plt.plot(data['Hora'], data['Turbidez'], 'm-o')
plt.ylabel('Turbidez')
plt.xlabel('Tiempo')
plt.grid(True)

# Formatear el eje de tiempo
date_form = DateFormatter("%H:%M:%S")
for i in range(1, 5):
    plt.subplot(4, 1, i)
    plt.gca().xaxis.set_major_formatter(date_form)
    plt.gcf().autofmt_xdate()  # Rota las etiquetas para mejor legibilidad

# Ajustar el layout
plt.tight_layout()
plt.show()