#!/usr/bin/env python3
import threading
import queue
import time
import sys
import os
import shutil
import subprocess
from datetime import datetime
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

try:
    from zoneinfo import ZoneInfo, available_timezones
except Exception:
    ZoneInfo = None
    available_timezones = None

try:
    import serial
    from serial.tools import list_ports
except Exception as e:
    print("pyserial is required:", e, file=sys.stderr)
    sys.exit(1)

APP_TITLE = "UART Log Viewer"
DEFAULT_BAUD = "115200"
BAUD_RATES = [
    "300",
    "600",
    "1200",
    "2400",
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    "230400",
    "460800",
    "921600",
]

def get_serial_ports():
    ports = []
    for p in list_ports.comports():
        dev = p.device
        base = os.path.basename(dev)
        if base.startswith("ttyUSB") or base.startswith("ttyACM"):
            ports.append(dev)
    return sorted(ports)

class PortTab:
    def __init__(self, parent, port, get_timestamp, timestamp_enabled):
        self.port = port
        self._get_timestamp = get_timestamp
        self._timestamp_enabled = timestamp_enabled
        self._line_buffer = ""
        self.frame = ttk.Frame(parent)
        self.text = tk.Text(self.frame, wrap="none", height=24)
        self.text.configure(state="disabled")
        self.text.grid(row=1, column=0, columnspan=4, sticky="nsew", padx=8, pady=8)

        self.scroll_y = ttk.Scrollbar(self.frame, orient="vertical", command=self.text.yview)
        self.scroll_x = ttk.Scrollbar(self.frame, orient="horizontal", command=self.text.xview)
        self.text.configure(yscrollcommand=self.scroll_y.set, xscrollcommand=self.scroll_x.set)
        self.scroll_y.grid(row=1, column=4, sticky="ns")
        self.scroll_x.grid(row=2, column=0, columnspan=4, sticky="ew")

        self.baud_var = tk.StringVar(value=DEFAULT_BAUD)
        self.status_var = tk.StringVar(value="Disconnected")

        ttk.Label(self.frame, text=f"Port: {port}").grid(row=0, column=0, sticky="w", padx=8, pady=(8, 0))
        ttk.Label(self.frame, text="Baud:").grid(row=0, column=1, sticky="e", padx=4, pady=(8, 0))
        self.baud_entry = ttk.Combobox(self.frame, textvariable=self.baud_var, values=BAUD_RATES, width=10, state="readonly")
        self.baud_entry.grid(row=0, column=2, sticky="w", padx=4, pady=(8, 0))
        self.connect_btn = ttk.Button(self.frame, text="Connect", command=self.toggle_connect)
        self.connect_btn.grid(row=0, column=3, sticky="e", padx=8, pady=(8, 0))

        self.send_var = tk.StringVar(value="")
        self.send_entry = ttk.Entry(self.frame, textvariable=self.send_var)
        self.send_entry.grid(row=3, column=0, columnspan=2, sticky="ew", padx=8, pady=(0, 8))
        self.send_entry.bind("<Return>", self._on_send_enter)

        self.send_btn = ttk.Button(self.frame, text="Enter", command=self.send_line)
        self.send_btn.grid(row=3, column=2, sticky="e", padx=4, pady=(0, 8))
        self.clear_btn = ttk.Button(self.frame, text="Clear", command=self.clear_send)
        self.clear_btn.grid(row=3, column=3, sticky="e", padx=8, pady=(0, 8))

        self.status_lbl = ttk.Label(self.frame, textvariable=self.status_var)
        self.status_lbl.grid(row=4, column=0, columnspan=4, sticky="w", padx=8, pady=(0, 8))

        self.frame.grid_rowconfigure(1, weight=1)
        self.frame.grid_columnconfigure(0, weight=1)
        self.frame.grid_columnconfigure(1, weight=1)

        self._ser = None
        self._thread = None
        self._q = queue.Queue()
        self._stop = threading.Event()

    def append_text(self, s):
        self.text.configure(state="normal")
        self.text.insert("end", s)
        self.text.see("end")
        self.text.configure(state="disabled")

    def toggle_connect(self):
        if self._ser:
            self.disconnect()
        else:
            self.connect()

    def connect(self):
        baud = self.baud_var.get().strip()
        if not baud.isdigit():
            messagebox.showerror(APP_TITLE, f"Invalid baud rate: {baud}")
            return
        try:
            self._ser = serial.Serial(self.port, int(baud), timeout=0.1)
        except Exception as e:
            messagebox.showerror(APP_TITLE, f"Failed to open {self.port}: {e}")
            self._ser = None
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._reader, daemon=True)
        self._thread.start()
        self.connect_btn.configure(text="Disconnect")
        self.status_var.set(f"Connected @ {baud}")

    def disconnect(self):
        self._stop.set()
        if self._thread:
            self._thread.join(timeout=1)
        if self._ser:
            try:
                self._ser.close()
            except Exception:
                pass
        self._ser = None
        self._thread = None
        self.connect_btn.configure(text="Connect")
        self.status_var.set("Disconnected")

    def _on_send_enter(self, _event=None):
        self.send_line()

    def clear_send(self):
        self.send_var.set("")

    def send_line(self):
        data = self.send_var.get()
        if data is None:
            return
        if not self._ser:
            messagebox.showerror(APP_TITLE, "Port not connected.")
            return
        try:
            payload = (data + "\r\n").encode()
            self._ser.write(payload)
        except Exception as e:
            messagebox.showerror(APP_TITLE, f"Send failed: {e}")

    def _reader(self):
        while not self._stop.is_set():
            try:
                data = self._ser.read(4096)
            except Exception:
                break
            if data:
                try:
                    text = data.decode(errors="replace")
                except Exception:
                    text = repr(data)
                # Normalize common UART line endings and strip NULs that show as squares
                text = text.replace("\x00", "")
                text = text.replace("\r\n", "\n").replace("\r", "\n")
                self._q.put(text)
            else:
                time.sleep(0.02)

    def drain_queue(self):
        out = []
        try:
            while True:
                out.append(self._q.get_nowait())
        except queue.Empty:
            pass
        if out:
            self.append_text(self._format_text("".join(out)))

    def _format_text(self, text):
        if not self._timestamp_enabled.get():
            return text

        text = self._line_buffer + text
        lines = text.split("\n")
        self._line_buffer = lines.pop()  # last partial line

        stamped = []
        for line in lines:
            if line == "":
                stamped.append("\n")
                continue
            stamped.append(f"{self._get_timestamp()} {line}\n")
        return "".join(stamped)

    def get_log_text(self):
        return self.text.get("1.0", "end-1c")

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(APP_TITLE)
        self.geometry("980x620")

        self.tabs = ttk.Notebook(self)
        self.tabs.pack(fill="both", expand=True)

        self.port_tabs = []
        self.timestamp_enabled = tk.BooleanVar(value=False)
        self.tz_name = self._get_system_tz()
        self.theme_var = tk.StringVar(value="Dark")
        self._build_menu()
        self._apply_theme(self.theme_var.get())
        self._populate_tabs()

        self.after(50, self._tick)
        self.protocol("WM_DELETE_WINDOW", self._on_close)
        self.bind_all("<Control-f>", self._open_find_shortcut)
        self._find_dialog = None
        self._find_vars = None

    def _build_menu(self):
        menubar = tk.Menu(self)
        file_menu = tk.Menu(menubar, tearoff=0)
        file_menu.add_command(label="New Tab...", command=self.new_tab_dialog)
        file_menu.add_command(label="Refresh Ports", command=self.refresh_ports)
        file_menu.add_command(label="Save Logs...", command=self.save_logs)
        file_menu.add_separator()
        file_menu.add_command(label="Quit", command=self._on_close)
        menubar.add_cascade(label="File", menu=file_menu)

        tools_menu = tk.Menu(menubar, tearoff=0)
        tools_menu.add_checkbutton(label="Timestamp Each Line", variable=self.timestamp_enabled)
        tools_menu.add_command(label="Timezone...", command=self.select_timezone)
        tools_menu.add_command(label="Find...", command=self.open_find_dialog, accelerator="Ctrl+F")
        tools_menu.add_separator()
        tools_menu.add_radiobutton(label="Theme: Dark", variable=self.theme_var, value="Dark", command=self._on_theme_change)
        tools_menu.add_radiobutton(label="Theme: Light", variable=self.theme_var, value="Light", command=self._on_theme_change)
        menubar.add_cascade(label="Tools", menu=tools_menu)
        self.config(menu=menubar)

    def _populate_tabs(self):
        ports = get_serial_ports()
        if not ports:
            ttk.Label(self, text="No serial ports detected.").pack(pady=20)
            return
        for port in ports:
            self._add_tab(port)

    def _add_tab(self, port):
        tab = PortTab(self.tabs, port, self._get_timestamp, self.timestamp_enabled)
        self.port_tabs.append(tab)
        self.tabs.add(tab.frame, text=os.path.basename(port))

    def refresh_ports(self):
        # Do not auto-add; just refresh list for New Tab dialog
        self._available_ports = get_serial_ports()

    def new_tab_dialog(self):
        self.refresh_ports()
        existing = {t.port for t in self.port_tabs}
        ports = [p for p in self._available_ports if p not in existing]
        if not ports:
            messagebox.showinfo(APP_TITLE, "No new /dev/ttyUSB* or /dev/ttyACM* ports detected.")
            return

        dialog = tk.Toplevel(self)
        dialog.title("Open New UART")
        dialog.transient(self)
        dialog.grab_set()

        ttk.Label(dialog, text="Select port:").grid(row=0, column=0, padx=10, pady=10, sticky="w")
        port_var = tk.StringVar(value=ports[0])
        combo = ttk.Combobox(dialog, textvariable=port_var, values=ports, state="readonly", width=30)
        combo.grid(row=0, column=1, padx=10, pady=10, sticky="ew")

        btn_frame = ttk.Frame(dialog)
        btn_frame.grid(row=1, column=0, columnspan=2, pady=(0, 10))
        def on_ok():
            port = port_var.get()
            if port:
                self._add_tab(port)
            dialog.destroy()
        def on_cancel():
            dialog.destroy()
        ttk.Button(btn_frame, text="Open", command=on_ok).grid(row=0, column=0, padx=6)
        ttk.Button(btn_frame, text="Cancel", command=on_cancel).grid(row=0, column=1, padx=6)

        dialog.columnconfigure(1, weight=1)
        combo.focus_set()

    def _get_system_tz(self):
        tz = datetime.now().astimezone().tzinfo
        if ZoneInfo and isinstance(tz, ZoneInfo):
            return tz.key
        try:
            return tz.tzname(None)
        except Exception:
            return "UTC"

    def _get_timestamp(self):
        if ZoneInfo and self.tz_name:
            try:
                tz = ZoneInfo(self.tz_name)
            except Exception:
                tz = datetime.now().astimezone().tzinfo
        else:
            tz = datetime.now().astimezone().tzinfo
        now = datetime.now(tz)
        ms = f"{now.microsecond // 1000:03d}"
        return now.strftime(f"[%d-%m-%Y %H:%M:%S:{ms}]")

    def select_timezone(self):
        if not available_timezones:
            messagebox.showerror(APP_TITLE, "Timezone database not available.")
            return
        zones = sorted(available_timezones())

        dialog = tk.Toplevel(self)
        dialog.title("Select Timezone")
        dialog.transient(self)
        dialog.grab_set()

        ttk.Label(dialog, text="Timezone:").grid(row=0, column=0, padx=10, pady=10, sticky="w")
        tz_var = tk.StringVar(value=self.tz_name or "UTC")
        combo = ttk.Combobox(dialog, textvariable=tz_var, values=zones, state="readonly", width=40)
        combo.grid(row=0, column=1, padx=10, pady=10, sticky="ew")

        btn_frame = ttk.Frame(dialog)
        btn_frame.grid(row=1, column=0, columnspan=2, pady=(0, 10))
        def on_ok():
            self.tz_name = tz_var.get()
            dialog.destroy()
        def on_cancel():
            dialog.destroy()
        ttk.Button(btn_frame, text="Set", command=on_ok).grid(row=0, column=0, padx=6)
        ttk.Button(btn_frame, text="Cancel", command=on_cancel).grid(row=0, column=1, padx=6)

        dialog.columnconfigure(1, weight=1)
        combo.focus_set()

    def _tick(self):
        for tab in self.port_tabs:
            tab.drain_queue()
        self.after(50, self._tick)

    def save_logs(self):
        tab_idx = self.tabs.index(self.tabs.select()) if self.port_tabs else None
        if tab_idx is None:
            return
        tab = self.port_tabs[tab_idx]
        default = f"uart_log_{tab.port.replace('/', '_')}.txt"
        path = self._save_path_dialog(default)
        if not path:
            return
        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(tab.get_log_text())
        except Exception as e:
            messagebox.showerror(APP_TITLE, f"Failed to save: {e}")

    def _on_close(self):
        for tab in self.port_tabs:
            tab.disconnect()
        self.destroy()

    def _on_theme_change(self):
        self._apply_theme(self.theme_var.get())

    def _apply_theme(self, theme_name):
        style = ttk.Style(self)
        theme = theme_name.lower()
        if theme == "dark":
            bg = "#1A1410"        # Claude Code Dark background
            fg = "#F5E6D3"        # primary text
            entry_bg = "#221C16"  # surface/panels
            text_bg = "#1A1410"
            accent = "#E67D22"    # Claude orange
            border = "#3D2E22"
            secondary = "#C4A584"
            self.configure(bg=bg)
            style.theme_use("default")
            style.configure(".", background=bg, foreground=fg)
            style.configure("TFrame", background=bg)
            style.configure("TLabel", background=bg, foreground=fg)
            style.configure("TButton", background=entry_bg, foreground=fg, bordercolor=border, focusthickness=1)
            style.map("TButton", background=[("active", "#2a241e")])
            style.configure("TNotebook", background=bg, borderwidth=0)
            style.configure("TNotebook.Tab", background=entry_bg, foreground=secondary, padding=[8, 4])
            style.map("TNotebook.Tab", background=[("selected", bg)], foreground=[("selected", fg)])
            style.configure("TEntry", fieldbackground=entry_bg, foreground=fg, insertcolor=fg)
            style.configure("TCombobox", fieldbackground=entry_bg, foreground=fg)
            style.map("TCombobox", fieldbackground=[("readonly", entry_bg)], foreground=[("readonly", fg)])
            self.option_add("*Text.background", text_bg)
            self.option_add("*Text.foreground", fg)
            self.option_add("*Text.insertBackground", fg)
            self.option_add("*Text.selectBackground", accent)
            self.option_add("*Text.selectForeground", "#ffffff")
        else:
            bg = "#FAF9F5"        # Claude Code Light background
            fg = "#1F1E1D"        # primary text
            entry_bg = "#F4F3EE"  # surface/panels
            text_bg = "#FAF9F5"
            accent = "#C96442"    # terracotta
            border = "#E5E3DC"
            secondary = "#6F6F78"
            self.configure(bg=bg)
            style.theme_use("default")
            style.configure(".", background=bg, foreground=fg)
            style.configure("TFrame", background=bg)
            style.configure("TLabel", background=bg, foreground=fg)
            style.configure("TButton", background=entry_bg, foreground=fg, bordercolor=border, focusthickness=1)
            style.map("TButton", background=[("active", "#ebe7de")])
            style.configure("TNotebook", background=bg, borderwidth=0)
            style.configure("TNotebook.Tab", background=entry_bg, foreground=secondary, padding=[8, 4])
            style.map("TNotebook.Tab", background=[("selected", bg)], foreground=[("selected", fg)])
            style.configure("TEntry", fieldbackground=entry_bg, foreground=fg, insertcolor=fg)
            style.configure("TCombobox", fieldbackground=entry_bg, foreground=fg)
            style.map("TCombobox", fieldbackground=[("readonly", entry_bg)], foreground=[("readonly", fg)])
            self.option_add("*Text.background", text_bg)
            self.option_add("*Text.foreground", fg)
            self.option_add("*Text.insertBackground", fg)
            self.option_add("*Text.selectBackground", accent)
            self.option_add("*Text.selectForeground", "#ffffff")

    def _save_path_dialog(self, default_name):
        if shutil.which("zenity"):
            default_path = os.path.join(os.path.expanduser("~"), default_name)
            proc = subprocess.run(
                [
                    "zenity",
                    "--file-selection",
                    "--save",
                    "--confirm-overwrite",
                    "--title=Save Logs",
                    f"--filename={default_path}",
                ],
                text=True,
                capture_output=True,
            )
            if proc.returncode != 0:
                return ""
            return proc.stdout.strip()

        return filedialog.asksaveasfilename(
            title="Save Logs",
            defaultextension=".txt",
            initialfile=default_name,
            filetypes=[("Text Files", "*.txt"), ("All Files", "*")],
        )

    def _open_find_shortcut(self, _event=None):
        self.open_find_dialog()

    def _current_tab(self):
        if not self.port_tabs:
            return None
        idx = self.tabs.index(self.tabs.select())
        return self.port_tabs[idx]

    def open_find_dialog(self):
        tab = self._current_tab()
        if not tab:
            return

        if self._find_dialog and self._find_dialog.winfo_exists():
            self._find_dialog.lift()
            return

        dialog = tk.Toplevel(self)
        dialog.title("Find")
        dialog.transient(self)
        dialog.grab_set()
        self._find_dialog = dialog

        if not self._find_vars:
            self._find_vars = {
                "query": tk.StringVar(value=""),
                "match_case": tk.BooleanVar(value=False),
                "direction": tk.StringVar(value="down"),
            }

        ttk.Label(dialog, text="Find:").grid(row=0, column=0, padx=10, pady=10, sticky="w")
        entry = ttk.Entry(dialog, textvariable=self._find_vars["query"], width=36)
        entry.grid(row=0, column=1, padx=10, pady=10, sticky="ew")

        opts = ttk.Frame(dialog)
        opts.grid(row=1, column=0, columnspan=2, padx=10, sticky="w")
        ttk.Checkbutton(opts, text="Match case", variable=self._find_vars["match_case"]).grid(row=0, column=0, sticky="w")
        ttk.Label(opts, text="Direction:").grid(row=0, column=1, padx=(12, 4), sticky="w")
        ttk.Radiobutton(opts, text="Down", variable=self._find_vars["direction"], value="down").grid(row=0, column=2, sticky="w")
        ttk.Radiobutton(opts, text="Up", variable=self._find_vars["direction"], value="up").grid(row=0, column=3, sticky="w")

        btns = ttk.Frame(dialog)
        btns.grid(row=2, column=0, columnspan=2, pady=(8, 10))
        ttk.Button(btns, text="Find Next", command=self.find_next).grid(row=0, column=0, padx=6)
        ttk.Button(btns, text="Close", command=dialog.destroy).grid(row=0, column=1, padx=6)

        dialog.columnconfigure(1, weight=1)
        entry.focus_set()
        entry.select_range(0, "end")

    def find_next(self):
        tab = self._current_tab()
        if not tab:
            return
        textw = tab.text
        query = self._find_vars["query"].get()
        if not query:
            return

        match_case = self._find_vars["match_case"].get()
        direction = self._find_vars["direction"].get()

        start = textw.index("insert")
        if direction == "down":
            idx = textw.search(query, start, stopindex="end", nocase=not match_case)
            if not idx:
                idx = textw.search(query, "1.0", stopindex="end", nocase=not match_case)
        else:
            idx = textw.search(query, start, stopindex="1.0", backwards=True, nocase=not match_case)
            if not idx:
                idx = textw.search(query, "end", stopindex="1.0", backwards=True, nocase=not match_case)

        if not idx:
            messagebox.showinfo(APP_TITLE, f"'{query}' not found.")
            return

        end = f"{idx}+{len(query)}c"
        textw.tag_remove("find_match", "1.0", "end")
        textw.tag_add("find_match", idx, end)
        textw.tag_config("find_match", background="#ffd966")
        textw.mark_set("insert", end)
        textw.see(idx)

if __name__ == "__main__":
    app = App()
    app.mainloop()
