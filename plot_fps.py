import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import sys, os

# --- Load CSV ---
csv_path = sys.argv[1] if len(sys.argv) > 1 else "vw_session_perf.csv"
df = pd.read_csv(csv_path)

# --- Prepare Data ---
for col in ['avg_fps', 'min_fps', 'max_fps']:
    df[col] = pd.to_numeric(df[col], errors='coerce')

# Parse timestamps; keep timezone if present, then convert to UTC-naive for plotting
dt = pd.to_datetime(df['created_at'], errors='coerce', utc=True)
df['created_at'] = dt.dt.tz_convert('UTC').dt.tz_localize(None)
df = df.sort_values('created_at')

# Shorter labels for the session-id plot
df['session_short'] = df['session_id'].str.slice(0, 6)

# -------------------------------
# Plot 1: FPS over time (Oct 2025 →)
# -------------------------------
fig1 = plt.figure(figsize=(10, 5))
plt.plot(df['created_at'], df['avg_fps'], 'o-', label='Average FPS', linewidth=2)
plt.plot(df['created_at'], df['min_fps'], 'x--', label='Min FPS', linewidth=1.5, markersize=7)
plt.plot(df['created_at'], df['max_fps'], '^--', label='Max FPS', linewidth=1.5, markersize=7)

plt.title('Session Performance Over Time')
plt.xlabel('Session Month')
plt.ylabel('Frames Per Second (FPS)')
plt.legend()
plt.grid(True)

ax = plt.gca()
# Month ticks & labels like "Oct 2025"
ax.xaxis.set_major_locator(mdates.MonthLocator(interval=1))
ax.xaxis.set_major_formatter(mdates.DateFormatter('%b %Y'))
plt.xticks(rotation=30, ha='right')

# Force x-axis to start at Oct 1, 2025
start = pd.Timestamp(2025, 10, 1)
end = df['created_at'].max() if pd.notna(df['created_at'].max()) else start
# pad end a bit so the point isn't on the edge
end = end + pd.Timedelta(days=7)
plt.xlim(start, end)

# Tight y-range padding if values are very close
y_all = pd.concat([df['avg_fps'], df['min_fps'], df['max_fps']]).dropna()
if not y_all.empty:
    y_min, y_max = y_all.min(), y_all.max()
    if y_max - y_min < 5:
        pad = max(1.0, (5 - (y_max - y_min)) / 2)
        plt.ylim(y_min - pad, y_max + pad)

plt.tight_layout()
out1 = os.path.splitext(csv_path)[0] + "_time_graph.png"
plt.savefig(out1, dpi=150)
print(f"Saved: {out1}")

# -------------------------------
# Plot 2: FPS by Session ID
# -------------------------------
fig2 = plt.figure(figsize=(10, 5))
plt.plot(df['session_short'], df['avg_fps'], 'o-', label='Average FPS', linewidth=2)
plt.plot(df['session_short'], df['min_fps'], 'x--', label='Min FPS', linewidth=1.5, markersize=7)
plt.plot(df['session_short'], df['max_fps'], '^--', label='Max FPS', linewidth=1.5, markersize=7)

plt.title('Session Performance by Test Session')
plt.xlabel('Session ID (short)')
plt.ylabel('Frames Per Second (FPS)')
plt.legend()
plt.grid(True)
plt.tight_layout()
out2 = os.path.splitext(csv_path)[0] + "_session_graph.png"
plt.savefig(out2, dpi=150)
print(f"Saved: {out2}")

plt.show()
