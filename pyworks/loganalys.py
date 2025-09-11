import pandas as pd

# CSV読み込み
df = pd.read_csv("yourfile.csv", usecols=["time", "id"], dtype={"time": "int64", "id": "string"})

# idごとにソート
df = df.sort_values(["id", "time"])

# timeの差分を計算（n回目とn+1回目の差）
df["diff"] = df.groupby("id")["time"].diff()

# 各idごとの統計量を計算
stats = df.groupby("id")["diff"].agg(
    mean="mean",
    std="std",
    min="min",
    max="max",
    count="count"
).reset_index()

# 整形表でテキストファイルに書き出し
with open("output.txt", "w") as f:
    f.write(stats.to_string(index=False))

print("書き出し完了: output.txt")
