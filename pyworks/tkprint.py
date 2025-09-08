import tkinter as tk
import threading
import time
import random

# グローバル変数（Entryの値）
HKD = AOMR = AKT = IWT = YMGT = MYG = ""

# Entryウィジェットの変数
entry_HKD: tk.Entry
entry_AOMR: tk.Entry
entry_AKT: tk.Entry
entry_IWT: tk.Entry
entry_YMGT: tk.Entry
entry_MYG: tk.Entry

running = False
thread = None

def update_globals():
    """Entryの内容をグローバル変数に反映"""
    global HKD, AOMR, AKT, IWT, YMGT, MYG
    HKD = entry_HKD.get()
    AOMR = entry_AOMR.get()
    AKT = entry_AKT.get()
    IWT = entry_IWT.get()
    YMGT = entry_YMGT.get()
    MYG = entry_MYG.get()

def print_loop():
    global running
    while running:
        update_globals()
        # Entryの値をコンソール出力
        print(HKD, AOMR, AKT, IWT, YMGT, MYG)
        # 乱数を3つ生成してラベルに表示
        for lbl in random_labels:
            value = random.randint(1, 100)
            lbl['value_label'].config(text=str(value))
        time.sleep(1)

def toggle():
    global running, thread
    if not running:
        running = True
        thread = threading.Thread(target=print_loop, daemon=True)
        thread.start()
        start_button.config(text="停止")
    else:
        running = False
        start_button.config(text="開始")

# メインウィンドウ
root = tk.Tk()
root.title("数字入力ウィンドウ")

# Entry作成（6個）
entries_info = [
    ("HKD", 0),
    ("AOMR", 1),
    ("AKT", 2),
    ("IWT", 3),
    ("YMGT", 4),
    ("MYG", 5)
]

for name, row in entries_info:
    lbl = tk.Label(root, text=name + ":")
    lbl.grid(row=row, column=0, padx=5, pady=5, sticky="e")

    entry = tk.Entry(root, width=10)
    entry.grid(row=row, column=1, padx=5, pady=5)

    # 変数に直接代入
    if name == "HKD":
        entry_HKD = entry
    elif name == "AOMR":
        entry_AOMR = entry
    elif name == "AKT":
        entry_AKT = entry
    elif name == "IWT":
        entry_IWT = entry
    elif name == "YMGT":
        entry_YMGT = entry
    elif name == "MYG":
        entry_MYG = entry

# 乱数表示用ラベル（3つ）
random_labels = []
for i, label_name in enumerate(['R1', 'R2', 'R3']):
    lbl_name = tk.Label(root, text=label_name + ":")
    lbl_name.grid(row=i, column=2, padx=5, pady=5, sticky="e")

    lbl_value = tk.Label(root, text="0", width=5, fg="blue")
    lbl_value.grid(row=i, column=3, padx=5, pady=5)

    random_labels.append({'name_label': lbl_name, 'value_label': lbl_value})

# 開始/停止ボタン
start_button = tk.Button(root, text="開始", command=toggle)
start_button.grid(row=6, column=0, columnspan=4, pady=10)

root.mainloop()
