import csv
import random
from datetime import datetime, timedelta

# Parameters
filename = "BTCUSDT_big.csv"
num_rows = 500000  # Half a million rows
start_time = datetime(2020, 1, 1, 0, 0)

with open(filename, mode="w", newline="") as file:
    writer = csv.writer(file)

    writer.writerow(["timestamp", "open", "high", "low", "close", "volume"])

    current_time = start_time
    price = 10000

    for _ in range(num_rows):
        open_p = price + random.uniform(-50, 50)
        close_p = open_p + random.uniform(-50, 50)
        high_p = max(open_p, close_p) + random.uniform(0, 10)
        low_p = min(open_p, close_p) - random.uniform(0, 10)
        volume = random.uniform(0.1, 100)

        writer.writerow([
            int(current_time.timestamp()),
            round(open_p, 2),
            round(high_p, 2),
            round(low_p, 2),
            round(close_p, 2),
            round(volume, 2)
        ])

        # Increment time by 1 minute
        current_time += timedelta(minutes=1)

print(f"Generated {num_rows} rows in {filename}")
