#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import multiprocessing
import recepcion_mqtt_CSV.py  # Script that collects data from MQTT and saves it
import grafica_csv.py    # Script that reads the file and plots data

if __name__ == "__main__":
    p1 = multiprocessing.Process(target=recepcion_mqtt_CSV.main)  
    p2 = multiprocessing.Process(target=grafica_csv.main)  

    p1.start()
    p2.start()

    p1.join()
    p2.join()
