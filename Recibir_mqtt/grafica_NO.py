import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

escala = 20

# This function is called periodically from FuncAnimation
def actualiza_grafica(i, horas, deformaciones):    
    horas, deformaciones = leer_datos("datos.txt")
    
    # Draw x and y lists
    ax.clear()
    ax.plot(horas, deformaciones)

    # Format plot
    plt.xticks(rotation=45, ha='right') # Gira los valores del eje x para que 
                                        # se lean bien
    plt.subplots_adjust(bottom=0.30)
    plt.title('Grafica')
    plt.ylabel("Deformacion")

# Función para leer los datos desde el archivo
def leer_datos(archivo):
    horas = deque(maxlen=escala)
    deformaciones = deque(maxlen=escala)
    
    try:
        with open(archivo, 'r') as file:
            for line in file:
                partes = line.split()
                if len(partes) == 2:  # Ensure there are exactly 2 values per line
                    horas.append(partes[1])
                    deformaciones.append(float(partes[0]))

    except Exception as e:
        print(f"Ocurrió un error al leer el archivo: {e}")
    
    return list(horas), list(deformaciones)


horas, deformaciones = leer_datos("datos.txt")
print(horas)
print(deformaciones)

fig, ax = plt.subplots()

ani = animation.FuncAnimation(fig, actualiza_grafica, fargs =(horas, deformaciones), interval=5000)
plt.show()



