import os
import csv
import sys
import threading
import queue
import time
from datetime import datetime
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import serial
import serial.tools.list_ports

# Determine base path for the app (works for both script and PyInstaller exe)
if getattr(sys, 'frozen', False):
    BASE_DIR = os.path.dirname(sys.executable)
else:
    BASE_DIR = os.path.dirname(os.path.abspath(__file__))

DATA_DIR = os.path.join(BASE_DIR, 'data')
CARDS_FILE = os.path.join(DATA_DIR, 'cards.csv')
LOG_FILE = os.path.join(DATA_DIR, 'attendance_log.csv')

# Ensure data directory exists
if not os.path.exists(DATA_DIR):
    os.makedirs(DATA_DIR)

class CardDatabase:
    def __init__(self, filename):
        self.filename = filename
        self.cards = {} # uid -> dict of info
        self.header = ['uid', 'name', 'student_id', 'class_name', 'registered_at', 'note']
        self._ensure_file()
        self.load()

    def _ensure_file(self):
        if not os.path.exists(self.filename):
            with open(self.filename, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(self.header)

    def load(self):
        self.cards = {}
        with open(self.filename, 'r', newline='', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                if 'uid' in row and row['uid']:
                    self.cards[row['uid']] = row

    def save_all(self):
        with open(self.filename, 'w', newline='', encoding='utf-8') as f:
            writer = csv.DictWriter(f, fieldnames=self.header)
            writer.writeheader()
            for uid, info in self.cards.items():
                writer.writerow(info)

    def add_card(self, uid, name, student_id, class_name, note=""):
        self.cards[uid] = {
            'uid': uid,
            'name': name,
            'student_id': student_id,
            'class_name': class_name,
            'registered_at': datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            'note': note
        }
        self.save_all()
        return self.cards[uid]

    def find_card(self, uid):
        return self.cards.get(uid, None)

    def get_all(self):
        return list(self.cards.values())

class AttendanceLog:
    def __init__(self, filename):
        self.filename = filename
        self.header = ['uid', 'name', 'student_id', 'class_name', 'time', 'status']
        self._ensure_file()

    def _ensure_file(self):
        if not os.path.exists(self.filename):
            with open(self.filename, 'w', newline='', encoding='utf-8') as f:
                writer = csv.writer(f)
                writer.writerow(self.header)

    def add_log(self, uid, status, card_info=None):
        log_entry = {
            'uid': uid,
            'name': card_info['name'] if card_info else '',
            'student_id': card_info['student_id'] if card_info else '',
            'class_name': card_info['class_name'] if card_info else '',
            'time': datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            'status': status
        }
        with open(self.filename, 'a', newline='', encoding='utf-8') as f:
            writer = csv.DictWriter(f, fieldnames=self.header)
            writer.writerow(log_entry)
        return log_entry

    def get_all(self):
        logs = []
        with open(self.filename, 'r', newline='', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                logs.append(row)
        return logs

class SerialManager:
    def __init__(self, data_callback, log_callback):
        self.serial_port = serial.Serial()
        self.thread = None
        self.running = False
        self.data_callback = data_callback
        self.log_callback = log_callback

    def get_ports(self):
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def connect(self, port, baudrate):
        try:
            self.serial_port.port = port
            self.serial_port.baudrate = baudrate
            self.serial_port.timeout = 1
            self.serial_port.open()
            self.running = True
            self.thread = threading.Thread(target=self._read_loop, daemon=True)
            self.thread.start()
            return True, "Connected successfully."
        except Exception as e:
            return False, str(e)

    def disconnect(self):
        self.running = False
        if self.serial_port.is_open:
            self.serial_port.close()

    def send(self, data):
        if self.serial_port.is_open:
            try:
                self.serial_port.write(data.encode('utf-8'))
                self.log_callback(f"SENT: {data.strip()}")
            except Exception as e:
                self.log_callback(f"SEND ERROR: {e}")

    def _read_loop(self):
        buffer = ""
        while self.running:
            try:
                if self.serial_port.in_waiting > 0:
                    data = self.serial_port.read(self.serial_port.in_waiting).decode('utf-8', errors='ignore')
                    buffer += data
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()
                        if line:
                            self.log_callback(f"RECV: {line}")
                            self.data_callback(line)
            except Exception as e:
                self.log_callback(f"READ ERROR: {e}")
                self.disconnect()
                break
            time.sleep(0.01)

class SmartAttendanceApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Smart Attendance System")
        self.geometry("1000x700")
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

        self.db = CardDatabase(CARDS_FILE)
        self.log_db = AttendanceLog(LOG_FILE)
        
        # UI Queue for thread-safe UI updates
        self.ui_queue = queue.Queue()
        self.serial_mgr = SerialManager(self.on_serial_data, self.on_serial_log)

        self.current_scanned_uid = None

        self._setup_ui()
        self._load_data()
        self._check_queue()

    def _setup_ui(self):
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=2)
        self.grid_rowconfigure(1, weight=1)

        # 1. Connection Panel (Top)
        conn_frame = ttk.LabelFrame(self, text="Connection Setup")
        conn_frame.grid(row=0, column=0, columnspan=2, padx=10, pady=5, sticky="ew")

        ttk.Label(conn_frame, text="COM Port:").grid(row=0, column=0, padx=5, pady=5)
        self.cb_port = ttk.Combobox(conn_frame, values=self.serial_mgr.get_ports(), width=15)
        self.cb_port.grid(row=0, column=1, padx=5, pady=5)
        if self.cb_port['values']:
            self.cb_port.current(0)

        ttk.Button(conn_frame, text="Refresh", command=self.refresh_ports).grid(row=0, column=2, padx=5, pady=5)

        ttk.Label(conn_frame, text="Baudrate:").grid(row=0, column=3, padx=5, pady=5)
        self.cb_baud = ttk.Combobox(conn_frame, values=["9600", "19200", "38400", "57600", "115200"], width=10)
        self.cb_baud.grid(row=0, column=4, padx=5, pady=5)
        self.cb_baud.set("9600")

        self.btn_connect = ttk.Button(conn_frame, text="Connect", command=self.connect_serial)
        self.btn_connect.grid(row=0, column=5, padx=5, pady=5)

        self.btn_disconnect = ttk.Button(conn_frame, text="Disconnect", command=self.disconnect_serial, state=tk.DISABLED)
        self.btn_disconnect.grid(row=0, column=6, padx=5, pady=5)

        self.lbl_status = ttk.Label(conn_frame, text="Disconnected", foreground="red", font=("Arial", 10, "bold"))
        self.lbl_status.grid(row=0, column=7, padx=10, pady=5)

        # Left Column: Scan Info & Registration
        left_frame = tk.Frame(self)
        left_frame.grid(row=1, column=0, sticky="nsew", padx=10, pady=5)
        left_frame.grid_columnconfigure(0, weight=1)

        # 3. Current Card Panel
        self.scan_frame = tk.LabelFrame(left_frame, text="Current Scan", font=("Arial", 12, "bold"), bg="#f0f0f0")
        self.scan_frame.grid(row=0, column=0, sticky="nsew", pady=(0, 10))
        self.scan_frame.grid_columnconfigure(1, weight=1)

        self.lbl_scan_uid = tk.Label(self.scan_frame, text="---", font=("Arial", 24, "bold"), bg="#f0f0f0")
        self.lbl_scan_uid.grid(row=0, column=0, columnspan=2, pady=10)

        self.lbl_scan_status = tk.Label(self.scan_frame, text="Waiting for card...", font=("Arial", 14), bg="#f0f0f0")
        self.lbl_scan_status.grid(row=1, column=0, columnspan=2, pady=5)

        tk.Label(self.scan_frame, text="Name:", bg="#f0f0f0").grid(row=2, column=0, sticky="e", padx=5, pady=2)
        self.lbl_scan_name = tk.Label(self.scan_frame, text="---", font=("Arial", 12, "bold"), bg="#f0f0f0")
        self.lbl_scan_name.grid(row=2, column=1, sticky="w", padx=5, pady=2)

        tk.Label(self.scan_frame, text="ID/MSSV:", bg="#f0f0f0").grid(row=3, column=0, sticky="e", padx=5, pady=2)
        self.lbl_scan_id = tk.Label(self.scan_frame, text="---", bg="#f0f0f0")
        self.lbl_scan_id.grid(row=3, column=1, sticky="w", padx=5, pady=2)

        tk.Label(self.scan_frame, text="Class:", bg="#f0f0f0").grid(row=4, column=0, sticky="e", padx=5, pady=2)
        self.lbl_scan_class = tk.Label(self.scan_frame, text="---", bg="#f0f0f0")
        self.lbl_scan_class.grid(row=4, column=1, sticky="w", padx=5, pady=2)
        
        tk.Label(self.scan_frame, text="Time:", bg="#f0f0f0").grid(row=5, column=0, sticky="e", padx=5, pady=(2,10))
        self.lbl_scan_time = tk.Label(self.scan_frame, text="---", bg="#f0f0f0")
        self.lbl_scan_time.grid(row=5, column=1, sticky="w", padx=5, pady=(2,10))

        # 4. Registration Panel
        self.reg_frame = ttk.LabelFrame(left_frame, text="Register New Card")
        self.reg_frame.grid(row=1, column=0, sticky="nsew", pady=(0, 10))
        self.reg_frame.grid_columnconfigure(1, weight=1)

        ttk.Label(self.reg_frame, text="UID:").grid(row=0, column=0, sticky="e", padx=5, pady=5)
        self.entry_reg_uid = ttk.Entry(self.reg_frame, state="readonly")
        self.entry_reg_uid.grid(row=0, column=1, sticky="ew", padx=5, pady=5)

        ttk.Label(self.reg_frame, text="Name:").grid(row=1, column=0, sticky="e", padx=5, pady=5)
        self.entry_reg_name = ttk.Entry(self.reg_frame)
        self.entry_reg_name.grid(row=1, column=1, sticky="ew", padx=5, pady=5)

        ttk.Label(self.reg_frame, text="ID/MSSV:").grid(row=2, column=0, sticky="e", padx=5, pady=5)
        self.entry_reg_id = ttk.Entry(self.reg_frame)
        self.entry_reg_id.grid(row=2, column=1, sticky="ew", padx=5, pady=5)

        ttk.Label(self.reg_frame, text="Class:").grid(row=3, column=0, sticky="e", padx=5, pady=5)
        self.entry_reg_class = ttk.Entry(self.reg_frame)
        self.entry_reg_class.grid(row=3, column=1, sticky="ew", padx=5, pady=5)

        self.btn_register = ttk.Button(self.reg_frame, text="Register This Card", command=self.register_card, state=tk.DISABLED)
        self.btn_register.grid(row=4, column=0, columnspan=2, pady=10)

        # Right Column: Data Tables
        right_frame = tk.Frame(self)
        right_frame.grid(row=1, column=1, sticky="nsew", padx=(0, 10), pady=5)
        right_frame.grid_rowconfigure(0, weight=1)
        right_frame.grid_columnconfigure(0, weight=1)

        # 5 & 6. Notebook for Tables
        self.notebook = ttk.Notebook(right_frame)
        self.notebook.grid(row=0, column=0, sticky="nsew")

        # Attendance Log Tab
        log_tab = ttk.Frame(self.notebook)
        self.notebook.add(log_tab, text="Attendance Log")
        log_tab.grid_rowconfigure(0, weight=1)
        log_tab.grid_columnconfigure(0, weight=1)
        
        cols_log = ('time', 'uid', 'name', 'student_id', 'class_name', 'status')
        self.tree_log = ttk.Treeview(log_tab, columns=cols_log, show='headings')
        for col in cols_log:
            self.tree_log.heading(col, text=col.replace('_', ' ').title())
            self.tree_log.column(col, width=100)
        self.tree_log.column('time', width=150)
        self.tree_log.grid(row=0, column=0, sticky="nsew")
        
        sb_log = ttk.Scrollbar(log_tab, orient=tk.VERTICAL, command=self.tree_log.yview)
        self.tree_log.configure(yscroll=sb_log.set)
        sb_log.grid(row=0, column=1, sticky='ns')

        # Registered Cards Tab
        cards_tab = ttk.Frame(self.notebook)
        self.notebook.add(cards_tab, text="Registered Cards")
        cards_tab.grid_rowconfigure(0, weight=1)
        cards_tab.grid_columnconfigure(0, weight=1)

        cols_cards = ('uid', 'name', 'student_id', 'class_name', 'registered_at')
        self.tree_cards = ttk.Treeview(cards_tab, columns=cols_cards, show='headings')
        for col in cols_cards:
            self.tree_cards.heading(col, text=col.replace('_', ' ').title())
            self.tree_cards.column(col, width=100)
        self.tree_cards.column('registered_at', width=150)
        self.tree_cards.grid(row=0, column=0, sticky="nsew")

        sb_cards = ttk.Scrollbar(cards_tab, orient=tk.VERTICAL, command=self.tree_cards.yview)
        self.tree_cards.configure(yscroll=sb_cards.set)
        sb_cards.grid(row=0, column=1, sticky='ns')

        # 2. UART Log Panel (Bottom)
        log_frame = ttk.LabelFrame(self, text="UART Log")
        log_frame.grid(row=2, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        
        self.txt_log = scrolledtext.ScrolledText(log_frame, height=8, state='disabled', font=("Consolas", 9))
        self.txt_log.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        ttk.Button(log_frame, text="Clear Log", command=self.clear_log).pack(anchor="e", padx=5, pady=(0,5))

    def refresh_ports(self):
        ports = self.serial_mgr.get_ports()
        self.cb_port['values'] = ports
        if ports:
            self.cb_port.current(0)
        else:
            self.cb_port.set("")

    def connect_serial(self):
        port = self.cb_port.get()
        baudrate = self.cb_baud.get()
        if not port or not baudrate:
            messagebox.showerror("Error", "Please select a COM port and Baudrate.")
            return

        success, msg = self.serial_mgr.connect(port, int(baudrate))
        if success:
            self.lbl_status.config(text="Connected", foreground="green")
            self.btn_connect.config(state=tk.DISABLED)
            self.btn_disconnect.config(state=tk.NORMAL)
            self.cb_port.config(state=tk.DISABLED)
            self.cb_baud.config(state=tk.DISABLED)
        else:
            messagebox.showerror("Connection Error", msg)

    def disconnect_serial(self):
        self.serial_mgr.disconnect()
        self.lbl_status.config(text="Disconnected", foreground="red")
        self.btn_connect.config(state=tk.NORMAL)
        self.btn_disconnect.config(state=tk.DISABLED)
        self.cb_port.config(state="readonly")
        self.cb_baud.config(state="readonly")

    def _load_data(self):
        # Load registered cards into Treeview
        for item in self.tree_cards.get_children():
            self.tree_cards.delete(item)
        for card in self.db.get_all():
            self.tree_cards.insert('', 'end', values=(card['uid'], card['name'], card['student_id'], card['class_name'], card['registered_at']))

        # Load logs into Treeview
        for item in self.tree_log.get_children():
            self.tree_log.delete(item)
        for log in reversed(self.log_db.get_all()): # Show latest first
            self.tree_log.insert('', 'end', values=(log['time'], log['uid'], log['name'], log['student_id'], log['class_name'], log['status']))

    def on_serial_log(self, text):
        # Push to UI thread
        ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        self.ui_queue.put(('log', f"[{ts}] {text}\n"))

    def on_serial_data(self, data):
        # Push to UI thread
        self.ui_queue.put(('data', data))

    def _check_queue(self):
        while not self.ui_queue.empty():
            msg_type, content = self.ui_queue.get()
            if msg_type == 'log':
                self.txt_log.config(state='normal')
                self.txt_log.insert(tk.END, content)
                self.txt_log.see(tk.END)
                self.txt_log.config(state='disabled')
            elif msg_type == 'data':
                self._process_serial_data(content)
        self.after(100, self._check_queue) # Check queue every 100ms

    def _process_serial_data(self, data):
        data = data.upper()
        if data.startswith("CARD:"):
            uid = data[5:].strip()
            self.current_scanned_uid = uid
            self.lbl_scan_uid.config(text=uid)
            current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            self.lbl_scan_time.config(text=current_time)

            card_info = self.db.find_card(uid)

            if card_info:
                # Card is Registered
                self.scan_frame.config(bg="#d4edda") # Light green
                for child in self.scan_frame.winfo_children():
                    child.config(bg="#d4edda")
                
                self.lbl_scan_status.config(text="Registered", fg="green", font=("Arial", 14, "bold"))
                self.lbl_scan_name.config(text=card_info['name'])
                self.lbl_scan_id.config(text=card_info['student_id'])
                self.lbl_scan_class.config(text=card_info['class_name'])

                # Log to file and table
                log_entry = self.log_db.add_log(uid, "Registered", card_info)
                self.tree_log.insert('', 0, values=(log_entry['time'], log_entry['uid'], log_entry['name'], log_entry['student_id'], log_entry['class_name'], log_entry['status']))

                # Send back to STM32
                self.serial_mgr.send(f"Known:{card_info['name']}\n")

                # Disable reg form
                self.entry_reg_uid.config(state=tk.NORMAL)
                self.entry_reg_uid.delete(0, tk.END)
                self.entry_reg_uid.config(state="readonly")
                self.btn_register.config(state=tk.DISABLED)

            else:
                # Card is Unknown
                self.scan_frame.config(bg="#fff3cd") # Light yellow
                for child in self.scan_frame.winfo_children():
                    child.config(bg="#fff3cd")

                self.lbl_scan_status.config(text="Unknown Card", fg="#856404", font=("Arial", 14, "bold"))
                self.lbl_scan_name.config(text="---")
                self.lbl_scan_id.config(text="---")
                self.lbl_scan_class.config(text="---")

                # Log to file and table
                log_entry = self.log_db.add_log(uid, "Unknown")
                self.tree_log.insert('', 0, values=(log_entry['time'], log_entry['uid'], '', '', '', log_entry['status']))

                # Send back to STM32
                self.serial_mgr.send("Unknown\n")

                # Enable reg form
                self.entry_reg_uid.config(state=tk.NORMAL)
                self.entry_reg_uid.delete(0, tk.END)
                self.entry_reg_uid.insert(0, uid)
                self.entry_reg_uid.config(state="readonly")
                self.entry_reg_name.delete(0, tk.END)
                self.entry_reg_id.delete(0, tk.END)
                self.entry_reg_class.delete(0, tk.END)
                self.btn_register.config(state=tk.NORMAL)
                self.entry_reg_name.focus()

    def register_card(self):
        uid = self.entry_reg_uid.get()
        name = self.entry_reg_name.get().strip()
        student_id = self.entry_reg_id.get().strip()
        class_name = self.entry_reg_class.get().strip()

        if not uid or not name:
            messagebox.showwarning("Incomplete Data", "Name is required to register a card.")
            return

        # Add to database
        card_info = self.db.add_card(uid, name, student_id, class_name)
        
        # Update cards Treeview
        self.tree_cards.insert('', 'end', values=(card_info['uid'], card_info['name'], card_info['student_id'], card_info['class_name'], card_info['registered_at']))

        # Update Current Scan UI to green
        self.scan_frame.config(bg="#d4edda")
        for child in self.scan_frame.winfo_children():
            child.config(bg="#d4edda")
        self.lbl_scan_status.config(text="Registered (New)", fg="green", font=("Arial", 14, "bold"))
        self.lbl_scan_name.config(text=name)
        self.lbl_scan_id.config(text=student_id)
        self.lbl_scan_class.config(text=class_name)

        # Send response to STM32
        self.serial_mgr.send(f"Added:{name}\n")

        # Disable registration button and clear fields
        self.btn_register.config(state=tk.DISABLED)
        self.entry_reg_name.delete(0, tk.END)
        self.entry_reg_id.delete(0, tk.END)
        self.entry_reg_class.delete(0, tk.END)
        
        messagebox.showinfo("Success", f"Card {uid} registered to {name} successfully.")

    def clear_log(self):
        self.txt_log.config(state='normal')
        self.txt_log.delete(1.0, tk.END)
        self.txt_log.config(state='disabled')

    def on_closing(self):
        self.serial_mgr.disconnect()
        self.destroy()

if __name__ == "__main__":
    app = SmartAttendanceApp()
    app.mainloop()
