"""
DSP với Python — Bài 1: Tín hiệu cơ bản & Sampling
====================================================
Chạy: python lesson.py
Mỗi phần sẽ hiện 1 cửa sổ đồ thị. Đóng cửa sổ để chuyển sang phần tiếp theo.
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy import signal

# ── Cấu hình chung ──────────────────────────────────────────────────────────
plt.rcParams['figure.figsize'] = (13, 4)
plt.rcParams['lines.linewidth'] = 1.5
plt.rcParams['font.size'] = 11


# ════════════════════════════════════════════════════════════════════════════
# PHẦN 1 — Sóng sin cơ bản
# ════════════════════════════════════════════════════════════════════════════
print("\n[1/5] Sóng sin cơ bản — đóng cửa sổ để tiếp tục...")

fs       = 1000          # Sampling rate: 1000 Hz (1000 mẫu/giây)
duration = 1.0           # 1 giây
f_sig    = 5             # Tần số tín hiệu: 5 Hz

t = np.arange(0, duration, 1 / fs)        # Mảng thời gian
x = np.sin(2 * np.pi * f_sig * t)         # Công thức sóng sin

print(f"  Số mẫu (samples): {len(t)}")
print(f"  Khoảng cách giữa 2 mẫu: {1/fs*1000:.1f} ms")

fig, axes = plt.subplots(1, 2)
fig.suptitle("Phần 1 — Sóng sin cơ bản", fontweight="bold")

axes[0].plot(t, x, color="steelblue")
axes[0].set(title=f"Toàn bộ 1 giây | f={f_sig}Hz, fs={fs}Hz",
            xlabel="Thời gian (s)", ylabel="Biên độ")
axes[0].grid(True, alpha=0.3)

# Zoom vào 200ms đầu để thấy từng sample
n_show = int(0.2 * fs)
axes[1].plot(t[:n_show], x[:n_show], "o-", color="steelblue",
             markersize=3, label="Mỗi chấm = 1 sample")
axes[1].set(title="Zoom 200ms đầu (thấy rõ từng sample)",
            xlabel="Thời gian (s)", ylabel="Biên độ")
axes[1].grid(True, alpha=0.3)
axes[1].legend()

plt.tight_layout()
plt.show()


# ════════════════════════════════════════════════════════════════════════════
# PHẦN 2 — Định lý Nyquist
# ════════════════════════════════════════════════════════════════════════════
print("[2/5] Định lý Nyquist — đóng cửa sổ để tiếp tục...")

f_true = 10   # Tín hiệu thật 10 Hz

# Giả lập tín hiệu liên tục bằng fs rất cao
t_cont = np.linspace(0, 0.5, 5000)
x_cont = np.sin(2 * np.pi * f_true * t_cont)

cases = [
    {"fs": 200, "label": "fs=200Hz\n(đủ: 200 >> 2×10)",  "color": "green",      "status": "✅ Tốt"},
    {"fs": 25,  "label": "fs=25Hz\n(tối thiểu: 25>2×10)", "color": "darkorange", "status": "⚠️ Tạm"},
    {"fs": 12,  "label": "fs=12Hz\n(THIẾU: 12<2×10)",    "color": "red",         "status": "❌ Lỗi!"},
]

fig, axes = plt.subplots(1, 3, figsize=(16, 4))
fig.suptitle(f"Phần 2 — Nyquist: Tín hiệu {f_true}Hz với các Sampling Rate khác nhau",
             fontweight="bold")

for ax, c in zip(axes, cases):
    t_s = np.arange(0, 0.5, 1 / c["fs"])
    x_s = np.sin(2 * np.pi * f_true * t_s)

    ax.plot(t_cont, x_cont, color="lightgray", linewidth=2, label="Tín hiệu gốc")
    ax.stem(t_s, x_s, linefmt=c["color"], markerfmt="o", basefmt="k-")
    ax.plot(t_s, x_s, "--", color=c["color"], alpha=0.7, label="Tái tạo")
    ax.set(title=f"{c['status']} {c['label']}", xlabel="Thời gian (s)", ylabel="Biên độ")
    ax.grid(True, alpha=0.3)
    ax.legend(fontsize=9)

plt.tight_layout()
plt.show()


# ════════════════════════════════════════════════════════════════════════════
# PHẦN 3 — Aliasing
# ════════════════════════════════════════════════════════════════════════════
print("[3/5] Aliasing — đóng cửa sổ để tiếp tục...")

fs_adc  = 100                    # STM32 ADC: 100 Hz
f_real  = 90                     # Tín hiệu thật: 90 Hz (vi phạm Nyquist)
f_alias = abs(f_real - fs_adc)   # Tần số "ma" xuất hiện: 10 Hz

t_cont    = np.linspace(0, 0.5, 10000)
t_sampled = np.arange(0, 0.5, 1 / fs_adc)

fig, ax = plt.subplots(figsize=(13, 5))
ax.plot(t_cont, np.sin(2*np.pi*f_real*t_cont),
        color="lightblue", linewidth=1, alpha=0.8, label=f"Tín hiệu THẬT: {f_real}Hz")
ax.plot(t_cont, np.sin(2*np.pi*f_alias*t_cont),
        color="red", linewidth=2, linestyle="--", label=f"ALIAS thấy: {f_alias}Hz (SAI!)")
ax.stem(t_sampled, np.sin(2*np.pi*f_real*t_sampled),
        linefmt="gray", markerfmt="ko", basefmt="k-")

ax.set(title=f"Aliasing: ADC {fs_adc}Hz đọc tín hiệu {f_real}Hz → thấy {f_alias}Hz (không có thật!)",
       xlabel="Thời gian (s)", ylabel="Biên độ")
ax.legend()
ax.grid(True, alpha=0.3)

print(f"  ADC @ {fs_adc}Hz đọc tín hiệu {f_real}Hz")
print(f"  Nyquist yêu cầu fs > {2*f_real}Hz → VI PHẠM!")
print(f"  Kết quả: MCU 'thấy' tín hiệu {f_alias}Hz — SAI HOÀN TOÀN!")
print(f"  Giải pháp: đặt Low-pass Anti-aliasing Filter TRƯỚC chân ADC")

plt.tight_layout()
plt.show()


# ════════════════════════════════════════════════════════════════════════════
# PHẦN 4 — Tín hiệu phức tạp (giống thực tế: rung động motor)
# ════════════════════════════════════════════════════════════════════════════
print("[4/5] Tín hiệu phức tạp — đóng cửa sổ để tiếp tục...")

fs = 2000
t  = np.arange(0, 1.0, 1/fs)

np.random.seed(42)
components = [
    (50,  1.0, "Tần số quay motor: 50Hz",    "steelblue"),
    (150, 0.5, "Harmonic bậc 3: 150Hz",       "darkorange"),
    (300, 0.3, "Harmonic bậc 6: 300Hz",       "green"),
    (0,   0.2, "Noise (EMI, rung môi trường)", "gray"),   # đặc biệt xử lý dưới
]

sigs = [
    1.0 * np.sin(2*np.pi*50*t),
    0.5 * np.sin(2*np.pi*150*t),
    0.3 * np.sin(2*np.pi*300*t),
    0.2 * np.random.randn(len(t)),
]
x_composite = sum(sigs)

labels = [c[2] for c in components] + ["Tín hiệu TỔNG HỢP (sensor thực tế)"]
colors = [c[3] for c in components] + ["darkred"]
all_sigs = sigs + [x_composite]

fig, axes = plt.subplots(5, 1, figsize=(13, 14))
fig.suptitle("Phần 4 — Tín hiệu phức tạp = Nhiều thành phần + Noise", fontweight="bold")

show = slice(0, 400)  # 200ms
for ax, sig, label, color in zip(axes, all_sigs, labels, colors):
    ax.plot(t[show]*1000, sig[show], color=color, linewidth=1.2)
    ax.set(title=label, ylabel="Biên độ")
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, 200)

axes[-1].set_xlabel("Thời gian (ms)")
print("  Nhìn vào tín hiệu tổng hợp → rất khó nhận ra từng thành phần!")
print("  → Đó là lý do cần FFT (Bài 2) để phân tích phổ tần số.")
plt.tight_layout()
plt.show()


# ════════════════════════════════════════════════════════════════════════════
# PHẦN 5 — Thử nghiệm tự do (thay số và chạy lại)
# ════════════════════════════════════════════════════════════════════════════
print("[5/5] Bài tập tự do — chỉnh thông số ở đây rồi Ctrl+S → F5 để chạy lại!")

# ======= THAY ĐỔI Ở ĐÂY =======
fs_exp  = 1000   # Thử: 50, 200, 5000
f1_exp  = 10     # Tần số sóng 1 (Hz)
f2_exp  = 30     # Tần số sóng 2 (Hz)
A1_exp  = 1.0    # Biên độ sóng 1
A2_exp  = 0.5    # Biên độ sóng 2
noise   = 0.1    # Mức nhiễu: thử 0.0, 0.5, 1.0
# ================================

t_exp = np.arange(0, 1.0, 1/fs_exp)
x_exp = (A1_exp * np.sin(2*np.pi*f1_exp*t_exp) +
         A2_exp * np.sin(2*np.pi*f2_exp*t_exp) +
         noise  * np.random.randn(len(t_exp)))

f_max = max(f1_exp, f2_exp)
ok    = fs_exp >= 2 * f_max
status_txt = "✅ Nyquist OK" if ok else f"❌ VI PHẠM! Cần fs > {2*f_max}Hz"

fig, ax = plt.subplots(figsize=(13, 4))
show_n = min(len(t_exp), int(0.3*fs_exp))
ax.plot(t_exp[:show_n]*1000, x_exp[:show_n], color="steelblue")
ax.set(title=f"Thử nghiệm — fs={fs_exp}Hz | f1={f1_exp}Hz | f2={f2_exp}Hz",
       xlabel="Thời gian (ms)", ylabel="Biên độ")
ax.text(0.98, 0.05, status_txt, transform=ax.transAxes, ha="right",
        color="green" if ok else "red", fontsize=12, fontweight="bold")
ax.grid(True, alpha=0.3)

print(f"  Nyquist limit: {fs_exp/2}Hz | f_max trong tín hiệu: {f_max}Hz → {status_txt}")
plt.tight_layout()
plt.show()

print("\n✅ Xong Bài 1! Bài tiếp theo: 02_fft_analysis/lesson.py")