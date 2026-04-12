#!/usr/bin/env python3
"""
Build the complete Phase II Final Report DOCX with embedded cover page.
Output: canvasdoucment/Phase_II_Final_Report_FINAL.docx
"""

import os
import re
from docx import Document
from docx.shared import Pt, Inches, RGBColor, Cm
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml.ns import qn

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
CANVAS_DIR = os.path.join(SCRIPT_DIR, 'canvasdoucment')
MD_PATH = os.path.join(CANVAS_DIR, 'Phase_II_Final_Report.md')
OUTPUT_PATH = os.path.join(CANVAS_DIR, 'Phase_II_Final_Report_FINAL.docx')


def clean_md(text):
    text = re.sub(r'\*\*(.*?)\*\*', r'\1', text)
    text = re.sub(r'\*(.*?)\*', r'\1', text)
    text = re.sub(r'`(.*?)`', r'\1', text)
    return text.strip()


def add_centered(doc, text, size=12, bold=False, space_after=6):
    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = p.add_run(text)
    run.font.name = 'Times New Roman'
    run.font.size = Pt(size)
    run.bold = bold
    p.paragraph_format.space_after = Pt(space_after)
    p.paragraph_format.space_before = Pt(0)
    return p


def add_cover_page(doc):
    """Build the official Phase II cover page matching the CityU template."""

    # Spacing at top
    for _ in range(3):
        p = doc.add_paragraph()
        p.paragraph_format.space_after = Pt(6)

    # University name
    add_centered(doc, "City University of Hong Kong", size=16, bold=True, space_after=4)
    add_centered(doc, "School of Creative Media: BScCM", size=14, bold=False, space_after=4)
    add_centered(doc, "Department of Computer Science", size=14, bold=False, space_after=20)

    # Report title
    add_centered(doc, "BScCM Final Year Project 2025–2026", size=14, bold=True, space_after=6)
    add_centered(doc, "Phase II Final Report of SM4701 (LA2)", size=14, bold=True, space_after=6)
    add_centered(doc, "Graduation Thesis/Project", size=12, bold=False, space_after=6)
    add_centered(doc, "(BScCM FYP Implementation – Phase II)", size=12, bold=False, space_after=30)

    # Cover page
    add_centered(doc, "< Phase II Final Report Cover >", size=11, bold=False, space_after=6)
    add_centered(doc, "< April 2026 >", size=11, bold=False, space_after=30)

    # Project info table
    info_data = [
        ("Project Title:", "Arcane Souls: Rebirth"),
        ("Group Name:", "AIGEN"),
        ("Group No:", "AIGEN"),
        ("Programme Code:", "SM4712B (LA2)"),
        ("", ""),
        ("Student Name / EID:", "Lee Chun Kit / cklee96 (Group Leader)"),
        ("Student Name / EID:", "Lam Chi Him / kelvelam6"),
        ("", ""),
        ("Supervisors SCM:", "Jussi Pekka HOLOPAINEN"),
        ("Supervisors CS:", "—"),
        ("", ""),
        ("Date:", "12 April 2026"),
    ]

    tbl = doc.add_table(rows=len(info_data), cols=2)
    tbl.autofit = True
    for ri, (label, value) in enumerate(info_data):
        tbl.rows[ri].cells[0].text = label
        tbl.rows[ri].cells[1].text = value
        for ci in range(2):
            for para in tbl.rows[ri].cells[ci].paragraphs:
                for run in para.runs:
                    run.font.name = 'Times New Roman'
                    run.font.size = Pt(12)
                if ci == 0:
                    for run in para.runs:
                        run.bold = True

    # Page break after cover
    doc.add_page_break()


def add_body_from_md(doc, md_path):
    """Parse markdown and add content to document."""
    with open(md_path, 'r', encoding='utf-8') as f:
        lines = f.read().split('\n')

    i = 0
    in_code = False
    table_rows = []

    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Code block toggle
        if stripped.startswith('```'):
            in_code = not in_code
            i += 1
            continue

        # Code content
        if in_code:
            p = doc.add_paragraph()
            run = p.add_run(line.rstrip())
            run.font.name = 'Courier New'
            run.font.size = Pt(9)
            pf = p.paragraph_format
            pf.space_before = Pt(0)
            pf.space_after = Pt(0)
            i += 1
            continue

        # Empty line
        if not stripped:
            i += 1
            continue

        # Skip the markdown title block (first few lines that duplicate cover)
        if stripped.startswith('# BScCM') or stripped.startswith('# Phase II'):
            i += 1
            continue

        # Horizontal rule
        if stripped == '---':
            i += 1
            continue

        # Table handling
        if '|' in stripped and stripped.startswith('|'):
            cells = [c.strip() for c in stripped.split('|')[1:-1]]
            if all(set(c.replace(' ', '')) <= set('-:') for c in cells):
                i += 1
                continue
            table_rows.append(cells)
            next_is_table = (i + 1 < len(lines) and
                             '|' in lines[i + 1].strip() and
                             lines[i + 1].strip().startswith('|'))
            if not next_is_table:
                if table_rows:
                    max_cols = max(len(r) for r in table_rows)
                    tbl = doc.add_table(rows=len(table_rows), cols=max_cols)
                    tbl.style = 'Table Grid'
                    for ri, row in enumerate(table_rows):
                        for ci, cell in enumerate(row):
                            if ci < max_cols:
                                cell_text = clean_md(cell)
                                tbl.rows[ri].cells[ci].text = cell_text
                                for para in tbl.rows[ri].cells[ci].paragraphs:
                                    for run in para.runs:
                                        run.font.size = Pt(10)
                                        run.font.name = 'Times New Roman'
                    for ci in range(max_cols):
                        if tbl.rows[0].cells[ci].paragraphs:
                            for para in tbl.rows[0].cells[ci].paragraphs:
                                for run in para.runs:
                                    run.bold = True
                    doc.add_paragraph()
                table_rows = []
            i += 1
            continue

        # Headings
        if stripped.startswith('#### '):
            doc.add_heading(clean_md(stripped[5:]), level=4)
            i += 1
            continue
        elif stripped.startswith('### '):
            doc.add_heading(clean_md(stripped[4:]), level=3)
            i += 1
            continue
        elif stripped.startswith('## '):
            heading_text = clean_md(stripped[3:])
            doc.add_heading(heading_text, level=2)
            i += 1
            continue
        elif stripped.startswith('# '):
            # Skip duplicate title headers from md
            i += 1
            continue

        # Bullet points
        if stripped.startswith('- '):
            text = clean_md(stripped[2:])
            doc.add_paragraph(text, style='List Bullet')
            i += 1
            continue

        # Numbered items with colon-separated metadata
        if re.match(r'^\*\*.*?\*\*', stripped):
            # Bold label line
            p = doc.add_paragraph()
            parts = re.split(r'(\*\*.*?\*\*)', stripped)
            for part in parts:
                if part.startswith('**') and part.endswith('**'):
                    run = p.add_run(part[2:-2])
                    run.bold = True
                    run.font.name = 'Times New Roman'
                    run.font.size = Pt(12)
                else:
                    run = p.add_run(clean_md(part))
                    run.font.name = 'Times New Roman'
                    run.font.size = Pt(12)
            i += 1
            continue

        # Regular text
        text = clean_md(stripped)
        if text:
            p = doc.add_paragraph(text)
            p.style = doc.styles['Normal']

        i += 1

    return doc


def main():
    print(f"Reading MD from: {MD_PATH}")
    if not os.path.exists(MD_PATH):
        print(f"ERROR: {MD_PATH} not found!")
        return

    doc = Document()

    # Set default font
    style = doc.styles['Normal']
    font = style.font
    font.name = 'Times New Roman'
    font.size = Pt(12)

    # Section 1: Cover page
    print("Building cover page...")
    add_cover_page(doc)

    # Section 2-16: Body content from markdown
    print("Building body content...")
    add_body_from_md(doc, MD_PATH)

    # Save
    doc.save(OUTPUT_PATH)
    print(f"\n=== COMPLETE ===")
    print(f"Saved: {OUTPUT_PATH}")
    print(f"Paragraphs: {len(doc.paragraphs)}")
    print(f"Tables: {len(doc.tables)}")


if __name__ == '__main__':
    main()
