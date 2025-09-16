import tkinter as tk
from tkinter import filedialog, messagebox
import threading
import time
import csv
from canlib import canlib

class CanLoggerApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Kvaser CAN Logger")

        # チャネル選択
        tk.Label(root, text="CAN Channel:").grid(row=0, column=0, sticky="w")
        self.channel_var = tk.StringVar(value="0")
        self.channel_menu = tk.OptionMenu(root, self.channel_var, *self.get_channels())
        self.channel_menu.grid(row=0, column=1, sticky="ew")

        # 保存先
        tk.Label(root, text="Log file:").grid(row=1, column=0, sticky="w")
        self.log_path_var = tk.StringVar()
        tk.Entry(root, textvariable=self.log_path_var, width=40).grid(row=1, column=1, sticky="ew")
        tk.Button(root, text="Browse", command=self.browse_file).grid(row=1, column=2)

        # 開始・終了ボタン
        self.start_button = tk.Button(root, text="Start Logging", command=self.start_logging)
        self.start_button.grid(row=2, column=0, pady=10)
        self.stop_button = tk.Button(root, text="Stop Logging", command=self.stop_logging, state="disabled")
        self.stop_button.grid(row=2, column=1, pady=10)

        # ステータス表示
        self.status_var = tk.StringVar(value="Idle")
        tk.Label(root, textvariable=self.status_var).grid(row=3, column=0, columnspan=3)

        self.running = False
        self.thread = None
        self.ch = None

    def get_channels(self):
        """利用可能なKvaserチャネル一覧を取得"""
        channels = []
        num_channels = canlib.getNumberOfChannels()
        for i in range(num_channels):
            chdata = canlib.ChannelData(i)
            channels.append(f"{i}: {chdata.channel_name}")
        return channels

    def browse_file(self):
        """ログ保存先を選択"""
        file_path = filedialog.asksaveasfilename(
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")]
        )
        if file_path:
            self.log_path_var.set(file_path)

    def start_logging(self):
        if not self.log_path_var.get():
            messagebox.showerror("Error", "Please select a log file path")
            return
        self.running = True
        self.start_button.config(state="disabled")
        self.stop_button.config(state="normal")
        self.status_var.set("Logging...")
        self.thread = threading.Thread(target=self.log_worker, daemon=True)
        self.thread.start()

    def stop_logging(self):
        self.running = False
        self.start_button.config(state="normal")
        self.stop_button.config(state="disabled")
        self.status_var.set("Stopped")

    def log_worker(self):
        channel_index = int(self.channel_var.get().split(":")[0])
        logfile = self.log_path_var.get()

        try:
            # CANチャネルオープン (サイレントモード)
            self.ch = canlib.openChannel(
                channel_index,
                canlib.canOPEN_ACCEPT_VIRTUAL | canlib.canOPEN_REQUIRE_INIT_ACCESS
            )
            self.ch.setBusOutputControl(canlib.canDRIVER_SILENT)
            self.ch.setBusParams(canlib.canBITRATE_500K)  # 適宜調整
            self.ch.busOn()

            with open(logfile, "w", newline="", buffering=1) as f:
                writer = csv.writer(f)
                # ヘッダー
                writer.writerow(["Timestamp", "ID", "DLC", "Data"])

                while self.running:
                    try:
                        frame = self.ch.read(timeout=500)
                        timestamp = time.time()
                        can_id = f"0x{frame.id:X}"
                        dlc = frame.dlc
                        data = " ".join(f"{b:02X}" for b in frame.data)

                        writer.writerow([f"{timestamp:.6f}", can_id, dlc, data])

                    except canlib.CanNoMsg:
                        continue
                    except canlib.CanError as e:
                        print("CAN error:", e)
                        break

        finally:
            if self.ch:
                self.ch.busOff()
                self.ch.close()

        self.status_var.set("Finished")

if __name__ == "__main__":
    root = tk.Tk()
    app = CanLoggerApp(root)
    root.mainloop()
