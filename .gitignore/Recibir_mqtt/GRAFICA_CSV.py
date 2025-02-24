import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import csv
import datetime

escala = 20
horas = deque(maxlen=escala)
deformaciones = deque(maxlen=escala)

# Function to read data from file
def leer_datos(archivo):
    nuevas_horas = deque(maxlen=escala)
    nuevas_deformaciones = deque(maxlen=escala)
    
    try:
        with open(archivo, 'r') as file:
            csvreader = csv.reader(file)
            next(csvreader, None)
            
            for line in csvreader:
                if len(line) == 2:
                    nuevas_horas.append(line[1])  # Time (string)
                    nuevas_deformaciones.append(float(line[0]))  # Deformation (float)

    except Exception as e:
        print(f"Ocurrió un error al leer el archivo: {e}")
    
    return nuevas_horas, nuevas_deformaciones

# Function to update graph
def actualiza_grafica(i, horas, deformaciones):
    # Read new data
    nuevas_horas, nuevas_deformaciones = leer_datos("datos.csv")

    if nuevas_horas and nuevas_deformaciones:
        horas.clear()
        deformaciones.clear()
        horas.extend(nuevas_horas)
        deformaciones.extend(nuevas_deformaciones)

    # Debug: Print values to check if they are updating
    print("Horas:", list(horas))
    print("Deformaciones:", list(deformaciones))

    # Clear and replot
    ax.clear()
    ax.plot(list(horas), list(deformaciones), marker='o', linestyle='-')

    # Force Matplotlib to refresh
    plt.draw()

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('Gráfica Temperatura')
    plt.ylabel("Temperatura")

# Initialize figure
fig, ax = plt.subplots()

# Set window title
fig.canvas.manager.set_window_title("Grafica en tiempo real")

# Set up animation (with fargs to pass arguments)
ani = animation.FuncAnimation(fig, actualiza_grafica, fargs=(horas, deformaciones), interval=2000, blit=False)

# Keep reference to `ani` to prevent garbage collection
plt.show()


