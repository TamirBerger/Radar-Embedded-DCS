import serial as ser
import numpy as np
import matplotlib.pyplot as plt
import tkinter as tk
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from tkinter import filedialog
import time
import threading

light_arr_distances = [0] * 50
size = 60   # number of samples during scan
AllTasks = []
AllTasksbytes = []
flag_state2 = 0

def create_radar_ldr_plot(ldr_list, mask_distance):

    angles = np.linspace(0, 180, size)  # Generate angles from 0 to 180 degrees
    distances = []
    for i in range(0,size):
        distances.append(1000)

    for index, distance in ldr_list:
        distances[int(index)] = int(distance)

    # Filter the distances to exclude values outside the mask distance
    distances = [dist if dist <= mask_distance else np.nan for dist in distances]

    fig = plt.figure(figsize=(8, 6))
    ax = fig.add_subplot(111, polar=True)

    # Plot the radar points
    ax.plot(np.radians(angles), distances, marker='o', markersize=6, linestyle='-', linewidth=2)

    # Set the maximum distance as the maximum value in the distances list (excluding masked values)
    valid_distances = [dist for dist in distances if not np.isnan(dist)]
    if len(valid_distances) > 0:
        max_distance = max(valid_distances)
    else:
        max_distance = 50
    ax.set_ylim(0, max_distance + 10)

    # Set the radar angle grid and labels
    ax.set_thetagrids(np.arange(0, 181, 10), labels=np.arange(0, 181, 10))  # Corrected range for 180 degrees
    ax.set_rlabel_position(0)

    # Set radial offset for maximum radius labels
    label_offset = 10  # Adjust this value to control the offset
    for label, angle in zip(ax.get_yticklabels(), range(0, 181, 10)):
        if angle == 90:  # For the label at 90 degrees (max radius), move it away from the center
            label.set_position((label.get_position()[0], label.get_position()[1] + label_offset))

    # Set grid lines and title
    ax.grid(True)
    plt.title("Radar Plot of Distances (in cm)", fontsize=16)

    plt.show()

def create_radar_plot(distances_list, mask_distance, min_angle = 0, max_angle = 180):
    angles = np.linspace(min_angle, max_angle, len(distances_list))  # Generate angles from 0 to 180 degrees
    distances = [int(distance) for distance in distances_list]  # Convert the list of strings to integers

    # Filter the distances to exclude values outside the mask distance
    distances = [dist if dist <= mask_distance else np.nan for dist in distances]

    fig = plt.figure(figsize=(8, 6))
    ax = fig.add_subplot(111, polar=True)

    # Plot the radar points
    ax.plot(np.radians(angles), distances, marker='o', markersize=6, linestyle='-', linewidth=2)

    # Set the maximum distance as the maximum value in the distances list (excluding masked values)
    valid_distances = [dist for dist in distances if not np.isnan(dist)]
    if len(valid_distances) > 0:
        max_distance = max(valid_distances)
    else:
        max_distance = 50
    ax.set_ylim(0, max_distance + 10)

    # Set the radar angle grid and labels
    ax.set_thetagrids(np.arange(min_angle, max_angle+1, 10), labels=np.arange(min_angle, max_angle + 1, 10))  # Corrected range for 180 degrees
    ax.set_rlabel_position(0)

    # Set radial offset for maximum radius labels
    label_offset = 10  # Adjust this value to control the offset
    for label, angle in zip(ax.get_yticklabels(), range(min_angle, max_angle + 1, 10)):
        if angle == 90:  # For the label at 90 degrees (max radius), move it away from the center
            label.set_position((label.get_position()[0], label.get_position()[1] + label_offset))

    # Set grid lines and title
    ax.grid(True)
    plt.title("Radar Plot of Distances (in cm)", fontsize=16)

    plt.show()


def encode_instruction(instruction, operand1=None, operand2=None):
    opcodes = {
        'inc_lcd': '01',
        'dec_lcd': '02',
        'rra_lcd': '03',
        'set_delay': '04',
        'clear_lcd': '05',
        'servo_deg': '06',
        'servo_scan': '07',
        'sleep': '08'
    }

    if instruction in opcodes:
        opcode = opcodes[instruction]
        if operand1 is not None and operand2 is None:
            if opcode == "03":
                operand1 = chr(operand1)
                return f'{opcode}{ord(operand1):02X}'
            else:
                return f'{opcode}{operand1:02X}'
        elif operand1 is None and operand2 is None:
            return opcode
        elif operand1 is not None and operand2 is not None:
            if opcode == 3:
                return f'{opcode}{ord(operand1):02X}{ord(operand2):02X}'
            else:
                return f'{opcode}{operand1:02X}{operand2:02X}'
    else:
        return None


def UploadFile(event=None):
    UploadFile.counter = UploadFile.counter + 1
    filename = filedialog.askopenfilename()
    # print('Selected:', filename)
    fileObj = open(filename, "r")  # opens the file in read mode
    lines = fileObj.read().splitlines()  # puts the file into an array

    encoded_lines = []
    for line in lines:
        parts = line.split()
        instruction = parts[0]
        operand1 = None
        operand2 = None
        if len(parts) > 1:
            operands = parts[1].split(',')
            operand1 = int(operands[0])
            if len(operands) > 1:
                operand2 = int(operands[1])
        encoded = encode_instruction(instruction, operand1, operand2)
        if encoded is not None:
            encoded_lines.append(encoded)

    AllTasks.extend('s')
    AllTasks.extend(encoded_lines)
    if UploadFile.counter == 3:
        AllTasks.extend('l')
        print('AllTasks')
    else:
        AllTasks.extend('f')

    fileObj.close()


def option1(mask_distance_entry, s):
    idx = 0
    distances_list = []
    mask_distance = int(mask_distance_entry.get())

    while (idx < size):

        while (s.in_waiting > 0):  # while the input buffer isn't empty
            line = s.readline()  # read  from the buffer until the terminator is received,
                                 # readline() can also be used if the terminator is '\n'
                                 # line = s.read_until(terminator='\n')

            print(line.decode("ascii"))
            distances_list.append(line.decode("ascii"))
            idx = idx + 1

            if (s.in_waiting == 0):
                outChar = 'a'    # ack
                bytesChar = bytes(outChar, 'ascii')
                s.write(bytesChar)

    create_radar_plot(distances_list, mask_distance)


def option2(s, flag_state2=None):
    def on_closing():
        outChar = 'e'  # Send 'e' to the controller when the Telemeter window is closed.
                       # cotroller will change state to state0 of its FSM
        bytesChar = bytes(outChar, 'ascii')
        s.write(bytesChar)
        telemeter_window.destroy()

    def update_distance():
        if s.in_waiting > 0:
            line = s.readline()
            distance = line.decode("ascii").strip()  # Get the distance value from the controller
            distance_label.config(text=f"Distance: {distance} cm")  # Update the distance label
            # send ack
            outChar = 'a'  # ack
            bytesChar = bytes(outChar, 'ascii')
            s.write(bytesChar)

        telemeter_window.after(100, update_distance)  # Repeat the update every 100 ms

    def set_angle():
        angle = angle_entry.get()
        angle_bytes = bytes(angle + '\n', 'ascii')
        s.write(angle_bytes)  # Send the desired angle to the controller

    # Create the Telemeter window
    telemeter_window = tk.Toplevel()
    telemeter_window.title("Telemeter")

    angle_label = tk.Label(telemeter_window, text="Enter Desired Angle:")
    angle_label.pack()

    angle_entry = tk.Entry(telemeter_window)
    angle_entry.pack()

    set_angle_button = tk.Button(telemeter_window, text="Set Angle", command=set_angle)
    set_angle_button.pack()

    distance_label = tk.Label(telemeter_window, text="Distance: N/A cm")
    distance_label.pack()

    # Start updating the distance in real-time
    telemeter_window.after(100, update_distance)

    # Handle window closing event
    telemeter_window.protocol("WM_DELETE_WINDOW", on_closing)


def option3(mask_distance_entry, s):  # light ditector
    flag = 0
    ldr_list = []
    mask_distance = int(mask_distance_entry.get())
    idx = 0
    idx_dist_flag = 0

    while (idx < 2*size):

        while (s.in_waiting > 0):  # while the input buffer isn't empty
            line = s.readline()  # read  from the buffer until the terminator is received,
                                 # readline() can also be used if the terminator is '\n'
                                 # line = s.read_until(terminator='\n')
            print(line.decode("ascii"))

            if line == b'999':
                idx = 3*size
                break
            #distances_list.append(line.decode("ascii"))
            if idx_dist_flag == 0:
                index = line.decode("ascii")
            else:
                ldr_val = line.decode("ascii")
                distance = convert_value(int(ldr_val), light_arr_distances)
                tup = (index, distance)
                ldr_list.append(tup)

            idx_dist_flag = not idx_dist_flag

            if (s.in_waiting == 0):
                outChar = 'a'    # ack
                bytesChar = bytes(outChar, 'ascii')
                s.write(bytesChar)

    create_radar_ldr_plot(ldr_list, mask_distance)


def option4():
    pass


def option5(s):
    def close_window_and_continue():
        for i in range(0, len(AllTasks)):
            for char in AllTasks[i]:
                bytesChar = bytes(char, 'ascii')
                s.write(bytesChar)
                time.sleep(0.15)
        flag_state5 = 1
        AllTasks.clear()
        print('Acknowledge of loading the script')
        file_window.destroy()  # Close the "file" window
        # read_line5()

    # Create the script window
    file_window = tk.Toplevel()
    file_window.title("file")

    button = tk.Button(file_window, text='Select file', command=UploadFile)
    button.pack()

    continue_button = tk.Button(file_window, text='continue', command=close_window_and_continue)
    continue_button.pack()

    # Start the separate thread

    file_window.mainloop()


def close_window_and_continue(s,file_window):
    for i in range(0, len(AllTasks)):
        for char in AllTasks[i]:
            bytesChar = bytes(char, 'ascii')
            s.write(bytesChar)
            time.sleep(0.15)
    flag_state5 = 1
    AllTasks.clear()
    print('Acknowledge of loading the script')
    file_window.destroy()  # Close the "file" window
    #read_line5()


def OPC_6(s):

    def on_closing5():
        telemeter_window.destroy()
        # send end msg
        outChar = 'a'  # ack
        bytesChar = bytes(outChar, 'ascii')
        s.write(bytesChar)
        print("Acknowledge end of OPC6 and change of flag od opc6")
        read_line5(s)

    distance = " "

    # Create the Telemeter window
    telemeter_window = tk.Toplevel()
    telemeter_window.title("Telemeter")

    distance_label = tk.Label(telemeter_window, text=f"Distance: {distance} cm")
    distance_label.pack()

    while s.in_waiting == 0:
        pass
    if s.in_waiting > 0:
        line = s.readline()
        distance = line.decode("ascii").strip()  # Get the distance value from the controller

    distance_label.config(text=f"Distance: {distance} cm")  # Update the distance label

    # send msg of dist
    outChar = 'a'  # ack
    bytesChar = bytes(outChar, 'ascii')
    s.write(bytesChar)
    print("Acknowledge of get distance in OPC6")

    # Handle window closing event
    telemeter_window.protocol("WM_DELETE_WINDOW", on_closing5)

    telemeter_window.mainloop()


def send_ack_script(s, option_var,script_window):
    selected_option = option_var.get()
    option_number = selected_option.split(".")[0]  # Extract the number from the selected option
    bytesChar = bytes(option_number, 'ascii')
    s.write(bytesChar)  # send the required state to microcontroller

    script_window.destroy()
    selected_option = option_var.get()
    option_number = selected_option.split(".")[0]  # Extract the number from the selected option
    bytesChar = bytes(option_number, 'ascii')
    s.write(bytesChar)  # send the required state to microcontroller

    script_window.destroy()
    read_line5(s)


def read_line5(s):
    line = '0'
    option_number = '0'
    flag5 = 0

    while line != "f" and line != "l":

        if s.in_waiting > 0:  # in opcode 6 or 7
            line = s.readline()
            option_number = line.decode("ascii").strip()  # Get the distance value from the controller

            # send ack
            outChar = 'a'  # ack
            bytesChar = bytes(outChar, 'ascii')
            s.write(bytesChar)
            print("Acknowledge get 6 or 7 opc")

        if option_number == '008':
            break

        if option_number == "006":
            OPC_6(s)
            option_number = '0'

        elif option_number == "007":
            idx = 0
            angle = []
            while (idx < 2):

                while (s.in_waiting > 0):  # while the input buffer isn't empty
                    line = s.readline()  # read  from the buffer until the terminator is received,
                    # readline() can also be used if the terminator is '\n'
                    # line = s.read_until(terminator='\n')

                    print(line.decode("ascii"))
                    angle.append(line.decode("ascii"))
                    idx = idx + 1

                    if (s.in_waiting == 0):
                        outChar = 'a'  # ack
                        bytesChar = bytes(outChar, 'ascii')
                        s.write(bytesChar)

            angle[0] = int(angle[0])
            angle[1] = int(angle[1])
            size_7 = (int(angle[1]) - angle[0]) // (180 / size)
            idx = 0
            distances_list = []
            while (idx < size_7):

                while (s.in_waiting > 0):  # while the input buffer isn't empty
                    line = s.readline()  # read  from the buffer until the terminator is received,
                    # readline() can also be used if the terminator is '\n'
                    # line = s.read_until(terminator='\n')

                    print(line.decode("ascii"))
                    distances_list.append(line.decode("ascii"))
                    idx = idx + 1

                    if (s.in_waiting == 0):
                        outChar = 'a'  # ack
                        bytesChar = bytes(outChar, 'ascii')
                        s.write(bytesChar)

            create_radar_plot(distances_list, 450, angle[0], angle[1])
            option_number = '0'


def option6(s):

    # Create the script window
    script_window = tk.Toplevel()
    script_window.title("Choose Script:")

    # Option selection dropdown
    option_var = tk.StringVar(script_window)
    option_var.set("Select a script:")  # Default value
    options = ["1.script 1", "2.script 2", "3. script 3"]
    option_menu = tk.OptionMenu(script_window, option_var, *options)
    option_menu.pack()

    # "Select" button
    Select_button = tk.Button(script_window, text="Select",
                              command=lambda: send_ack_script(s, option_var,script_window))
    Select_button.pack()

    script_window.mainloop()


def on_option_selected(root, option_var, mask_distance_entry, s):
    selected_option = option_var.get()
    option_number = selected_option.split(".")[0]  # Extract the number from the selected option
    bytesChar = bytes(option_number, 'ascii')
    s.write(bytesChar)  # send the required state to microcontroller

    if option_number == "1":
        option1(mask_distance_entry, s)
    elif option_number == "2":
        #flag_state2 = 1
        option2(s)
    elif option_number == "3":
        option3(mask_distance_entry, s)
    elif option_number == "4":
        option4()
    elif option_number == "5":
        option5(s)
        print("hi")
    elif option_number == "6":
        option6(s)


def fill_mat(light_samples, light_arr_distances):
        for k in range(10):
            for i in range(5):
                light_arr_distances[5 * k + i] = light_samples[k] + (light_samples[k + 1] - light_samples[k]) // (5 - i)


def find_closest_index(value, distances):
    return min(range(len(distances)), key=lambda i: abs(distances[i] - value))


def convert_value(ldr_code, light_arr_distances):
    converted_value = find_closest_index(ldr_code, light_arr_distances)
    return converted_value


def create_radar_ldr_plot(ldr_list, mask_distance):

    angles = np.linspace(0, 180, size)  # Generate angles from 0 to 180 degrees
    distances = []
    for i in range(0,size):
        distances.append(1000)
    #distances = [int(1000) for distance in distances_list]  # Convert the list of strings to integers

    for index, distance in ldr_list:
        distances[int(index)] = int(distance)

    # Filter the distances to exclude values outside the mask distance
    distances = [dist if dist <= mask_distance else np.nan for dist in distances]

    fig = plt.figure(figsize=(8, 6))
    ax = fig.add_subplot(111, polar=True)

    # Plot the radar points
    ax.plot(np.radians(angles), distances, marker='o', markersize=6, linestyle='-', linewidth=2)

    # Set the maximum distance as the maximum value in the distances list (excluding masked values)
    valid_distances = [dist for dist in distances if not np.isnan(dist)]
    if len(valid_distances) > 0:
        max_distance = max(valid_distances)
    else:
        max_distance = 50
    ax.set_ylim(0, max_distance + 10)

    # Set the radar angle grid and labels
    ax.set_thetagrids(np.arange(0, 181, 10), labels=np.arange(0, 181, 10))  # Corrected range for 180 degrees
    ax.set_rlabel_position(0)

    # Set radial offset for maximum radius labels
    label_offset = 10  # Adjust this value to control the offset
    for label, angle in zip(ax.get_yticklabels(), range(0, 181, 10)):
        if angle == 90:  # For the label at 90 degrees (max radius), move it away from the center
            label.set_position((label.get_position()[0], label.get_position()[1] + label_offset))

    # Set grid lines and title
    ax.grid(True)
    plt.title("Radar Plot of Distances (in cm)", fontsize=16)

    plt.show()


def main():
    light_samples = [0,260,270,280,310,410,425,470,490,505,600]

    # Call the function
    fill_mat(light_samples, light_arr_distances)

    s = ser.Serial('COM7', baudrate=9600, bytesize=ser.EIGHTBITS,
                   parity=ser.PARITY_NONE, stopbits=ser.STOPBITS_ONE,
                   timeout=1)  # timeout of 1 sec so that the read and write operations are blocking,
    # after the timeout the program continues

    # clear buffers
    s.reset_input_buffer()
    s.reset_output_buffer()

    UploadFile.counter = 0  # init counter

    # Create the GUI window
    root = tk.Tk()
    root.title("Main Menu")

    # Option selection dropdown
    option_var = tk.StringVar(root)
    option_var.set("Select an option")  # Default value
    options = ["1.Objects Detector System", "2. Telemeter", "3. Light Sources Detector System",
               "4. Light Sources and Objects Detector System", "5. Load Script", "6. Run Script"]
    option_menu = tk.OptionMenu(root, option_var, *options)
    option_menu.pack()

    # "Select" button
    Select_button = tk.Button(root, text="Select",
                              command=lambda: on_option_selected(root, option_var, mask_distance_entry, s))
    Select_button.pack()

    mask_distance_label = tk.Label(root, text="Mask Distance (in cm):")
    mask_distance_label.pack()

    mask_distance_entry = tk.Entry(root)
    mask_distance_entry.insert(0, "50")
    mask_distance_entry.pack()

    root.mainloop()


if __name__ == '__main__':
    main()