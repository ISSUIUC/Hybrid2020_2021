import asyncio
from datetime import date
from json import JSONEncoder
import sys, time, serial, matplotlib, random
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from collections import deque
from serialemu import SerialEMU
from server import TelemetryServer

import numpy as np

try:
    import tkinter as tk
except ImportError:
    import Tkinter as tk

matplotlib.style.use("seaborn")


def init_gui():

    global root

    global p1_box
    global p2_box
    global p3_box

    global plot_size
    global x_values
    global p1_values
    global p2_values
    global p3_values
    global fig
    global ax1
    global line1
    global line2
    global line3

    global daq_toggle_button

    plot_size = 1000

    x_values = [i for i in range(plot_size)]
    p1_values = deque(np.zeros(plot_size))
    p2_values = deque(np.zeros(plot_size))
    p3_values = deque(np.zeros(plot_size))

    # plt.ion()
    # fig = plt.figure(figsize=(20,7))
    fig = plt.figure(figsize=(15,5)) #may need to change depending on computer screen size
    ax1 = fig.add_subplot(111)
    ax1.set_title("Pressure Readings")
    ax1.set_xlim(0, plot_size)
    ax1.set_ylim(0, 1100)
    ax1.set_yticks([0, 100, 200, 300, 400, 500, 600, 700, 800,900,1000,1100])
    ax1.axhline(8192, color="r", linestyle="--", linewidth=0.7)
    line1, = ax1.plot(x_values, p1_values, 'r-', linewidth=0.6)
    line2, = ax1.plot(x_values, p2_values, 'g-', linewidth=0.6)
    line3, = ax1.plot(x_values, p3_values, 'b-', linewidth=0.6)
    # plt.grid()

    root = tk.Tk()
    root.protocol("WM_DELETE_WINDOW", _quit)
    root.wm_title("ISS HYBRID DAQ SYSTEM")

    primary_frame = tk.Frame(root)

    canvas = FigureCanvasTkAgg(fig, master=primary_frame)  # A tk.DrawingArea.
    canvas.draw()
    canvas.get_tk_widget().grid(row=2, column = 1, columnspan = 2)

    pressure_frame = tk.Frame(primary_frame)

    p1_label = soc_label = tk.Label(pressure_frame, text="Pressure 1", font=("Arial", 15, "bold"))
    p1_label.grid(row=1, column=1, columnspan=1)
    p1_box = tk.Text(pressure_frame, height = 3, width = 20)
    p1_box.insert(tk.END, ("0"))
    p1_box.tag_add("center", 1.0, "end")
    p1_box.tag_configure("center", justify="center")
    p1_box.configure(font=("Arial", 20))
    p1_box.grid(row = 1, column = 2, columnspan=1, padx = 30, pady = 5)

    p2_label = soc_label = tk.Label(pressure_frame, text="Pressure 2", font=("Arial", 15, "bold"))
    p2_label.grid(row=2, column=1, columnspan=1)
    p2_box = tk.Text(pressure_frame, height = 3, width = 20)
    p2_box.insert(tk.END, ("0"))
    p2_box.tag_add("center", 1.0, "end")
    p2_box.tag_configure("center", justify="center")
    p2_box.configure(font=("Arial", 20))
    p2_box.grid(row = 2, column = 2, columnspan=1, padx = 30, pady = 5)

    p3_label = soc_label = tk.Label(pressure_frame, text="Pressure 3", font=("Arial", 15, "bold"))
    p3_label.grid(row=3, column=1, columnspan=1)
    p3_box = tk.Text(pressure_frame, height = 3, width = 20)
    p3_box.insert(tk.END, ("0"))
    p3_box.tag_add("center", 1.0, "end")
    p3_box.tag_configure("center", justify="center")
    p3_box.configure(font=("Arial", 20))
    p3_box.grid(row = 3, column = 2, columnspan=1, padx = 30, pady = 5)

    pressure_frame.grid(row = 1, column = 2)

    serial_frame = tk.Frame(primary_frame)

    daq_toggle_button = tk.Button(serial_frame, text="TOGGLE DAQ", height=3, width = 20, command=lambda: daq_toggle())
    daq_toggle_button.config({"background" : "red"})
    daq_toggle_button.config({"justify" : "center"})
    daq_toggle_button.grid(row=2, column=1)

    serial_frame.grid(row=1, column=1)

    primary_frame.pack()

def close_ball_valve():
    global ser
    global BALL_VALVE_OPEN
    
    ser.write("a".encode("utf-8"))
    daq_toggle_button.config({"background" : "red"})
    daq_toggle_button.config({"justify" : "center"})
    BALL_VALVE_OPEN = False

def open_ball_valve():
    global ser
    global BALL_VALVE_OPEN

    ser.write("b".encode("utf-8"))
    daq_toggle_button.config({"background" : "green3"})
    daq_toggle_button.config({"justify" : "center"})
    BALL_VALVE_OPEN = True

def daq_toggle():
    global BALL_VALVE_OPEN

    if BALL_VALVE_OPEN:
        close_ball_valve()
    else:
        open_ball_valve()

def update_plot(new_p1, new_p2, new_p3):

    p1_values.popleft()
    p2_values.popleft()
    p3_values.popleft()

    if new_p1 == None:
        p1_values.append(p1_values[len(p1_values)-1])
    else:
        p1_values.append(new_p1)

    if new_p2 == None:
        p2_values.append(p2_values[len(p2_values)-1])
    else:
        p2_values.append(new_p2)

    if new_p3 == None:
        p3_values.append(p3_values[len(p3_values)-1])
    else:
        p3_values.append(new_p3)

    line1.set_ydata(p1_values)
    line2.set_ydata(p2_values)
    line3.set_ydata(p3_values)

    fig.canvas.draw()

    fig.canvas.flush_events()

    p1_box.delete("1.0", tk.END)
    p1_box.insert(tk.END, str(new_p1))
    p1_box.tag_add("center", 1.0, "end")
    p2_box.delete("1.0", tk.END)
    p2_box.insert(tk.END, str(new_p2))
    p2_box.tag_add("center", 1.0, "end")
    p3_box.delete("1.0", tk.END)
    p3_box.insert("1.0", str(new_p3))
    p3_box.tag_add("center", 1.0, "end")


def _quit():
    root.quit()
    root.destroy()
    sys.exit()

out_file = open(f"${date.today()}Data.csv", "at")

async def mcu_loop():
    while True:
        await asyncio.sleep(0.01)
        try:
            data = str(ser.readline()[:-2].decode("utf-8"))
            ser.flush()
            if data:
                tStamp, val1, val2, val3 = data.split("\t")
                a = 1.36089
                b = 206.801
                val2 = float(val2)*a+b
                val3 = float(val3)*a+b
                out_file.write(f"{tStamp},{val1},{val2},{val3}\n")
                out_file.flush()

                update_plot(float(val1), float(val2), float(val3))
                telem_server.update_data(JSONEncoder().encode({'Force': val1, 'Pressure1': val2, 'Pressure2': val3, 'BallVale': "Open" if BALL_VALVE_OPEN else "Closed"}))

                ser.flush()

            else:
                update_plot(None, None, None)

        except ValueError:
            ser.flush()


if __name__ == "__main__":

    global ser
    global BALL_VALVE_OPEN

    BALL_VALVE_OPEN = False

    init_gui()

    update_plot(0, 0, 0)

    # ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    ser = serial.Serial("COM4", 9600, timeout=1) # Who use this code should change the port name.

    # ser = SerialEMU()
    telem_server = TelemetryServer() 
    telem_server.add_close_listener(close_ball_valve)
    telem_server.add_open_listener(open_ball_valve)

    try:
        loop = asyncio.get_event_loop()
        loop.create_task(telem_server.start(80))
        loop.create_task(mcu_loop())
        loop.run_forever();
    except KeyboardInterrupt:
        sys.exit()
