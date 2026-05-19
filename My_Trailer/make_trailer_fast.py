"""
Arcane Souls Rebirth - FYP Trailer v4
- NO letterbox bars
- Game audio PRESERVED
- Subtitles at bottom with semi-transparent background
- Vignette + film grain + color grade
- Fade in/out per section
"""
import os, subprocess, shutil
from PIL import Image, ImageDraw, ImageFont
import imageio_ffmpeg
FFMPEG = imageio_ffmpeg.get_ffmpeg_exe()

BASE  = os.path.dirname(os.path.abspath(__file__))
FTG   = os.path.join(BASE, "Footage")
SLATE = os.path.join(FTG, "3SecondsOpeningSlateTemplate.png")
DEMO  = os.path.join(FTG, "UnrealEditor_HhKTcv7XnR-00.00.00.000-00.02.54.937-seg1.mp4")
NARR  = os.path.join(FTG, "narration.mp4")
OUT   = os.path.join(BASE, "BScCMFYP_Group02_AIGen.mp4")
BGMC  = [os.path.join(os.path.dirname(BASE),"Content","Audio","BGM_JapaneseShrineTheme.mp3"),
         os.path.join(BASE, "BGM_JapaneseShrineTheme.mp3")]
BGM   = next((p for p in BGMC if os.path.exists(p)), "")
TMP   = os.path.join(BASE, "_tmp"); os.makedirs(TMP, exist_ok=True)

W, H, FPS = 1920, 1080, 24
FD = r"C:\Windows\Fonts"
FI = os.path.join(FD,"impact.ttf")
FB = os.path.join(FD,"arialbd.ttf")
FR = os.path.join(FD,"arial.ttf")

SECS = [
    (  0,40,"CONCEPT",
       ["Arcane Souls Rebirth - a third-person action game in Unreal Engine 5",
        "inspired by Sekiro: Shadows Die Twice"]),
    ( 40,45,"AIMS AND OBJECTIVE",
       ["Recreate skill-based combat with precise parrying,",
        "posture mechanics, and intelligent Boss AI"]),
    ( 85,45,"PRODUCTION PROCESS",
       ["Built entirely in C++ on UE5.5",
        "Combat Component | AI Behavior Tree | Posture System | Slate UI"]),
    (130,44,"KEY ACHIEVEMENT",
       ["Posture-break and execution system. Autonomous AI attack loop.",
        "Full game loop: difficulty selection, death/respawn, pause menu"]),
]

def ff(*args, desc=""):
    cmd = [FFMPEG,"-y"] + list(args)
    print(f"  [{desc}]")
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        print(f"  ERR: {r.stderr[-500:]}")
        raise RuntimeError(desc)
    print(f"  OK: {desc}")

def ep(p):
    """Escape path for drawtext on Windows"""
    s = p.replace("\\","/")
    return s[:1] + "\\:" + s[2:]

def card(items, path):
    img = Image.new("RGB",(W,H),(0,0,0))
    d = ImageDraw.Draw(img)
    for it in items:
        fnt = ImageFont.truetype(it["f"],it["s"])
        bb = d.textbbox((0,0),it["t"],font=fnt)
        x = it.get("x",(W-(bb[2]-bb[0]))//2)
        d.text((x,it["y"]),it["t"],fill=it["c"],font=fnt)
    img.save(path)

def wtxt(text, name):
    p = os.path.join(TMP, name)
    with open(p,"w",encoding="utf-8") as f: f.write(text)
    return p

# ── PNG clip with SILENT audio track (so concat works) ──
def png2mp4_audio(png, mp4, dur, fi=0.8, fo=0.5):
    fos = dur - fo
    ff("-loop","1","-i",png,
       "-f","lavfi","-i","anullsrc=r=48000:cl=stereo",
       "-t",str(dur),
       "-vf",f"scale={W}:{H},fade=t=in:st=0:d={fi},fade=t=out:st={fos}:d={fo}",
       "-c:v","libx264","-pix_fmt","yuv420p","-r",str(FPS),
       "-c:a","aac","-b:a","128k","-shortest",
       mp4, desc=f"png2mp4 {os.path.basename(mp4)}")

# ── Clips ──
def make_slate(out):
    print("\n=== Slate ===")
    png2mp4_audio(SLATE, out, 3.0, fi=0.3, fo=0.3)

def make_title(out):
    print("\n=== Title Card ===")
    p = os.path.join(TMP,"title.png")
    card([
        {"t":"ARCANE SOULS","f":FI,"s":108,"c":"#FFD700","y":H//2-130},
        {"t":"REBIRTH",     "f":FB,"s":52, "c":"#CCCCCC","y":H//2+20},
        {"t":"BScCM Final Year Project  |  City University of Hong Kong  |  2025-2026",
                            "f":FR,"s":21, "c":"#555555","y":H-75},
    ], p)
    png2mp4_audio(p, out, 5.0, fi=1.0, fo=0.5)

def make_section(i, start, dur, title, body_lines, out):
    print(f"\n=== Section {i+1}: {title} ===")

    # Write subtitle text to files
    t_file = wtxt(f"[ {title} ]", f"s{i}_t.txt")
    b_files = [wtxt(ln, f"s{i}_b{j}.txt") for j,ln in enumerate(body_lines)]

    fb_e = ep(FB); fr_e = ep(FR)

    # Subtitle Y: bottom of screen (inside frame, no black bar)
    # Title line at y=H-90, body lines below
    yt = H - 95
    ybs = [H - 65 + j*30 for j in range(len(body_lines))]

    def dt(txtfile, font_e, size, color, y):
        tf = ep(txtfile)
        return (f"drawtext=fontfile='{font_e}':textfile='{tf}':"
                f"fontsize={size}:fontcolor={color}:"
                f"x=(w-text_w)/2:y={y}:"
                f"box=1:boxcolor=black@0.55:boxborderw=6")

    dts  = [dt(t_file, fb_e, 20, "gold", yt)]
    dts += [dt(b_files[j], fr_e, 19, "white", ybs[j]) for j in range(len(b_files))]

    vf = ",".join([
        f"scale={W}:{H}",
        # Color grade (warm cinematic)
        "curves=r='0/0 0.5/0.56 1/1':g='0/0 0.5/0.49 1/0.98':b='0/0 0.5/0.44 1/0.88'",
        "eq=contrast=1.05:saturation=1.08",
        # Vignette
        "vignette=angle=PI/5",
        # Film grain (subtle)
        "noise=alls=6:allf=t",
    ] + dts + [
        # Fade video
        "fade=t=in:st=0:d=0.5",
        f"fade=t=out:st={dur-0.5}:d=0.5",
    ])

    # Audio fade in/out too (game sounds)
    af = f"afade=t=in:st=0:d=0.5,afade=t=out:st={dur-0.5}:d=0.5"

    ff("-ss",str(start),"-t",str(dur),"-i",DEMO,
       "-vf",vf,"-af",af,
       "-c:v","libx264","-pix_fmt","yuv420p","-r",str(FPS),
       "-c:a","aac","-b:a","192k",
       out, desc=f"section{i+1} {start}s-{start+dur}s")

def make_end(out):
    print("\n=== End Card ===")
    p = os.path.join(TMP,"end.png")
    card([
        {"t":"Arcane Souls: Rebirth",             "f":FB,"s":40,"c":"#CCCCCC","y":H//2-70},
        {"t":"BScCM Final Year Project 2025~2026","f":FR,"s":28,"c":"#888888","y":H//2+10},
        {"t":"City University of Hong Kong",      "f":FR,"s":28,"c":"#888888","y":H//2+50},
    ], p)
    png2mp4_audio(p, out, 4.0, fi=1.0, fo=1.5)

# ── Concat (preserves audio!) ──
def concat_all(parts, out):
    print("\n=== Concat ===")
    lst = os.path.join(TMP,"list.txt")
    with open(lst,"w") as f:
        for p in parts: f.write(f"file '{p}'\n")
    ff("-f","concat","-safe","0","-i",lst,
       "-c:v","libx264","-pix_fmt","yuv420p","-r",str(FPS),
       "-c:a","aac","-b:a","192k",
       out, desc="concat (with game audio)")

# ── Audio mix: game audio + BGM + narration ──
def mix_audio(video_in, video_out):
    print("\n=== Audio Mix ===")
    total = 3+5+40+45+45+44+4  # 186s
    inputs = ["-i", video_in]
    filters = []
    labels = []
    n = 1

    # [0:a] = game audio from concat (already there!)
    labels.append("[0:a]")

    if BGM and os.path.exists(BGM):
        inputs += ["-i", BGM]
        filters.append(
            f"[{n}:a]aloop=loop=-1:size=2e+09,"
            f"atrim=0:{total},"
            f"afade=t=in:st=0:d=2,"
            f"afade=t=out:st={total-3}:d=3,"
            f"volume=0.25[bgm]"
        )
        labels.append("[bgm]"); n += 1
        print("  BGM OK")
    else:
        print("  BGM not found")

    if os.path.exists(NARR):
        inputs += ["-i", NARR]
        filters.append(
            f"[{n}:a]adelay=8000|8000,"
            f"volume=1.0,"
            f"afade=t=in:st=8:d=0.5[narr]"
        )
        labels.append("[narr]"); n += 1
        print("  Narration OK (8s)")

    if len(labels) == 1:
        # Only game audio, just copy
        shutil.copy(video_in, video_out); return

    af_chain = ";".join(filters) + ";" if filters else ""
    af_chain += "".join(labels) + f"amix=inputs={len(labels)}:duration=first:dropout_transition=2[aout]"

    ff(*inputs,
       "-filter_complex", af_chain,
       "-map","0:v","-map","[aout]",
       "-c:v","copy","-c:a","aac","-b:a","320k",
       video_out, desc="audio_mix (game+bgm+narr)")

# ── Main ──
if __name__ == "__main__":
    print("="*60)
    print("  Arcane Souls Rebirth — Trailer v4")
    print("="*60)

    parts = []
    p = os.path.join(TMP,"01_slate.mp4");  make_slate(p);  parts.append(p)
    p = os.path.join(TMP,"02_title.mp4");  make_title(p);  parts.append(p)
    for i,(st,du,ti,bo) in enumerate(SECS):
        p = os.path.join(TMP,f"0{3+i}_sec{i+1}.mp4")
        make_section(i,st,du,ti,bo,p); parts.append(p)
    p = os.path.join(TMP,"07_end.mp4");    make_end(p);    parts.append(p)

    raw = os.path.join(TMP,"raw.mp4")
    concat_all(parts, raw)
    mix_audio(raw, OUT)

    mb = os.path.getsize(OUT)/1024/1024
    print(f"\n{'='*60}")
    print(f"  DONE -> {OUT}")
    print(f"  {mb:.0f} MB  |  ~186s (3.1 min)")
    print(f"  {'OK <1GB' if mb<1024 else 'WARNING >1GB'}")
    shutil.rmtree(TMP, ignore_errors=True)
    print("  Temp cleaned")
