from pptx import Presentation
from pptx.util import Inches, Pt, Emu
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.util import Inches, Pt
from pptx.oxml.ns import qn
from lxml import etree
import copy

# ─── Eastern Anime Colour Palette ──────────────────────────────────
BG_DARK    = RGBColor(0x0D, 0x0D, 0x1F)   # Deep navy (title slide overlay)
DARK_NAVY  = RGBColor(0x1C, 0x2B, 0x5E)   # Main body text (on light bg)
ACCENT1    = RGBColor(0x38, 0x6B, 0xC0)   # Eastern azure blue
ACCENT2    = RGBColor(0xD4, 0x6A, 0x28)   # Warm amber / sunset orange
ACCENT3    = RGBColor(0xC0, 0x55, 0x75)   # Sakura rose
WHITE      = RGBColor(0xFF, 0xFF, 0xFF)   # For title slides (dark bg)
LIGHTGRAY  = RGBColor(0x44, 0x55, 0x77)   # Readable slate (on light bg)
RED_PERIL  = RGBColor(0xC0, 0x20, 0x20)   # Deep perilous red
GREEN_OK   = RGBColor(0x16, 0x7A, 0x3C)   # Forest green
# Card / panel fills (frosted-glass, light)
CARD_MAIN  = RGBColor(0xDF, 0xEC, 0xF8)   # Light azure card
CARD_WARM  = RGBColor(0xF8, 0xED, 0xDF)   # Warm amber card
CARD_PERIL = RGBColor(0xF8, 0xDF, 0xDF)   # Light red card
CARD_OK    = RGBColor(0xDF, 0xF5, 0xE8)   # Light green card
ROW_ALT1   = RGBColor(0xE5, 0xF1, 0xFF)   # Table row alternating 1
ROW_ALT2   = RGBColor(0xF2, 0xF7, 0xFF)   # Table row alternating 2

W = Inches(13.33)   # Widescreen 16:9 width
H = Inches(7.5)     # Widescreen 16:9 height

prs = Presentation()
prs.slide_width  = W
prs.slide_height = H

BLANK = prs.slide_layouts[6]   # completely blank layout

# ─── BACKGROUND IMAGE PATHS ──────────────────────────────────────
import os
_here = os.path.dirname(os.path.abspath(__file__))
BG_TITLE   = os.path.join(_here, "bg_title.png")
BG_CONTENT = os.path.join(_here, "bg_content.png")

# ─── HELPER FUNCTIONS ────────────────────────────────────────────

def add_rect(slide, l, t, w, h, fill=None, line=None, alpha=None):
    shape = slide.shapes.add_shape(1, l, t, w, h)
    if fill:
        shape.fill.solid()
        shape.fill.fore_color.rgb = fill
    else:
        shape.fill.background()
    if line:
        shape.line.color.rgb = line
        shape.line.width = Pt(1.5)
    else:
        shape.line.fill.background()
    return shape

def add_text(slide, text, l, t, w, h,
             font_size=24, bold=False, italic=False,
             color=WHITE, align=PP_ALIGN.LEFT, word_wrap=True):
    txBox = slide.shapes.add_textbox(l, t, w, h)
    tf = txBox.text_frame
    tf.word_wrap = word_wrap
    p = tf.paragraphs[0]
    p.alignment = align
    run = p.add_run()
    run.text = text
    run.font.size = Pt(font_size)
    run.font.bold = bold
    run.font.italic = italic
    run.font.color.rgb = color
    run.font.name = "Calibri"
    return txBox

def set_bg_image(slide, img_path):
    """Set a full-bleed image as slide background."""
    slide.shapes.add_picture(img_path, 0, 0, W, H)
    # Move the picture to the back using XML ordering
    sp_tree = slide.shapes._spTree
    pic_elem = sp_tree[-1]          # just added picture
    sp_tree.remove(pic_elem)
    sp_tree.insert(2, pic_elem)     # index 2 = behind all shapes

def apply_text_shadow(txBox, blur=63500, dist=50800, alpha=80000):
    """Add dark outer shadow to text for readability on busy backgrounds.
    No overlay needed — shadow outlined around each character."""
    tf = txBox.text_frame
    for para in tf.paragraphs:
        for run in para.runs:
            r_elem = run._r
            rPr = r_elem.find(qn('a:rPr'))
            if rPr is None:
                rPr = etree.SubElement(r_elem, qn('a:rPr'))
                r_elem.insert(0, rPr)
            effectLst = rPr.find(qn('a:effectLst'))
            if effectLst is None:
                effectLst = etree.SubElement(rPr, qn('a:effectLst'))
            outerShdw = etree.SubElement(effectLst, qn('a:outerShdw'))
            outerShdw.set('blurRad', str(blur))
            outerShdw.set('dist',    str(dist))
            outerShdw.set('dir',     '2700000')
            outerShdw.set('algn',    'ctr')
            outerShdw.set('rotWithShape', '0')
            srgbClr = etree.SubElement(outerShdw, qn('a:srgbClr'))
            srgbClr.set('val', '000000')
            alpha_elem = etree.SubElement(srgbClr, qn('a:alpha'))
            alpha_elem.set('val', str(alpha))

def add_slide(title_slide=False):
    s = prs.slides.add_slide(BLANK)
    img = BG_TITLE if title_slide else BG_CONTENT
    set_bg_image(s, img)
    return s

def accent_bar(slide, color=ACCENT1, height=Pt(6)):
    add_rect(slide, 0, 0, W, height, fill=color)

def slide_title(slide, title, subtitle=None):
    # Frosted azure header band — Eastern Anime style
    add_rect(slide, 0, Inches(0.05), W, Inches(0.82),
             fill=RGBColor(0xB5, 0xD3, 0xEE))
    add_text(slide, title,
             Inches(0.5), Inches(0.1), Inches(12.3), Inches(0.82),
             font_size=30, bold=True, color=DARK_NAVY)
    if subtitle:
        add_text(slide, subtitle,
                 Inches(0.5), Inches(0.88), Inches(12.3), Inches(0.45),
                 font_size=14, italic=True, color=LIGHTGRAY)
    # bottom accent line
    add_rect(slide, 0, H - Pt(4), W, Pt(4), fill=ACCENT1)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 1 — TITLE
# ═══════════════════════════════════════════════════════════════════
s = add_slide(title_slide=True)
# Left accent strip
add_rect(s, 0, 0, Inches(0.12), H, fill=ACCENT1)

# Frosted white card behind meta info (bottom area of slide)
add_rect(s, Inches(0.4), Inches(4.72), Inches(12.5), Inches(2.55),
         fill=RGBColor(0xF5, 0xF8, 0xFF), line=ACCENT1)

# Big title — DARK NAVY (light background needs dark text)
add_text(s, "Arcane Souls: Rebirth",
         Inches(0.5), Inches(1.45), Inches(12.3), Inches(1.5),
         font_size=54, bold=True, color=DARK_NAVY, align=PP_ALIGN.CENTER)

# Subtitle
add_text(s, "Sekiro-Inspired Posture Combat in Unreal Engine 5",
         Inches(0.5), Inches(2.95), Inches(12.3), Inches(0.7),
         font_size=22, italic=True, color=ACCENT1, align=PP_ALIGN.CENTER)

# HR divider
add_rect(s, Inches(2.0), Inches(3.78), Inches(9.3), Pt(2), fill=ACCENT2)

# Meta inside frosted panel
add_text(s,
         "Group: AIGEN  |  Lee Chun Kit & Lam Chi Him\n"
         "Advisor: Jussi Pekka Holopainen  |  SM4712B (LA2)\n"
         "City University of Hong Kong — School of Creative Media",
         Inches(0.6), Inches(4.82), Inches(12.1), Inches(1.5),
         font_size=15, color=DARK_NAVY, align=PP_ALIGN.CENTER)

# Course tag
add_text(s, "Phase II Final Presentation  |  April 2026",
         Inches(0.5), Inches(6.3), Inches(12.3), Inches(0.5),
         font_size=13, color=ACCENT2, align=PP_ALIGN.CENTER)

# AI-generated disclaimer (bottom right)
add_text(s, "* Background artwork: AI-generated",
         Inches(8.2), H - Inches(0.42), Inches(4.8), Inches(0.4),
         font_size=9, italic=True,
         color=RGBColor(0x88, 0x88, 0x88), align=PP_ALIGN.RIGHT)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 2 — AGENDA
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Agenda")
items = [
    "01  Introduction & Problem Statement",
    "02  Objectives",
    "03  What We Built (Deliverables)",
    "04  System Architecture",
    "05  Boss AI & Perilous Attack System",
    "06  VRM Character Pipeline",
    "07  Results & Evaluation",
    "08  Challenges & Solutions",
    "09  Conclusion",
]
for i, item in enumerate(items):
    y = Inches(1.35) + i * Inches(0.6)
    c = ACCENT1 if i % 2 == 0 else DARK_NAVY
    add_text(s, item, Inches(1.2), y, Inches(10), Inches(0.55),
             font_size=18, color=c)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 3 — INTRODUCTION
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Introduction", "Why does this game matter?")

add_rect(s, Inches(0.5), Inches(1.4), Inches(5.8), Inches(4.5),
         fill=CARD_MAIN, line=ACCENT1)
add_text(s, "The Problem",
         Inches(0.7), Inches(1.5), Inches(5.4), Inches(0.5),
         font_size=18, bold=True, color=ACCENT1)
add_text(s,
         "• Souls-like genre is growing but few combine:\n"
         "  anime visuals + deep posture combat\n\n"
         "• Small indie teams lack scalable pipelines\n  for VRM anime characters in UE5\n\n"
         "• No existing project merges Sekiro-style\n  posture mechanics with stylized UE5 workflow",
         Inches(0.7), Inches(2.1), Inches(5.4), Inches(3.5),
         font_size=16, color=LIGHTGRAY)

add_rect(s, Inches(7.0), Inches(1.4), Inches(5.8), Inches(4.5),
         fill=CARD_WARM, line=ACCENT2)
add_text(s, "Our Solution",
         Inches(7.2), Inches(1.5), Inches(5.4), Inches(0.5),
         font_size=18, bold=True, color=ACCENT2)
add_text(s,
         "• Arcane Souls: Rebirth\n  — Sekiro-inspired 3D action game\n\n"
         "• Built in Unreal Engine 5.5 with C++\n\n"
         "• Touhou Project anime characters\n  (Reimu, Patchouli) via VRM4U plugin\n\n"
         "• Full posture / parry / perilous attack loop",
         Inches(7.2), Inches(2.1), Inches(5.4), Inches(3.5),
         font_size=16, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 4 — OBJECTIVES
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Objectives")

cols = [
    ("🎨 Creative", ACCENT1, CARD_MAIN, [
        "Cohesive anime aesthetic",
        "Cinematic stylized visuals",
        "Touhou character designs",
    ]),
    ("⚙️ Technical", ACCENT2, CARD_WARM, [
        "4-frame (67ms) parry window",
        "Lumen global illumination",
        "Behavior Tree boss AI",
        "Niagara VFX for combat",
    ]),
    ("🎯 Performance", GREEN_OK, CARD_OK, [
        "60 FPS @ 1080p (GTX 1660S)",
        "Input latency < 100 ms",
        "p95 frame time < 20 ms",
    ]),
]
for ci, (title, color, card, bullets) in enumerate(cols):
    x = Inches(0.4) + ci * Inches(4.3)
    add_rect(s, x, Inches(1.3), Inches(4.1), Inches(5.5),
             fill=card, line=color)
    add_text(s, title, x + Inches(0.15), Inches(1.45), Inches(3.8), Inches(0.55),
             font_size=20, bold=True, color=color)
    for bi, b in enumerate(bullets):
        add_text(s, "▶  " + b,
                 x + Inches(0.15), Inches(2.2) + bi * Inches(0.85),
                 Inches(3.8), Inches(0.8),
                 font_size=16, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 5 — DELIVERABLES
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "What We Built", "Phase II Deliverables — all ✅ delivered")

deliverables = [
    ("Playable Vertical Slice", "Full game: player + boss + arena environment"),
    ("Player Character (Reimu)", "VRM model, IK-retargeted combat animations"),
    ("Boss (BP_SekiroEnemy)",    "4 standard attacks + 2 perilous attacks"),
    ("Combat System (C++)",      "Posture, deflection, deathblow execution"),
    ("Boss AI",                  "Behavior Tree + randomized attack selection"),
    ("Boss Arena Level",         "Shrine-themed with Lumen lighting"),
    ("HUD/UI System",            "Health, posture, lock-on, perilous warning"),
    ("Source Code",              "C++ + Blueprints + Python automation"),
]
for i, (name, desc) in enumerate(deliverables):
    row = i // 2
    col = i % 2
    x = Inches(0.4) + col * Inches(6.5)
    y = Inches(1.3) + row * Inches(1.3)
    add_rect(s, x, y, Inches(6.2), Inches(1.1),
             fill=CARD_MAIN, line=ACCENT1)
    add_text(s, "✅  " + name, x + Inches(0.15), y + Inches(0.05),
             Inches(5.9), Inches(0.45),
             font_size=17, bold=True, color=ACCENT2)
    add_text(s, desc, x + Inches(0.35), y + Inches(0.55),
             Inches(5.7), Inches(0.45),
             font_size=14, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 6 — SYSTEM ARCHITECTURE
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "System Architecture", "Component-based C++ design shared by player & boss")

add_text(s, "ASekiroCharacter  (C++ Base Class)",
         Inches(4.5), Inches(1.3), Inches(4.8), Inches(0.5),
         font_size=18, bold=True, color=ACCENT2, align=PP_ALIGN.CENTER)
add_rect(s, Inches(4.5), Inches(1.25), Inches(4.8), Inches(0.5),
         line=ACCENT2)

components = [
    ("USekiroPostureComponent",        "Posture accumulation & break"),
    ("USekiroDeflectComponent",        "Parry window timing (4 frames)"),
    ("USekiroCombatComponent",         "Attack combos & hit detection"),
    ("USekiroAttributeComponent",      "Health, damage, death"),
    ("USekiroEnemyAttributeComponent", "Boss: montage pools & perilous"),
    ("Behavior Tree + AI Controller",  "Boss decision-making"),
]
for i, (comp, desc) in enumerate(components):
    y = Inches(2.0) + i * Inches(0.82)
    card = CARD_MAIN if i < 4 else CARD_WARM
    color = ACCENT1 if i < 4 else ACCENT2
    add_rect(s, Inches(1.0), y, Inches(11.3), Inches(0.72),
             fill=card, line=color)
    add_text(s, comp, Inches(1.2), y + Inches(0.08), Inches(5.5), Inches(0.35),
             font_size=15, bold=True, color=color)
    add_text(s, "→  " + desc, Inches(6.8), y + Inches(0.08), Inches(5.2), Inches(0.35),
             font_size=14, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 7 — BOSS AI & PERILOUS ATTACK
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Boss AI & Perilous Attack System")

# Left: BT structure
add_rect(s, Inches(0.4), Inches(1.3), Inches(6.0), Inches(5.5),
         fill=CARD_MAIN, line=ACCENT1)
add_text(s, "Behavior Tree Structure",
         Inches(0.6), Inches(1.35), Inches(5.7), Inches(0.45),
         font_size=17, bold=True, color=ACCENT1)
bt_lines = [
    "Root (Selector)",
    "  ├── [IsAlerted=true]",
    "  │     └── Sequence: Combat",
    "  │           ├── Move To Player",
    "  │           └── Select Attack",
    "  │                 ├── 75%: Standard Combo",
    "  │                 │     (4 montages, random)",
    "  │                 └── 25%: Perilous Attack",
    "  │                       (2 montages, random)",
    "  └── Sequence: Idle/Patrol",
    "        ├── Wait (2–4 sec)",
    "        └── Random move",
]
for i, line in enumerate(bt_lines):
    add_text(s, line,
             Inches(0.6), Inches(1.9) + i * Inches(0.33),
             Inches(5.6), Inches(0.33),
             font_size=12,
             color=RED_PERIL if "Perilous" in line else (ACCENT2 if "%" in line else LIGHTGRAY))

# Right: Perilous detail
add_rect(s, Inches(7.0), Inches(1.3), Inches(5.8), Inches(2.5),
         fill=CARD_PERIL, line=RED_PERIL)
add_text(s, "⚠  Perilous Attack: Unblockable",
         Inches(7.2), Inches(1.38), Inches(5.4), Inches(0.5),
         font_size=17, bold=True, color=RED_PERIL)
add_text(s,
         "• 「危」kanji warning displayed on screen\n"
         "• Cannot be blocked — must DODGE\n"
         "• Two types: Great Sword Slash + Upward Thrust\n"
         "• Probability: PerilousChance (default 25%)",
         Inches(7.2), Inches(1.95), Inches(5.4), Inches(1.7),
         font_size=14, color=LIGHTGRAY)

add_rect(s, Inches(7.0), Inches(4.0), Inches(5.8), Inches(2.7),
         fill=CARD_WARM, line=ACCENT2)
add_text(s, "Standard Attacks",
         Inches(7.2), Inches(4.05), Inches(5.4), Inches(0.45),
         font_size=17, bold=True, color=ACCENT2)
attacks = [
    ("Heavy Slam",    "High dmg, long windup, blockable"),
    ("Quick Sweep",   "Medium dmg, short windup, dodge"),
    ("Forward Thrust","Medium dmg, parryable"),
    ("Two-Hit Combo", "Delayed second hit"),
]
for i, (a, d) in enumerate(attacks):
    add_text(s, f"▸ {a} — {d}",
             Inches(7.2), Inches(4.6) + i * Inches(0.5),
             Inches(5.4), Inches(0.45),
             font_size=13, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 8 — VRM PIPELINE
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "VRM Anime Character Pipeline", "How we put Reimu and Patchouli into UE5")

steps = [
    ("VRM File",      "Touhou Project\nReimu / Patchouli\n(.vrm model)",    ACCENT1,  CARD_MAIN),
    ("VRM4U Plugin",  "Import into UE5\nAuto-generates\nSkeleton + Materials", ACCENT2, CARD_WARM),
    ("IK Retargeter", "RTG__魔_博麗_霊夢\nMannequin → VRM\nbone mapping",  ACCENT1,  CARD_MAIN),
    ("AnimBlueprint", "ABP_reimu_C\nRealtime animation\ntransfer",          ACCENT2,  CARD_WARM),
    ("In-Game",       "Full combat anims\non anime character\nat 60 FPS",   GREEN_OK, CARD_OK),
]

arrow_y = Inches(3.5)
for i, (label, desc, color, card) in enumerate(steps):
    x = Inches(0.4) + i * Inches(2.55)
    add_rect(s, x, Inches(1.4), Inches(2.3), Inches(4.0),
             fill=card, line=color)
    add_text(s, label, x + Inches(0.1), Inches(1.5), Inches(2.1), Inches(0.55),
             font_size=16, bold=True, color=color, align=PP_ALIGN.CENTER)
    add_text(s, desc, x + Inches(0.1), Inches(2.2), Inches(2.1), Inches(2.5),
             font_size=13, color=LIGHTGRAY, align=PP_ALIGN.CENTER)
    if i < len(steps) - 1:
        add_text(s, "→", x + Inches(2.35), arrow_y, Inches(0.35), Inches(0.5),
                 font_size=24, bold=True, color=ACCENT1, align=PP_ALIGN.CENTER)

# Challenges box
add_rect(s, Inches(0.4), Inches(5.6), Inches(12.4), Inches(1.6),
         fill=CARD_WARM, line=ACCENT2)
add_text(s, "Key Challenges:",
         Inches(0.6), Inches(5.65), Inches(3), Inches(0.4),
         font_size=15, bold=True, color=ACCENT2)
add_text(s,
         "Rest pose mismatch (VRM A-pose vs Mannequin T-pose) → Fixed Retarget Pose in IK Retargeter\n"
         "Weapon attached to Mannequin bone → Runtime re-attachment to VRM 「右手首」bone at BeginPlay",
         Inches(0.6), Inches(6.15), Inches(12.2), Inches(0.9),
         font_size=13, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 9 — COMBAT SYSTEM
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Combat System Deep Dive", "Posture · Deflect · Deathblow")

panels = [
    ("🛡  PARRY / DEFLECT", ACCENT1, CARD_MAIN, [
        "Fixed 4-frame window (≈67 ms @ 60 FPS)",
        "Perfect Parry → posture dmg to ATTACKER",
        "Late block → posture dmg to DEFENDER",
        "No input → direct health damage",
    ]),
    ("💀  POSTURE BREAK", ACCENT2, CARD_WARM, [
        "Posture accumulates when blocking attacks",
        "Posture breaks → stagger state",
        "Staggered boss: DEATHBLOW prompt appears",
        "Posture recovers slowly when not hit",
    ]),
    ("⚔  COMBAT LOOP", RED_PERIL, CARD_PERIL, [
        "Engage → Attack pattern (BT selects)",
        "Player parries → builds boss posture",
        "Posture break → execute deathblow",
        "Repeat (boss has multiple HP bars)",
    ]),
]
for ci, (title, color, card, bullets) in enumerate(panels):
    x = Inches(0.35) + ci * Inches(4.35)
    add_rect(s, x, Inches(1.3), Inches(4.1), Inches(5.5),
             fill=card, line=color)
    add_text(s, title, x + Inches(0.15), Inches(1.4), Inches(3.9), Inches(0.55),
             font_size=17, bold=True, color=color, align=PP_ALIGN.CENTER)
    for i, b in enumerate(bullets):
        add_text(s, "• " + b,
                 x + Inches(0.2), Inches(2.1) + i * Inches(0.9),
                 Inches(3.7), Inches(0.85),
                 font_size=14, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 10 — RESULTS
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Results & Evaluation")

# Performance table
add_text(s, "Performance Metrics",
         Inches(0.5), Inches(1.3), Inches(5.8), Inches(0.45),
         font_size=18, bold=True, color=ACCENT2)
perf = [
    ("FPS @ 1080p (GTX 1660S)", "≥ 60 FPS", "✅ Stable 60 FPS"),
    ("p95 Frame Time",           "< 20 ms",  "✅ ~17 ms"),
    ("Input Latency (parry)",    "< 100 ms", "✅ ~67 ms (4 frames)"),
    ("Asset Load Time",          "< 5 sec",  "✅ ~3 sec"),
]
headers = ["Metric", "Target", "Achieved"]
col_w = [Inches(2.5), Inches(1.1), Inches(2.0)]
col_x = [Inches(0.5), Inches(3.1), Inches(4.3)]
for ci, (h, cw, cx) in enumerate(zip(headers, col_w, col_x)):
    add_rect(s, cx, Inches(1.75), cw, Inches(0.38), fill=ACCENT1)
    add_text(s, h, cx + Inches(0.05), Inches(1.78), cw - Inches(0.1), Inches(0.35),
             font_size=13, bold=True, color=WHITE)
for ri, row in enumerate(perf):
    row_y = Inches(2.15) + ri * Inches(0.5)
    bg = ROW_ALT1 if ri % 2 == 0 else ROW_ALT2
    for ci, (val, cw, cx) in enumerate(zip(row, col_w, col_x)):
        add_rect(s, cx, row_y, cw, Inches(0.48), fill=bg)
        c = GREEN_OK if "✅" in val else DARK_NAVY
        add_text(s, val, cx + Inches(0.05), row_y + Inches(0.07), cw - Inches(0.1), Inches(0.38),
                 font_size=12, color=c)

# Playtest table
add_text(s, "Playtest Results  (n=5)",
         Inches(7.0), Inches(1.3), Inches(5.7), Inches(0.45),
         font_size=18, bold=True, color=ACCENT2)
play = [
    ("Souls beginner", "28%", "8",  "Warning clear"),
    ("Souls beginner", "35%", "6",  "Blocking responsive"),
    ("Casual gamer",   "22%", "12", "Visual feedback helps"),
    ("Sekiro veteran", "58%", "2",  "Parry feels fair"),
    ("Sekiro veteran", "65%", "1",  "Perilous adds variety"),
]
ph = ["Level", "Parry%", "Deaths", "Feedback"]
px = [Inches(7.0), Inches(8.6), Inches(9.5), Inches(10.3)]
pw = [Inches(1.5), Inches(0.8), Inches(0.75), Inches(2.8)]
for ci, (h, cx, cw) in enumerate(zip(ph, px, pw)):
    add_rect(s, cx, Inches(1.75), cw, Inches(0.38), fill=ACCENT1)
    add_text(s, h, cx + Inches(0.05), Inches(1.78), cw - Inches(0.1), Inches(0.35),
             font_size=12, bold=True, color=WHITE)
for ri, row in enumerate(play):
    row_y = Inches(2.15) + ri * Inches(0.5)
    bg = ROW_ALT1 if ri % 2 == 0 else ROW_ALT2
    for ci, (val, cx, cw) in enumerate(zip(row, px, pw)):
        add_rect(s, cx, row_y, cw, Inches(0.48), fill=bg)
        add_text(s, val, cx + Inches(0.05), row_y + Inches(0.07), cw - Inches(0.1), Inches(0.38),
                 font_size=11, color=DARK_NAVY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 11 — CHALLENGES
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Challenges & Solutions")

challenges = [
    ("VRM Forward Lean",      "REST POSE",       "Rest pose mismatch: VRM A-pose vs UE5 T-pose",
     "Edited Retarget Pose in IK Retargeter to correct spine/pelvis rotations"),
    ("Weapon Wrong Position", "BONE MAPPING",    "Weapon on Mannequin hand_r, not VRM hand bone",
     "Runtime re-attachment to 「右手首」 (right hand) VRM bone at BeginPlay"),
    ("Boss T-Pose Attack",    "CDO vs INSTANCE", "AnimBP class assignment lost on Blueprint instance",
     "Re-assigned ABP_SekiroEnemy_New and re-spawned actor to inherit updated CDO"),
    ("Perilous Not Firing",   "CDO MISMATCH",    "CDO vs Instance data mismatch after Blueprint edit",
     "Deleted and re-spawned boss to inherit correct default values"),
    ("FBX Import Hijacked",   "PLUGIN CONFLICT", "VRM4U intercepted ALL FBX imports globally",
     "Temporarily disabled VRM4U plugin for animation imports only"),
]
for i, (prob, tag, root, fix) in enumerate(challenges):
    y = Inches(1.3) + i * Inches(1.18)
    bg     = CARD_MAIN if i % 2 == 0 else CARD_WARM
    border = ACCENT2    if i % 2 == 0 else ACCENT1
    hcolor = ACCENT2    if i % 2 == 0 else ACCENT1
    add_rect(s, Inches(0.4), y, Inches(12.4), Inches(1.08),
             fill=bg, line=border)
    add_text(s, prob,
             Inches(0.6), y + Inches(0.06), Inches(2.5), Inches(0.4),
             font_size=14, bold=True, color=hcolor)
    add_rect(s, Inches(3.1), y + Inches(0.08), Inches(1.5), Inches(0.32),
             fill=ACCENT1)
    add_text(s, tag, Inches(3.15), y + Inches(0.1), Inches(1.4), Inches(0.28),
             font_size=10, bold=True, color=WHITE)
    add_text(s, "Problem: " + root,
             Inches(0.6), y + Inches(0.5), Inches(5.5), Inches(0.42),
             font_size=12, color=LIGHTGRAY)
    add_text(s, "✅ Fix: " + fix,
             Inches(6.5), y + Inches(0.06), Inches(6.3), Inches(0.9),
             font_size=13, color=GREEN_OK)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 12 — DEMO PLACEHOLDER
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Live Demo / Video")

add_rect(s, Inches(2.5), Inches(1.5), Inches(8.3), Inches(4.5),
         fill=CARD_MAIN, line=ACCENT1)
add_text(s, "▶   GAME DEMO",
         Inches(2.5), Inches(3.2), Inches(8.3), Inches(1.0),
         font_size=42, bold=True, color=ACCENT1, align=PP_ALIGN.CENTER)

demo_items = [
    "① Player movement + lock-on system",
    "② Boss attack patterns (4 standard)",
    "③ Parry → Posture break → Deathblow",
    "④ Perilous attack: 「危」warning appears",
    "⑤ Dodge dodges the unblockable hit",
]
for i, item in enumerate(demo_items):
    x = Inches(0.5) if i < 3 else Inches(7.0)
    y = Inches(6.0) + (i if i < 3 else i - 3) * Inches(0.4)
    add_text(s, item, x, y, Inches(5.5), Inches(0.38),
             font_size=13, color=DARK_NAVY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 13 — CONCLUSION
# ═══════════════════════════════════════════════════════════════════
s = add_slide()
slide_title(s, "Conclusion & Lessons Learned")

add_rect(s, Inches(0.4), Inches(1.3), Inches(6.0), Inches(2.8),
         fill=CARD_OK, line=GREEN_OK)
add_text(s, "✅  What Went Well",
         Inches(0.6), Inches(1.38), Inches(5.7), Inches(0.45),
         font_size=17, bold=True, color=GREEN_OK)
good = [
    "Modular C++ component architecture — reusable between player & boss",
    "VRM4U → IK Retargeter pipeline — reproducible for future projects",
    "Behavior Tree + probability attack — easy designer tuning",
    "60 FPS stable — met all performance targets",
]
for i, g in enumerate(good):
    add_text(s, "• " + g,
             Inches(0.6), Inches(1.9) + i * Inches(0.53),
             Inches(5.7), Inches(0.5),
             font_size=13, color=LIGHTGRAY)

add_rect(s, Inches(6.8), Inches(1.3), Inches(6.0), Inches(2.8),
         fill=CARD_WARM, line=ACCENT2)
add_text(s, "⚠  What Could Be Better",
         Inches(7.0), Inches(1.38), Inches(5.7), Inches(0.45),
         font_size=17, bold=True, color=ACCENT2)
bad = [
    "IK retargeting quality — minor artefacts in extreme poses",
    "Content scope — planned 2 levels, delivered 1 boss arena",
    "No automated unit tests for combat calculations",
    "Stricter branch/PR workflow to avoid merge conflicts",
]
for i, b in enumerate(bad):
    add_text(s, "• " + b,
             Inches(7.0), Inches(1.9) + i * Inches(0.53),
             Inches(5.7), Inches(0.5),
             font_size=13, color=LIGHTGRAY)

add_rect(s, Inches(0.4), Inches(4.3), Inches(12.4), Inches(2.7),
         fill=CARD_MAIN, line=ACCENT1)
add_text(s, "🔑  Key Contributions",
         Inches(0.6), Inches(4.38), Inches(10), Inches(0.45),
         font_size=17, bold=True, color=ACCENT1)
contribs = [
    "Reproducible VRM → UE5 character integration pipeline",
    "Modular C++ posture-based combat architecture applicable to other projects",
    "Practical patterns for combining anime visuals with responsive action combat in UE5",
]
for i, c in enumerate(contribs):
    add_text(s, f"{i+1}.  " + c,
             Inches(0.6), Inches(4.95) + i * Inches(0.6),
             Inches(12.0), Inches(0.55),
             font_size=14, color=LIGHTGRAY)

# ═══════════════════════════════════════════════════════════════════
# SLIDE 14 — THANK YOU / REFERENCES / AI ETHICS
# ═══════════════════════════════════════════════════════════════════
s = add_slide(title_slide=False)   # ⭐ content background — same as slides 2-13
slide_title(s, "Thank You  ·  References & AI Ethics")

# Q&A invite
add_text(s, "Questions & Discussion Welcome  🙏",
         Inches(0.4), Inches(0.95), Inches(12.5), Inches(0.42),
         font_size=15, italic=True, color=ACCENT1, align=PP_ALIGN.CENTER)

# Members row
add_rect(s, Inches(0.4), Inches(1.43), Inches(12.5), Inches(0.38),
         fill=RGBColor(0xE8, 0xF2, 0xFA))
add_text(s,
         "Lee Chun Kit (57306141)    ·    Lam Chi Him (57185861)    ·    SM4712B  |  AIGEN  |  April 2026",
         Inches(0.4), Inches(1.45), Inches(12.5), Inches(0.35),
         font_size=11, color=DARK_NAVY, align=PP_ALIGN.CENTER)

# ---- REFERENCES ----
add_text(s, "📚  References",
         Inches(0.4), Inches(1.88), Inches(7), Inches(0.38),
         font_size=13, bold=True, color=ACCENT1)
refs = [
    "[1]  Epic Games. (2024). Unreal Engine 5 Documentation. docs.unrealengine.com",
    "[2]  FromSoftware. (2019). Sekiro: Shadows Die Twice. Activision Blizzard.",
    "[3]  Team Shanghai Alice (ZUN). (1996–2024). Touhou Project. Self-published.",
    "[4]  ruyo. (2024). VRM4U: VRM importer plugin for UE5. github.com/ruyo/VRM4U",
    "[5]  Gregory, J. (2018). Game Engine Architecture (3rd ed.). CRC Press.",
]
for i, ref in enumerate(refs):
    add_text(s, ref,
             Inches(0.5), Inches(2.3) + i * Inches(0.33),
             Inches(12.3), Inches(0.33),
             font_size=10, color=DARK_NAVY)

# ---- AI ETHICS PANEL ----
add_rect(s, Inches(0.4), Inches(4.02), Inches(12.5), Inches(2.75),
         fill=CARD_WARM, line=ACCENT2)
add_text(s, "⚙  AI Ethical Usage Statement",
         Inches(0.6), Inches(4.1), Inches(10), Inches(0.42),
         font_size=14, bold=True, color=ACCENT2)
ai_points = [
    "• AI was used as a learning consultant only — never as a code generator or blueprint drafter.",
    "• Blueprint nodes:  We never let AI drag or wire connections. We asked AI to explain the concept,",
    "   then implemented every node and connection ourselves inside the UE5 editor.",
    "• Hard bugs:  e.g. skeletal mesh IK mismatch & AnimNotify timing — AI explained the root cause;",
    "   all fixes were designed and applied manually in C++ and the UE5 editor by team members.",
    "• All source code, asset pipelines, and design decisions are fully original team work.",
]
for i, pt in enumerate(ai_points):
    add_text(s, pt,
             Inches(0.6), Inches(4.6) + i * Inches(0.35),
             Inches(12.1), Inches(0.35),
             font_size=10.5, color=DARK_NAVY)

# Blog links
add_text(s,
         "leechunkit01255210.wixsite.com/e-portfolio    |    nihonjin864.wixsite.com/lamchihim",
         Inches(1.5), H - Inches(0.4), Inches(10), Inches(0.38),
         font_size=11, color=ACCENT1, align=PP_ALIGN.CENTER)

# ─── SAVE ─────────────────────────────────────────────────────────
output = r"c:\Users\Kelvin Lam\Documents\GitHub\FYP\FYP\Documents\canvasdoucment\Arcane_Souls_Rebirth_Presentation.pptx"
prs.save(output)
print(f"✅ PPT saved: {output}")
print(f"   Slides: {len(prs.slides)}")
