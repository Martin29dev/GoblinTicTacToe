"""Generuje pliki audio do gry (czysty Python, bez dodatkowych bibliotek).
Uruchom:  python tools/generate_audio.py   -> zapisuje pliki WAV do folderu assets/
"""
import math, os, random, struct, wave

RATE = 22050
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")

def midi(n): return 440.0 * 2 ** ((n - 69) / 12)

def pulse(phase, duty=0.25): return 1.0 if (phase % 1.0) < duty else -1.0
def triangle(phase): return 4 * abs((phase % 1.0) - 0.5) - 1
def square(phase): return pulse(phase, 0.5)
def saw(phase): return 2 * (phase % 1.0) - 1

def tone(freq, dur, wave_fn, vol=0.5, attack=0.005, decay=6.0, slide_to=None):
    n = int(RATE * dur)
    out, phase = [], 0.0
    for i in range(n):
        t = i / RATE
        f = freq if slide_to is None else freq + (slide_to - freq) * (i / n)
        phase += f / RATE
        env = min(1.0, t / attack) * math.exp(-decay * t / dur)
        out.append(wave_fn(phase) * vol * env)
    return out

def mix_into(track, samples, start):
    for i, s in enumerate(samples):
        if start + i < len(track): track[start + i] += s

def save(name, samples, peak=0.85):
    m = max(1e-9, max(abs(s) for s in samples))
    data = b"".join(struct.pack("<h", int(max(-1, min(1, s / m * peak)) * 32767)) for s in samples)
    with wave.open(os.path.join(OUT, name), "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(RATE); w.writeframes(data)
    print("zapisano", name)

# ---------- efekty ----------
def make_click():  save("click.wav", tone(1300, 0.07, square, 0.6, decay=5, slide_to=800))

def make_enemy():  save("enemy.wav", tone(300, 0.12, triangle, 0.7, decay=4, slide_to=180))

def make_win():
    s = []
    for n in (72, 76, 79, 84):                       # C E G C
        s += tone(midi(n), 0.14, lambda p: pulse(p, 0.25), 0.6, decay=3)
    s += tone(midi(84), 0.5, lambda p: pulse(p, 0.25), 0.6, decay=3)
    save("win.wav", s)

def make_lose():
    s = []
    for f in (330, 262, 196):
        s += tone(f, 0.22, square, 0.5, decay=2.5, slide_to=f * 0.9)
    s += tone(130, 0.6, saw, 0.5, decay=3, slide_to=90)
    save("lose.wav", s)

def make_swish():                                    # "szum" przejscia
    random.seed(1)
    n = int(RATE * 0.45); out = []; lp = 0.0
    for i in range(n):
        t = i / n
        lp += (random.uniform(-1, 1) - lp) * (0.05 + 0.5 * math.sin(math.pi * t))
        out.append(lp * math.sin(math.pi * t) ** 2)
    save("swish.wav", out, 0.6)

# ---------- muzyka (A-moll, ok. 20 s, zapetlona) ----------
def make_music():
    bpm = 96
    eighth = 60.0 / bpm / 2
    melody = [
        [69,72,76,72,69,72,76,72], [65,69,72,69,65,69,72,69],
        [67,72,76,72,67,72,76,72], [67,71,74,71,67,71,74,79],
        [76,74,72,74,76,81,76,72], [77,76,72,76,77,81,77,72],
        [79,76,72,76,79,76,72,67], [76,71,68,71,76,71,68,71],
    ]
    bass = [45, 41, 48, 43, 45, 41, 48, 40]
    total = int(RATE * eighth * 8 * len(melody))
    track = [0.0] * (total + RATE)
    for bar, notes in enumerate(melody):
        t0 = bar * 8
        for i, n in enumerate(notes):
            mix_into(track, tone(midi(n), eighth * 1.6, lambda p: pulse(p, 0.25), 0.22, decay=4),
                     int((t0 + i) * eighth * RATE))
        for beat in (0, 4):                          # bas na 1 i 3
            mix_into(track, tone(midi(bass[bar]), eighth * 3.8, triangle, 0.5, decay=2),
                     int((t0 + beat) * eighth * RATE))
    save("music.wav", track[:total], 0.7)

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    make_click(); make_enemy(); make_win(); make_lose(); make_swish(); make_music()
