# Research, Analyse & Development Report — Group Work

Cover Page
- Group Name: AIGEN
- Group No.: TBA
- Project Title: Arcane Souls: Rebirth
- Advisor: Jussi Pekka HOLOPAINEN (SCM)
- Course Code: SM4712B
- Students:
  - Lee Chun Kit (SID: 57306141, EID: cklee96) — Group Leader
  - Lam Chi Him (SID: 57185861, EID: kelvilam6)
- Submission Date: 31 July 2025
- Individual Blogs:
  - Lee Chun Kit: https://leechunkit01255210.wixsite.com/e-portfolio
  - Lam Chi Him: https://nihonjin864.wixsite.com/lamchihim

Abstract (≈180–220 words)
Arcane Souls: Rebirth is a 2D anime‑style action‑adventure developed with Unreal Engine 5 (UE5) that fuses the deliberate, timing‑based combat philosophy of Sekiro: Shadows Die Twice with a hand‑drawn aesthetic and a narrative of rebirth. The project seeks to address the gap between high‑fidelity 3D “souls‑like” titles and stylized 2D games by delivering precise posture‑driven combat, responsive parry/dodge systems, and emotionally resonant storytelling through cinematic presentation and environmental cues. Technically, we adapt UE5 features—Lumen lighting, Behavior Trees, and Paper2D—into a performant 2D pipeline targeting 60 FPS at up to 4K, with modular level design for replayability and accessibility options such as adjustable parry timing windows. The research component spans mechanic design for player skill mastery, enemy AI responsiveness, and interaction design for smooth motion feedback. Planned deliverables include anime‑style art assets, five distinct levels, and robust AI across at least ten enemy archetypes. Milestones progress from concept and prototyping through full‑scale development, balancing, and optimization. This report presents the problem framing, literature context, methodology, requirements and design, development plan, evaluation approach, and risk management, laying the foundation for iterative builds that honor souls‑like challenge while innovating in 2D game expression.

Table of Contents
- 1. Introduction
- 2. Related Work / Literature Review
- 3. Research Questions and Methodology
- 4. Requirements and Design
- 5. Development Process
- 6. Analysis and Results
- 7. Testing and Quality Assurance
- 8. Project Management
- 9. Limitations and Future Work
- 10. Conclusion
- References
- Appendices

## 1. Introduction
Background and Motivation
Souls‑like games emphasize precise, timing‑based combat and high‑stakes encounters that reward mastery. Contemporary engines primarily showcase 3D realism, leaving an opportunity for 2D titles that preserve depth of challenge with expressive anime presentation. We aim to merge these strengths in a UE5‑based 2D pipeline.

Problem Statement and Scope
There is an underrepresentation of challenging, posture‑centric 2D action games with polished visuals and responsive AI. Scope covers a vertical slice demonstrating posture‑based combat, enemy behaviors, and modular levels within UE5, targeting Windows PC.

Objectives and Success Criteria
- Creative: Deliver a cohesive anime aesthetic with cinematic storytelling consistent with themes of rebirth and arcane mystery.
- Technical: Achieve responsive parry/dodge/combo mechanics; integrate Lumen‑aware 2D lighting; implement Behavior Tree‑driven AI.
- Performance: Target 60 FPS at 1080p–4K on mid‑range PCs; input latency suitable for precise parry windows (<100 ms end‑to‑end).
- Usability: Clear telegraphs and feedback (animation, SFX, VFX) measured by player success rates and post‑test surveys.

Contributions
- A reproducible 2D souls‑like combat model in UE5.
- A design framework for posture/guard‑break mechanics in 2D.
- Practical patterns for 2D lighting, animation, and AI in UE5.

## 2. Related Work / Literature Review
- Souls‑like combat design: timing, telegraphing, posture/poise models.
- Enemy AI in action games: finite state machines, Behavior Trees, and designer‑friendly authoring.
- 2D pipelines in modern engines: Paper2D, sprite animation, lighting/shadowing for stylized art.
- Player experience research on difficulty, flow, and mastery.
Note: Formal references to be finalized in a consistent style (APA/IEEE). Candidate sources: Sekiro design analyses; UE5 Paper2D/AI docs; academic works on Behavior Trees and game difficulty.

## 3. Research Questions and Methodology
Research Questions
- RQ1: How can posture‑based combat be adapted to 2D to preserve souls‑like depth and fairness?
- RQ2: What AI behavior patterns best elicit mastery without frustration in 2D timing‑centric combat?
- RQ3: How do visual/audio telegraphs influence parry timing accuracy and perceived responsiveness?

Methodology
- Approach: Mixed‑methods. Quantitative telemetry (parry success rate, posture break times, frame times). Qualitative think‑aloud and SUS/game‑specific questionnaires.
- Data Sources: Internal playtests across iterative prototypes; task scenarios focusing on parry, dodge, and counter.
- Instruments: In‑game logging, screen capture, post‑session surveys.
- Validity & Ethics: Informed consent for testers, anonymized data, reproducible builds with fixed seeds and configs.

## 4. Requirements and Design
Functional Requirements
- FR1: Posture/guard system with decisive strike on break.
- FR2: Parry, dodge, and timing‑based counter mechanics.
- FR3: Enemy AI across ≥10 archetypes with adaptive patterns via Behavior Trees.
- FR4: At least five handcrafted levels with modular/replayable segments.
- FR5: Cinematic cutscenes and environmental storytelling beats.

Non‑Functional Requirements
- NFR1: 60 FPS target at 1080p–4K; input latency suitable for parry windows.
- NFR2: Configurable difficulty for timing windows and damage models.
- NFR3: Accessibility options (e.g., visual/audio telegraph clarity, colorblind‑safe palettes).

Acceptance Criteria (Examples)
- AC1: Average skilled players achieve ≥60% parry success by the second session.
- AC2: Boss posture breaks occur within 45–120 seconds in baseline difficulty.
- AC3: Frame time p95 < 20 ms on target hardware during combat scenes.

System Architecture (Overview)
- UE5 project using Paper2D for sprites/flipbooks; Lumen‑aware 2D lighting where applicable.
- Gameplay layer: posture, stamina/poise, damage, and i‑frames.
- AI layer: Behavior Trees and blackboard data for stateful reactions.
- Level framework: modular rooms, spawn tables, and encounter scripting.

UI/UX
- Clear telegraphs (anticipation frames, SFX/VFX cues).
- Diegetic feedback for posture status; concise HUD for stamina/posture.

## 5. Development Process
Process Model
- Agile‑leaning iterative development with short playtest cycles.

Milestones (from proposal)
- Concept & Pre‑production (Month 1): Game concept, GDD, initial art.
- Prototyping (Months 2–3): Core combat loop, posture, parry timing sandbox.
- Development (Months 4–8): Levels, AI archetypes, content pipeline.
- Testing (Months 9–10): Balancing, bug fixing, usability tests.
- Optimization (Month 11): Performance tuning, asset polish.
- Release (Month 12): Final testing, packaging, and release prep.

Version Control & Branching
- Main (stable), develop (integration), feature/* (mechanics, AI, levels), hotfix/*.

## 6. Analysis and Results
Evaluation Plan
- Metrics: parry accuracy, time‑to‑posture‑break, deaths per encounter, frame time stats.
- Experiments: difficulty variants of parry windows; AI aggressiveness tuning; lighting impact on readability.
- Reporting: plots for learning curves across sessions; heatmaps for player failure points.

Current Findings (to be updated during sprints)
- Prototype telemetry baselines and usability feedback will populate this section.

## 7. Testing and Quality Assurance
- Unit tests for posture calculations and hit detection.
- Integration tests for AI behavior trees and encounter scripts.
- System tests for performance and loading; GPU/CPU profiling.
- Usability tests with task scenarios; surveys (SUS + custom scales).
- Defect tracking with severity/priority triage; regression gates before merges to main.

## 8. Project Management
Roles & Responsibilities
- Lee Chun Kit: Combat systems, AI behavior authoring, integration lead.
- Lam Chi Him: Level design, art/animation pipeline, UI/UX and VFX cues.

Timeline & Resources
- Platform: PC (Windows).
- Engine/Tools: Unreal Engine 5, Paper2D, Behavior Trees, Blueprints.
- Risks: Performance with complex scenes; AI tuning; content scope creep.
- Mitigation: Budgeted profiling sprint; AI tuning playbooks; content cut criteria.

## 9. Limitations and Future Work
- Adapting Lumen and high‑fidelity effects to a 2D pipeline may constrain lower‑end hardware.
- Behavior Tree complexity can grow; consider utility AI for scalability.
- Future: Additional enemy/boss variety, advanced accessibility presets, controller haptics, and console ports.

## 10. Conclusion
We outlined the rationale and plan for a 2D souls‑like in UE5, defined objectives and evaluation criteria, and detailed design and process structures. Next sprints will validate combat feel, AI readability, and performance, iterating toward a polished vertical slice.

References (Placeholders — finalize in one consistent style)
- FromSoftware. Sekiro: Shadows Die Twice, 2019.
- Epic Games. Unreal Engine 5 Documentation: Paper2D, Behavior Trees, and Lumen.
- Selected academic/industry sources on Behavior Trees, flow, and difficulty design.

Appendices
A. Deliverables (from proposal)
- 2D anime‑style art assets (characters, enemies, environments, UI).
- Five distinct levels with modular designs.
- Enemy AI system with ≥10 archetypes using Behavior Trees.
B. Production Plan & Schedule (table/figure to insert in Word)
C. Additional figures: architecture diagrams, UI mockups, encounter scripts

Authoring & Submission Notes
- Copy this content under the official cover `Documents/ReportOnResearchAnalyseDevelopment_Cover.doc`.
- Apply Heading styles for automatic ToC; caption all figures/tables.
- Export to PDF with fonts embedded. Suggested name: FYP_Group<NN>_Research-Analyse-Development_Report.pdf.
- Each member: publish the PDF to your individual project blog.
- Group leader: submit by 24 Oct, 11:59pm as per LMS instructions.
