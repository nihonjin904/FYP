"""
Arcane Souls Rebirth - FYP Final Trailer
Format: 1920x1080 @ 24fps, H.264, AAC 320k
Output: BScCMFYP_Group02_AIGen.mp4
"""

import os
import numpy as np
from moviepy import *
from moviepy.video.fx import FadeIn, FadeOut
from moviepy.audio.fx import AudioFadeIn, AudioFadeOut

# ── Paths ──────────────────────────────────────────────
BASE_DIR     = os.path.dirname(os.path.abspath(__file__))
FOOTAGE_DIR  = os.path.join(BASE_DIR, "Footage")
SLATE_PNG    = os.path.join(FOOTAGE_DIR, "3SecondsOpeningSlateTemplate.png")
DEMO_FILE    = os.path.join(FOOTAGE_DIR, "UnrealEditor_HhKTcv7XnR-00.00.00.000-00.02.54.937-seg1.mp4")
NARR_FILE    = os.path.join(FOOTAGE_DIR, "narration.mp4")
OUTPUT_PATH  = os.path.join(BASE_DIR, "BScCMFYP_Group02_AIGen.mp4")

BGM_CANDIDATES = [
    os.path.join(os.path.dirname(BASE_DIR), "Content", "Audio", "BGM_JapaneseShrineTheme.mp3"),
    os.path.join(BASE_DIR, "BGM_JapaneseShrineTheme.mp3"),
]
BGM_PATH = next((p for p in BGM_CANDIDATES if os.path.exists(p)), "")

# ── Fonts ─────────────────────────────────────────────
FONT_TITLE = r"C:\Windows\Fonts\impact.ttf"
FONT_BODY  = r"C:\Windows\Fonts\arialbd.ttf"
FONT_SMALL = r"C:\Windows\Fonts\arial.ttf"

W, H   = 1920, 1080
RES    = (W, H)
FPS    = 24
BAR_H  = int(H * 0.09)   # cinematic bars

# ── Subtitle sections (start, duration, text) ──────────
# Total demo = 174s → slate(3)+title(5)+4sections(174)+end(4) ≈ 3:06
SUBS = [
    (  0, 40,
     "[ CONCEPT ]\n"
     "Arcane Souls: Rebirth is a third-person action game built in Unreal Engine 5,\n"
     "inspired by FromSoftware's Sekiro: Shadows Die Twice."),
    ( 40, 45,
     "[ AIMS & OBJECTIVE ]\n"
     "Our aim is to recreate skill-based combat featuring precise parrying,\n"
     "posture mechanics, and an intelligent Boss AI."),
    ( 85, 45,
     "[ PRODUCTION PROCESS ]\n"
     "Built entirely in C++ on UE5.5.\n"
     "Systems: Combat Component · AI Behavior Tree · Posture System · Slate UI"),
    (130, 44,
     "[ KEY ACHIEVEMENT ]\n"
     "Posture-break & execution system.\n"
     "Autonomous AI attack loop. Full game loop with difficulty, respawn & pause menu."),
]


# ── Helpers ────────────────────────────────────────────
def cinematic(clip):
    """Color grade + letterbox bars"""
    def grade(f):
        f = f.astype(np.float32) / 255.0
        f = np.power(f, 1.08)
        f[:,:,0] = np.clip(f[:,:,0] * 1.06, 0, 1)
        f[:,:,1] = np.clip(f[:,:,1] * 0.97, 0, 1)
        f[:,:,2] = np.clip(f[:,:,2] * 0.88, 0, 1)
        return (f * 255).astype(np.uint8)

    graded = clip.image_transform(grade)
    top = ColorClip((W, BAR_H), color=(0,0,0)).with_duration(clip.duration)
    bot = ColorClip((W, BAR_H), color=(0,0,0)).with_duration(clip.duration)
    return CompositeVideoClip([
        graded.with_position((0,0)),
        top.with_position((0,0)),
        bot.with_position((0, H - BAR_H)),
    ], size=RES)


def sub_text(text, duration):
    """White subtitle at bottom, gold section label"""
    lines = text.split("\n")
    clips_out = []
    y = H - 160
    for i, line in enumerate(lines):
        color = "#FFD700" if i == 0 else "white"
        sz    = 26 if i == 0 else 28
        font  = FONT_BODY if i == 0 else FONT_SMALL
        tc = TextClip(
            font=font, text=line, font_size=sz, color=color,
            size=(W - 200, None), method="caption", duration=duration
        ).with_position(("center", y))
        y += sz + 10
        clips_out.append(tc)
    if not clips_out:
        return None
    ov = CompositeVideoClip(clips_out, size=RES).with_duration(duration)
    return ov.with_effects([FadeIn(0.4), FadeOut(0.4)])


# ── Build clips ────────────────────────────────────────
def make_slate():
    print("📋 Opening Slate (3s)...")
    return ImageClip(SLATE_PNG).with_duration(3.0).resized(RES)


def make_title():
    print("🎬 Title Card (5s)...")
    dur = 5.0
    bg  = ColorClip(RES, color=(0,0,0)).with_duration(dur)
    t1  = TextClip(font=FONT_TITLE, text="ARCANE SOULS", font_size=110, color="gold",
                   size=RES, method="label", duration=dur).with_position(("center", H//2 - 80))
    t2  = TextClip(font=FONT_BODY, text="REBIRTH", font_size=50, color="#CCCCCC",
                   size=RES, method="label", duration=dur).with_position(("center", H//2 + 55))
    t3  = TextClip(font=FONT_SMALL,
                   text="BScCM Final Year Project  City University of Hong Kong  2025~2026",
                   font_size=22, color="#666666",
                   size=RES, method="label", duration=dur).with_position(("center", H - 70))
    card = CompositeVideoClip([bg, t1, t2, t3], size=RES)
    return card.with_effects([FadeIn(1.0), FadeOut(0.5)])


def make_sections(demo):
    print(f"✂️  Demo 長度: {demo.duration:.1f}s → 分 {len(SUBS)} 節")
    result = []
    for start, dur, text in SUBS:
        end = min(start + dur, demo.duration)
        if start >= demo.duration:
            break
        sec = demo.subclipped(start, end).resized(RES)
        sec = cinematic(sec)
        ov  = sub_text(text, end - start)
        if ov:
            sec = CompositeVideoClip([sec, ov], size=RES)
        sec = sec.with_effects([FadeIn(0.3), FadeOut(0.3)])
        result.append(sec)
        print(f"   ✅ {start}s–{end:.0f}s | {text.split(chr(10))[0]}")
    return result


def make_end():
    print("🎬 End Card (4s)...")
    dur = 4.0
    bg  = ColorClip(RES, color=(0,0,0)).with_duration(dur)
    t   = TextClip(
        font=FONT_SMALL,
        text="Arcane Souls: Rebirth\nBScCM Final Year Project 2025~2026\nCity University of Hong Kong",
        font_size=36, color="#AAAAAA",
        size=RES, method="caption", duration=dur
    ).with_position("center")
    card = CompositeVideoClip([bg, t], size=RES)
    return card.with_effects([FadeIn(1.0), FadeOut(1.5)])


def add_audio(video):
    print("🔊 混合音頻...")
    tracks = []

    # BGM
    if BGM_PATH:
        bgm = AudioFileClip(BGM_PATH)
        if bgm.duration < video.duration:
            loops = int(video.duration / bgm.duration) + 2
            bgm = concatenate_audioclips([bgm] * loops)
        bgm = bgm.subclipped(0, video.duration).with_volume_scaled(0.35)
        bgm = bgm.with_effects([AudioFadeIn(2.0), AudioFadeOut(3.0)])
        tracks.append(bgm)
        print("   ✅ BGM")
    else:
        print("   ⚠️  BGM not found")

    # Narration (start at 8s = after slate+title)
    if os.path.exists(NARR_FILE):
        narr = AudioFileClip(NARR_FILE)
        narr = narr.with_start(8.0).with_volume_scaled(1.0)
        narr = narr.with_effects([AudioFadeIn(0.5), AudioFadeOut(1.0)])
        tracks.append(narr)
        print(f"   ✅ 旁白 ({narr.duration:.1f}s, 從第8秒開始)")

    if not tracks:
        return video
    return video.with_audio(CompositeAudioClip(tracks))


# ── Main ───────────────────────────────────────────────
if __name__ == "__main__":
    print("=" * 60)
    print("🎬  Arcane Souls Rebirth — FYP Final Trailer")
    print("=" * 60)

    slate    = make_slate()
    title    = make_title()
    demo     = VideoFileClip(DEMO_FILE)
    sections = make_sections(demo)
    end      = make_end()

    print("\n🔗 拼接...")
    final = concatenate_videoclips([slate, title] + sections + [end], method="compose")
    final = add_audio(final)

    dur = final.duration
    print(f"\n📊 總長度: {dur:.1f}s ({dur/60:.1f} 分鐘)")
    print(f"🎞️  輸出中... ({OUTPUT_PATH})")

    final.write_videofile(
        OUTPUT_PATH,
        fps=FPS,
        codec="libx264",
        audio_codec="aac",
        audio_bitrate="320k",
        threads=4,
        preset="medium",
        logger="bar",
        ffmpeg_params=["-profile:v", "high", "-level", "5.1", "-pix_fmt", "yuv420p"]
    )

    mb = os.path.getsize(OUTPUT_PATH) / 1024 / 1024
    print(f"\n✅ 完成！")
    print(f"📁 {OUTPUT_PATH}")
    print(f"📊 {mb:.0f} MB | {dur:.0f}s ({dur/60:.1f} min)")
    print(f"{'✅ <1GB OK' if mb < 1024 else '⚠️ 超過1GB！'}")
    print(f"\n交件名稱: BScCMFYP_Group02_AIGen.mp4")
