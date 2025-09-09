import tkinter as tk
import threading
import time
import random
from num2bin import num2bin

# グローバル変数（Entryの値）
HKD = AOMR = AKT = IWT = YMGT = MYG = 0

# Entryウィジェット
entry_HKD: tk.Entry
entry_AOMR: tk.Entry
entry_AKT: tk.Entry
entry_IWT: tk.Entry
entry_YMGT: tk.Entry
entry_MYG: tk.Entry

# チェックボックス変数
check_HKD: tk.IntVar
check_AOMR: tk.IntVar
check_AKT: tk.IntVar
check_IWT: tk.IntVar
check_YMGT: tk.IntVar
check_MYG: tk.IntVar

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
    MYG = num2bin.sf16_bin16(float(entry_MYG.get()))

    # MYG: 入力値を符号付き16bitにスケーリングして16進数表示
    # try:
    #     myg_float = float(entry_MYG.get())
    #     if myg_float < -6.5536:     myg_float = -6.5536
    #     elif myg_float > 6.5534:    myg_float = 6.5534
    #     MYG = int(round(myg_float * 5000))
    # except ValueError:
    #     MYG = 0

def print_loop():
    global running
    while running:
        update_globals()
        # チェックされているラベルだけ出力
        output = []
        if check_HKD.get(): output.append(HKD)
        if check_AOMR.get(): output.append(AOMR)
        if check_AKT.get(): output.append(AKT)
        if check_IWT.get(): output.append(IWT)
        if check_YMGT.get(): output.append(YMGT)
        if check_MYG.get(): output.append(hex(MYG & 0xFFFF))  # 16進数表示
        print(" ".join(output))

        # 乱数ラベル3つ
        for lbl in random_labels:
            lbl['value_label'].config(text=str(random.randint(1,100)))

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

# Entryとチェックボックス作成
names = ['HKD', 'AOMR', 'AKT', 'IWT', 'YMGT', 'MYG']
for i, name in enumerate(names):
    lbl = tk.Label(root, text=name + ":")
    lbl.grid(row=i, column=0, padx=5, pady=5, sticky="e")

    entry = tk.Entry(root, width=10)
    entry.grid(row=i, column=1, padx=5, pady=5)
    globals()[f"entry_{name}"] = entry

    var = tk.IntVar(value=1)
    check = tk.Checkbutton(root, variable=var, fg="black", selectcolor="white", state="normal")
    check.grid(row=i, column=2, padx=5, pady=5)
    globals()[f"check_{name}"] = var

# 乱数表示用ラベル（3つ）
random_labels = []
for i, label_name in enumerate(['R1','R2','R3']):
    lbl_name = tk.Label(root, text=label_name + ":")
    lbl_name.grid(row=i, column=3, padx=5, pady=5, sticky="e")
    lbl_value = tk.Label(root, text="0", width=5, fg="blue")
    lbl_value.grid(row=i, column=4, padx=5, pady=5)
    random_labels.append({'name_label': lbl_name, 'value_label': lbl_value})

# 開始/停止ボタン
start_button = tk.Button(root, text="開始", command=toggle)
start_button.grid(row=6, column=0, columnspan=5, pady=10)

root.mainloop()



import tkinter as tk

root = tk.Tk()
root.title("10x2 Data Input")

# 上部にチェックボックスを2つ追加
check_var1 = tk.BooleanVar()
check_var2 = tk.BooleanVar()

check1 = tk.Checkbutton(root, text="Option 1", variable=check_var1)
check2 = tk.Checkbutton(root, text="Option 2", variable=check_var2)

check1.grid(row=0, column=0, sticky="w", pady=5)
check2.grid(row=0, column=1, sticky="w", pady=5)

# 任意の名前リスト
names = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j"]

# Entryを配置（1行目をチェックボックスに使ったので +1 する）
for i, name in enumerate(names, start=1):
    # 1列目
    label1 = tk.Label(root, text=f"DATA {name}1")
    label1.grid(row=i, column=0, padx=5, pady=2, sticky="e")

    globals()[f"data_{name}1"] = tk.StringVar()
    entry1 = tk.Entry(root, textvariable=globals()[f"data_{name}1"], width=10)
    entry1.grid(row=i, column=1, padx=5, pady=2)

    # 2列目
    label2 = tk.Label(root, text=f"DATA {name}2")
    label2.grid(row=i, column=2, padx=5, pady=2, sticky="e")

    globals()[f"data_{name}2"] = tk.StringVar()
    entry2 = tk.Entry(root, textvariable=globals()[f"data_{name}2"], width=10)
    entry2.grid(row=i, column=3, padx=5, pady=2)

# 確認ボタン
def show_values():
    print("Check1:", check_var1.get())
    print("Check2:", check_var2.get())
    for name in names:
        print(f"data_{name}1 =", globals()[f"data_{name}1"].get())
        print(f"data_{name}2 =", globals()[f"data_{name}2"].get())

button = tk.Button(root, text="Show Values", command=show_values)
button.grid(row=len(names) + 1, column=0, columnspan=4, pady=10)

root.mainloop()

