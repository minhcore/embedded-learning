# 📖 Glossary — Tất cả hàm dùng trong khóa DSP

Mỗi khi gặp hàm lạ trong `lesson.py`, tra ở đây trước.
Cấu trúc mỗi mục: **Hàm → Đọc là → Làm gì → Ví dụ → Hay nhầm ở đâu**

---

## 🔷 NumPy (`import numpy as np`)

NumPy = thư viện tính toán số học. Nền tảng của mọi thứ.

---

### `np.arange(start, stop, step)`
**Đọc là:** "a-range" (array range)
**Làm gì:** Tạo mảng số từ `start` đến `stop`, bước nhảy `step`. Không bao gồm `stop`.
```python
np.arange(0, 1, 0.1)
# → [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9]
#    (không có 1.0 ← stop bị loại)

# Dùng trong DSP:
fs = 1000
t = np.arange(0, 1.0, 1/fs)
# → mảng thời gian: [0, 0.001, 0.002, ..., 0.999]
# → 1000 phần tử = 1000 mẫu trong 1 giây
```
**Hay nhầm:** `np.arange(0, 1, 0.1)` cho 10 phần tử, không phải 11. Stop bị loại.

---

### `np.linspace(start, stop, num)`
**Đọc là:** "lin-space" (linear space)
**Làm gì:** Tạo mảng `num` điểm chia đều từ `start` đến `stop`. Bao gồm `stop`.
```python
np.linspace(0, 1, 5)
# → [0.0, 0.25, 0.5, 0.75, 1.0]
#    (có 1.0 ← stop được bao gồm)

# Dùng khi cần mảng "liên tục" để vẽ đường mượt:
t_smooth = np.linspace(0, 0.5, 5000)  # 5000 điểm cho đường mượt
```
**Khác `arange`:** `arange` dùng bước nhảy, `linspace` dùng số lượng điểm.

---

### `np.sin(x)` / `np.cos(x)`
**Làm gì:** Tính sin/cos của từng phần tử trong mảng (đơn vị: radian).
```python
np.sin(np.pi / 2)   # → 1.0  (sin 90°)
np.sin(0)           # → 0.0

# Dùng để tạo tín hiệu:
x = np.sin(2 * np.pi * 5 * t)
#           ↑        ↑  ↑  ↑
#           2π       f  Hz  mảng thời gian
```
**Tại sao có `2 * np.pi`?** Vì sin nhận radian. 1 chu kỳ = 2π radian. Nếu thiếu 2π, tần số sẽ sai 6.28 lần.

---

### `np.pi`
**Làm gì:** Hằng số π ≈ 3.14159...
```python
np.pi   # → 3.141592653589793
2 * np.pi  # → 6.283185...  (= 1 chu kỳ tròn)
```

---

### `np.random.randn(N)`
**Đọc là:** "random normal" (phân phối chuẩn)
**Làm gì:** Tạo `N` số ngẫu nhiên theo phân phối Gaussian (chuông). Giá trị trung bình = 0, độ lệch chuẩn = 1.
```python
noise = 0.2 * np.random.randn(1000)
#       ↑    ↑                 ↑
#  biên độ  nhân để scale    số mẫu

# 0.2 * randn → noise nhỏ hơn (±0.2 thay vì ±1)
```
**Dùng để làm gì?** Giả lập nhiễu điện từ (EMI), nhiễu môi trường, noise của ADC.

---

### `np.random.seed(N)`
**Làm gì:** Cố định kết quả random. Chạy lại vẫn ra cùng số.
```python
np.random.seed(42)   # Fix seed
np.random.randn(3)   # → [0.496, -0.138, 0.647] — luôn như vậy
```
**Tại sao cần?** Để kết quả reproducible — bạn và mình chạy code ra cùng đồ thị.

---

### `np.abs(x)`
**Làm gì:** Giá trị tuyệt đối của từng phần tử.
```python
np.abs([-3, 1, -5])  # → [3, 1, 5]

# Dùng trong FFT: lấy magnitude của số phức
magnitude = np.abs(fft_result)
```

---

### `np.fft.rfft(x)` ← dùng trong Bài 2
**Đọc là:** "r-fft" (real FFT)
**Làm gì:** Tính FFT cho tín hiệu thực (real-valued). Trả về mảng số phức.
```python
X = np.fft.rfft(x)      # x là tín hiệu, X là phổ tần số (số phức)
magnitude = np.abs(X)   # Lấy biên độ từ số phức
```
**Tại sao dùng rfft thay vì fft?** Tín hiệu thực → phổ đối xứng → rfft chỉ trả nửa, nhanh hơn, đủ dùng.

---

### `np.fft.rfftfreq(N, d)` ← dùng trong Bài 2
**Làm gì:** Tạo mảng tần số (trục x) tương ứng với kết quả rfft.
```python
freqs = np.fft.rfftfreq(N=1000, d=1/fs)
# N  = số mẫu
# d  = khoảng cách giữa 2 mẫu = 1/fs
# → [0, 1, 2, 3, ..., 500] Hz
```
**Hay quên:** Thiếu `d=1/fs` thì trục x sẽ là index, không phải Hz.

---

### `np.zeros(N)` / `np.ones(N)`
**Làm gì:** Tạo mảng N phần tử toàn 0 hoặc toàn 1.
```python
np.zeros(5)  # → [0., 0., 0., 0., 0.]
np.ones(5)   # → [1., 1., 1., 1., 1.]

# Dùng để khởi tạo buffer trống
buffer = np.zeros(1024)  # Buffer 1024 mẫu, chờ dữ liệu điền vào
```

---

### `np.len(x)` → thực ra là `len(x)`
**Làm gì:** Trả về số phần tử trong mảng.
```python
t = np.arange(0, 1, 0.001)
len(t)   # → 1000
```

---

## 🔷 Matplotlib (`import matplotlib.pyplot as plt`)

Matplotlib = thư viện vẽ đồ thị.

---

### `plt.subplots(rows, cols)`
**Làm gì:** Tạo figure với nhiều ô đồ thị (gọi là `axes`).
```python
fig, axes = plt.subplots(1, 2)
# → 1 hàng, 2 cột → 2 ô đồ thị nằm ngang
# fig   = cửa sổ chứa tất cả
# axes  = mảng [ax0, ax1] — mỗi ax là 1 ô

fig, axes = plt.subplots(3, 1, figsize=(12, 8))
# → 3 hàng, 1 cột → 3 ô xếp dọc
# figsize=(rộng, cao) tính bằng inch
```

---

### `ax.plot(x, y)`
**Làm gì:** Vẽ đường nối các điểm.
```python
ax.plot(t, signal)                          # Đường đơn giản
ax.plot(t, signal, color='steelblue')       # Chỉ định màu
ax.plot(t, signal, 'o-', markersize=3)      # Đường + chấm tròn
ax.plot(t, signal, '--', alpha=0.7)         # Đường đứt, trong suốt 70%
```

---

### `ax.stem(x, y)`
**Làm gì:** Vẽ các "cọc" từ 0 lên từng điểm. Dùng để hiển thị tín hiệu rời rạc.
```python
ax.stem(t_samples, x_samples)
# → thấy rõ từng sample riêng lẻ, không nối liền
```
**Dùng khi nào?** Khi muốn nhấn mạnh tính "rời rạc" của tín hiệu số.

---

### `ax.set(title=..., xlabel=..., ylabel=...)`
**Làm gì:** Đặt tiêu đề và nhãn trục — gộp nhiều lệnh vào 1 dòng.
```python
# Cách 1 (dài):
ax.set_title('Tín hiệu')
ax.set_xlabel('Thời gian (s)')
ax.set_ylabel('Biên độ')

# Cách 2 (ngắn hơn, kết quả như nhau):
ax.set(title='Tín hiệu', xlabel='Thời gian (s)', ylabel='Biên độ')
```

---

### `ax.grid(True, alpha=0.3)`
**Làm gì:** Bật lưới, `alpha` là độ trong suốt (0=vô hình, 1=đặc).
```python
ax.grid(True, alpha=0.3)   # Lưới mờ nhạt (nhìn dễ hơn)
ax.grid(True, alpha=1.0)   # Lưới đậm (che khuất đồ thị)
```

---

### `ax.text(x, y, 'nội dung', transform=...)`
**Làm gì:** Thêm chữ vào đồ thị tại vị trí (x, y).
```python
ax.text(0.98, 0.05, '✅ OK',
        transform=ax.transAxes,  # tọa độ 0-1 theo tỉ lệ ô, không theo giá trị thật
        ha='right')              # ha = horizontal alignment
```

---

### `plt.tight_layout()`
**Làm gì:** Tự động căn chỉnh khoảng cách giữa các ô, tránh text bị đè.
```python
plt.tight_layout()   # Luôn gọi trước plt.show()
plt.show()
```

---

### `plt.show()`
**Làm gì:** Hiển thị cửa sổ đồ thị. Chương trình dừng lại chờ bạn đóng cửa sổ.

---

## 🔷 SciPy Signal (`from scipy import signal`)

scipy.signal = các hàm DSP chuyên dụng.

---

### `signal.square(x)` 
**Làm gì:** Tạo sóng vuông từ mảng phase.
```python
t = np.arange(0, 1, 0.001)
sq = signal.square(2 * np.pi * 5 * t)
# → sóng vuông 5Hz, giá trị chỉ là +1 hoặc -1
```

---

### `signal.sawtooth(x)`
**Làm gì:** Tạo sóng răng cưa.
```python
saw = signal.sawtooth(2 * np.pi * 5 * t)
# → tăng tuyến tính rồi reset về -1
```

---

### `signal.chirp(t, f0, f1, t1, method)` 
**Làm gì:** Tạo tín hiệu sweep — tần số tăng dần từ `f0` lên `f1` trong thời gian `t1`.
```python
chirp = signal.chirp(t, f0=1, f1=100, t1=1.0, method='linear')
# → tần số bắt đầu 1Hz, tăng lên 100Hz trong 1 giây
```
**Dùng để làm gì?** Kiểm tra đáp ứng tần số của hệ thống, SONAR, kiểm tra loa.

---

### `signal.butter(N, Wn, btype, fs)` ← dùng trong Bài 3
**Đọc là:** "butter" (Butterworth filter)
**Làm gì:** Thiết kế bộ lọc Butterworth bậc `N`.
```python
sos = signal.butter(N=4, Wn=100, btype='low', fs=1000, output='sos')
# N     = bậc filter (càng cao càng dốc, càng tốn tài nguyên MCU)
# Wn    = tần số cắt (Hz) — nếu lowpass thì tần số này trở xuống được giữ lại
# btype = 'low' / 'high' / 'band'
# fs    = sampling rate
# output='sos' → định dạng ổn định nhất (dùng cái này, không dùng 'ba')
```

---

### `signal.sosfilt(sos, x)` ← dùng trong Bài 3
**Đọc là:** "sos-filt" (second-order sections filter)
**Làm gì:** Áp dụng filter lên tín hiệu `x`.
```python
x_filtered = signal.sosfilt(sos, x)
# sos = hệ số filter (từ butter, firwin...)
# x   = tín hiệu đầu vào
# trả về tín hiệu đã lọc
```
**Tại sao không dùng `lfilter`?** `sosfilt` ổn định số hơn, ít bị lỗi floating point ở filter bậc cao.

---

### `signal.freqz(b, a, fs)` ← dùng trong Bài 3
**Đọc là:** "freq-z"
**Làm gì:** Tính đáp ứng tần số của filter — xem filter lọc tần số nào, giữ tần số nào.
```python
w, h = signal.freqz(b, a, fs=1000)
# w = mảng tần số
# h = đáp ứng (số phức) — lấy abs để thấy magnitude
plt.plot(w, np.abs(h))
```

---

## 🔷 Python built-in (không cần import)

---

### `len(x)`
Số phần tử trong list/mảng.

### `abs(x)`
Giá trị tuyệt đối. (Dùng `np.abs()` cho mảng numpy, `abs()` cho số đơn lẻ.)

### `min(a, b)` / `max(a, b)`
Giá trị nhỏ nhất / lớn nhất.

### `slice(start, stop)`
Cắt mảng theo index.
```python
show = slice(0, 400)
t[show]   # tương đương t[0:400]
```

### `f-string: f"..."` 
Nhúng biến vào chuỗi.
```python
fs = 1000
print(f"Sampling rate: {fs}Hz")         # → "Sampling rate: 1000Hz"
print(f"Nyquist: {fs/2:.1f}Hz")         # → "Nyquist: 500.0Hz"  (.1f = 1 chữ số thập phân)
```

---

## 📌 Cheatsheet nhanh — Khi nào dùng cái gì

| Muốn làm gì | Dùng hàm |
|---|---|
| Tạo mảng thời gian | `np.arange(0, duration, 1/fs)` |
| Tạo sóng sin | `np.sin(2 * np.pi * f * t)` |
| Thêm noise | `+ noise_level * np.random.randn(len(t))` |
| Vẽ 1 đồ thị | `fig, ax = plt.subplots()` |
| Vẽ nhiều đồ thị | `fig, axes = plt.subplots(rows, cols)` |
| Tính FFT | `np.fft.rfft(x)` |
| Lấy biên độ FFT | `np.abs(fft_result)` |
| Trục tần số FFT | `np.fft.rfftfreq(len(x), 1/fs)` |
| Thiết kế filter | `signal.butter(N, Wn, btype, fs)` |
| Áp dụng filter | `signal.sosfilt(sos, x)` |