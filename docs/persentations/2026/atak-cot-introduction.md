---
marp: true
theme: default
paginate: true
backgroundColor: #11140d
color: #e9e4d0
style: |
  section {
    font-size: 26px;
    background: #11140d;
    color: #e9e4d0;
    font-family: 'Segoe UI', 'Helvetica Neue', Arial, sans-serif;
    border-top: 6px solid #4b5320;
  }
  section.lead {
    display: flex;
    flex-direction: column;
    justify-content: center;
    border-top: none;
    position: relative;
    padding: 60px 90px;
    background:
      radial-gradient(ellipse 900px 600px at 78% 20%, rgba(75,83,32,0.5), transparent 60%),
      repeating-linear-gradient(0deg, rgba(217,201,138,0.07) 0px, rgba(217,201,138,0.07) 1px, transparent 1px, transparent 42px),
      repeating-linear-gradient(90deg, rgba(217,201,138,0.07) 0px, rgba(217,201,138,0.07) 1px, transparent 1px, transparent 42px),
      #0d0f0a;
  }
  section.lead h1 {
    font-size: 52px;
    letter-spacing: 1px;
    text-transform: uppercase;
  }
  section.lead h3 {
    font-weight: 400;
    color: #c9c2a0;
  }
  section.lead .kicker {
    display: inline-block;
    color: #11140d;
    background: #d9c98a;
    font-weight: 700;
    letter-spacing: 3px;
    font-size: 15px;
    padding: 5px 16px;
    margin-bottom: 22px;
  }
  section.lead .classbar {
    position: absolute;
    left: 0;
    right: 0;
    text-align: center;
    letter-spacing: 4px;
    font-size: 13px;
    font-weight: 700;
    color: #11140d;
    background: #8a9a52;
    padding: 7px 0;
  }
  section.lead .classbar.top { top: 0; }
  section.lead .classbar.bottom { bottom: 0; }
  section.lead .cover-glyph {
    position: absolute;
    right: 60px;
    bottom: 90px;
    font-size: 150px;
    color: rgba(217,201,138,0.08);
    line-height: 1;
  }
  h1 {
    color: #e9e4d0;
    text-shadow: 0 1px 3px #000;
  }
  h2 {
    color: #d9c98a;
    border-bottom: 3px solid #4b5320;
    padding-bottom: 6px;
    text-transform: uppercase;
    letter-spacing: 1px;
    font-size: 34px;
  }
  strong { color: #d9c98a; }
  a { color: #8fae52; }
  em { color: #b9b190; }
  code {
    color: #b5c99a;
    background: #1c2013;
  }
  pre {
    background: #1c2013;
    border: 1px solid #4b5320;
  }
  table {
    border-collapse: collapse;
    width: 100%;
  }
  table thead th {
    background: #3a4526;
    color: #e9e4d0;
    border: 1px solid #5c6b3c;
    text-transform: uppercase;
    font-size: 0.85em;
    letter-spacing: 0.5px;
  }
  table tbody td {
    background: #191d12;
    color: #e9e4d0;
    border: 1px solid #3a4526;
  }
  table tbody tr:nth-child(even) td {
    background: #20260c;
  }
  blockquote {
    border-left: 4px solid #d9c98a;
    color: #c9c2a0;
  }
  footer { color: #8a8768; font-size: 14px; }
---

<!-- _class: lead -->

<div class="classbar top">UNCLASSIFIED // TRAINING &amp; DEVELOPER REFERENCE</div>

<span class="kicker">TAK / ATAK BRIEFING</span>

# Introduction to ATAK & the CoT Protocol
### A primer for people new to TAK — messaging, protocol versions, and building plugins with Gradle

<div class="cover-glyph">⌖</div>
<div class="classbar bottom">v1.0 &nbsp;·&nbsp; ATAK-CIV &nbsp;·&nbsp; CoT Protocol &nbsp;·&nbsp; Gradle Plugin Dev</div>

---

## What is TAK?

- **TAK** = Team Awareness Kit — a family of situational-awareness apps and services originally built by the Air Force Research Laboratory (2010), based on NASA WorldWind.
- Shows a shared map with real-time positions, tracks, chat, drawings, and sensor feeds for every connected user.
- Used by military, first responders, and civil agencies worldwide (250k+ users).
- Extensible via a **plugin architecture** — this is how most mission-specific capability is added.

---

## The TAK Product Family

| Product | Platform | Purpose |
|---|---|---|
| **ATAK** | Android | Primary field client (mobile/tablet) |
| **WinTAK** | Windows | Desktop/command-post client |
| **iTAK** | iOS | Lightweight mobile client |
| **TAK Server** | Linux/Java | Routes, stores, and federates CoT between clients |

ATAK distributions: **ATAK-CIV** (public/first-responder, open source), **ATAK-GOV**, **ATAK-MIL** (ITAR-restricted), each built from the same core.

*Source: [tak.gov](https://tak.gov), [Wikipedia – Android Team Awareness Kit](https://en.wikipedia.org/wiki/Android_Team_Awareness_Kit)*

---

## ATAK in the Field

![bg right:55% fit](images/atak-screenshot.jpg)

- Map-centric UI with pan/zoom, offline map tiles
- Friendly/hostile icons follow MIL-STD-2525 / APP-6 symbology
- Chat, geofences, routes, casualty reporting, sensor feeds
- Everything shown on the map is driven by **CoT messages**

*Image: Wikimedia Commons, "Screenshot of ATAK-PR on Android"*

---

## Improving Your Maps & Imagery

![bg right:28% fit](images/atak-maps-qr.png)

- ATAK ships with limited built-in basemaps — most teams add extra **map sources** (MOBAC-format XML) for better offline coverage and imagery.
- **[joshuafuller/ATAK-Maps](https://github.com/joshuafuller/ATAK-Maps)** — a curated, open collection of 40+ satellite, topo, nautical, and overlay map sources.
- Scan the QR code on an ATAK device (5.1+) to install the whole set, or paste the link below into an on-device browser:
  ```
  tak://com.atakmap.app/import?url=https%3A%2F%2Fjoshuafuller.github.io%2FATAK-Maps%2Fpack%2Fatak-maps-all.zip
  ```
- **ESRI Clarity** is a strong default **satellite imagery** basemap — high-resolution, pairs well with a topo layer.

---

## Plugin Architecture

- ATAK's core stays generic; mission-specific features ship as **plugins** — separate Android APKs loaded at runtime.
- A plugin implements `IPlugin`/lifecycle interfaces and registers tools, map overlays, or CoT producers/consumers.
- Plugins are how commercial and government developers extend ATAK without modifying (or needing) the core source.
- Examples on tak.gov / GitHub: sensor integrations, routing, video, custom symbology.

---

## What is Cursor-on-Target (CoT)?

- **CoT** is the messaging schema TAK products use to describe *"what, where, when"* — a friendly unit, a sensor track, a chat message, a drawing.
- Originally developed by MITRE for the US Air Force to fuse sensor and command-and-control data.
- Transport-agnostic: works over UDP multicast (mesh), TCP/TLS to TAK Server (stream), or files.
- Every position update, chat message, and map object in TAK is a CoT **event**.

---

## CoT Message Structure

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<event version="2.0" uid="ANDROID-359785044927840"
       type="a-f-G-U-C" time="2026-08-13T12:00:00Z"
       start="2026-08-13T12:00:00Z"
       stale="2026-08-13T12:05:00Z" how="h-g-i-g-o">
  <point lat="38.8895" lon="-77.0353"
         hae="9999999.0" ce="9999999.0" le="9999999.0"/>
  <detail>
    <contact callsign="RAVEN-1"/>
    <__group name="Cyan" role="Team Member"/>
    <status battery="87"/>
  </detail>
</event>
```

- `type` — hierarchical affiliation/dimension code (MIL-STD-2525-derived), e.g. `a-f-G-U-C` = friendly ground unit, combat.
- `start`/`stale` — validity window; a receiver discards/greys out stale events.
- `<detail>` — free-form extension block (contact, group, status, chat, sensor payloads, etc.).

---

## Military Symbology (MIL-STD-2525D / APP-6D)

The CoT `type` code drives the **symbol** ATAK/WinTAK draw on the map — every friendly/hostile/neutral/unknown icon follows the same NATO standard.

| Friend | Hostile | Neutral | Unknown |
|:------:|:-------:|:-------:|:-------:|
| ![](../mkdocs/docs/images/bms/symbol/Frame_Friend_Land.png) | ![](../mkdocs/docs/images/bms/symbol/Frame_Hostile.png) | ![](../mkdocs/docs/images/bms/symbol/Frame_Neutral.png) | ![](../mkdocs/docs/images/bms/symbol/Frame_Unknown.png) |

- **Frame shape + colour** = affiliation (rectangle/blue = friend, diamond/red = hostile, square/green = neutral, quatrefoil/yellow = unknown).
- **Icon inside the frame** = entity type (infantry, tank, aircraft, ship...); **modifiers** add echelon/status.

---

## Military Symbology — SIDC Encoding

- Every symbol is encoded as a **SIDC** (Symbol Identification Code) — a 15- or 20-character string carrying affiliation, symbol set/dimension, and entity.
- ATAK/WinTAK derive the SIDC from the CoT `type` field (and vice versa) so the same event renders consistently across every TAK client.
- *Renders shown in this section are from LDM's own BMS symbology docs: [`docs/dev`](../dev) → TALOS BMS [military-symbology.md](../mkdocs/docs/bms/military-symbology.md).*

---

## Symbol Examples — Land, Air, Sea

| Friend | Hostile | Neutral | Unknown | Entity |
|:------:|:-------:|:-------:|:-------:|--------|
| ![](../mkdocs/docs/images/bms/symbol/F_LandUnit_121100.png) | ![](../mkdocs/docs/images/bms/symbol/H_LandUnit_121100.png) | ![](../mkdocs/docs/images/bms/symbol/N_LandUnit_121100.png) | ![](../mkdocs/docs/images/bms/symbol/U_LandUnit_121100.png) | Infantry |
| ![](../mkdocs/docs/images/bms/symbol/F_Air_110104.png) | ![](../mkdocs/docs/images/bms/symbol/H_Air_110104.png) | ![](../mkdocs/docs/images/bms/symbol/N_Air_110104.png) | ![](../mkdocs/docs/images/bms/symbol/U_Air_110104.png) | Fighter Aircraft |
| ![](../mkdocs/docs/images/bms/symbol/F_SeaSurface_120203.png) | ![](../mkdocs/docs/images/bms/symbol/H_SeaSurface_120203.png) | ![](../mkdocs/docs/images/bms/symbol/N_SeaSurface_120203.png) | ![](../mkdocs/docs/images/bms/symbol/U_SeaSurface_120203.png) | Destroyer |

- These are the exact symbols the **TALOS BMS** (`gva-app-bms`) renders for CoT/AIS/ADS-B tracks via `MilSymbolWidget`.
- Full catalogue (~1,100 entities × 4 affiliations): TALOS BMS [Symbol Reference](../mkdocs/docs/bms/symbol-reference.md).

---

## CoT Versions & Wire Formats

- **Schema version** — the `version="2.0"` attribute on `<event>` has been stable for years; the logical schema (event/point/detail) rarely changes.
- **Transport/wire format**:
  - **Plain XML CoT** — the original, human-readable format, still fully supported everywhere.
  - **TAK Protocol (Mesh & Stream)** — a **Protocol Buffers** binary encoding of the same CoT schema, used for bandwidth-constrained mesh/radio links and TAK Server streams (see `takproto` in the ATAK-CIV source tree).
- Endpoints negotiate XML vs. protobuf automatically — plain-XML CoT remains the universal fallback/interop format.

---

## CoT Flow Through TAK Server

![CoT flow between ATAK, WinTAK, and TAK Server](images/cot-flow.png)

- TAK Server brokers CoT between many clients, applies mission groups/channels, and persists mission data.
- Also federates between TAK Server instances and supports plugins of its own (routing, alerting, data feeds).

---

## Building ATAK Plugins — Prerequisites

- **Android Studio** + Android SDK/NDK matching the ATAK release you target.
- **ATAK Plugin SDK** — pulled straight from GitHub, two ways:
  ```bash
  # (a) clone the source repo (includes pluginsdk.zip at the root)
  git clone https://github.com/TAK-Product-Center/atak-civ.git

  # (b) or grab just the packaged SDK from a tagged release
  curl -LO https://github.com/TAK-Product-Center/atak-civ/releases/download/5.5.1.8/ATAK-CIV-5.5.1.8-SDK.zip
  ```
- The **`atak-gradle-takdev`** Gradle plugin (same repo) — wires the ATAK core AAR/API into your project so it builds against the real ATAK classes.
- A debug ATAK-CIV build installed on a device/emulator to load and test the plugin.

---

## Setting Up the Gradle Project

1. Start from the official `plugin-examples` in the ATAK-CIV source, or a community `atak-plugin-template` (Gradle/Android project skeleton).
2. Apply the ATAK Gradle plugin in `build.gradle`:
   ```gradle
   plugins {
     id 'com.atakmap.gradle.takdev'
   }
   ```
3. Point `local.properties` / `gradle.properties` at your ATAK Plugin SDK path and the **flavor** you're targeting: `civ`, `gov`, or `mil`.
4. Set your plugin's signing keystore — ATAK verifies plugin signatures before loading them.

---

## Build & Deploy

```bash
# Build the plugin APK for the civilian ATAK flavor
./gradlew assembleCivDebug

# Install alongside a debug ATAK-CIV build
adb install -r app/build/outputs/apk/civ/debug/app-civ-debug.apk
```

- ATAK's **Plugin Manager** (Settings → Tool Preferences → Plugins) lists installed plugins and lets you enable/disable them.
- Use `assembleCivRelease` (signed) for distribution outside debug testing.
- Iterate: edit → `gradlew assemble...Debug` → reinstall → relaunch ATAK.

---

## Debugging & Resources

- `adb logcat` — plugin lifecycle events, exceptions, CoT traffic if you log it.
- Android Studio "Attach Debugger to Process" → pick the ATAK process hosting your plugin.
- Key references:
  - [tak.gov](https://tak.gov) — official downloads, docs, SDKs
  - [github.com/TAK-Product-Center/atak-civ](https://github.com/TAK-Product-Center/atak-civ) — ATAK-CIV source, `atak-gradle-takdev`, `plugin-examples`
  - [civtak.org](https://civtak.org) / [wiki.civtak.org](http://wiki.civtak.org) — community support & docs

---

<!-- _class: lead -->

# Summary

- TAK = shared situational awareness across ATAK, WinTAK, iTAK, and TAK Server.
- **CoT** is the common event schema (`event`/`point`/`detail`) — plain XML or protobuf on the wire, same logical data.
- Capability is added through **plugins**, built with Android Studio + Gradle against the ATAK Plugin SDK.

### Questions?
