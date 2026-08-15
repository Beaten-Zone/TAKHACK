# TAKHACK

**TAK-HACK 2026** is a defence-focused innovation hackathon hosted by [Beaten Zone Venture Partners](https://www.beatenzone.vc/), bringing together founders, operators, engineers, technologists, and national security stakeholders to solve real-world operational challenges. It runs **14–16 August 2026** at The Precinct, Fortitude Valley, Brisbane, Australia.

Built around rapid collaboration, capability development, and mission-focused problem solving, TAK-HACK connects emerging technology with practical defence applications — with a focus on defence technology, autonomy, ISR, and emerging capability themes, including TAK/ATAK and the Cursor-on-Target (CoT) protocol.

More info: [beatenzone.vc/event/takhack-2026](https://www.beatenzone.vc/event/takhack-2026/)

## This Repository

This repo contains reference material and example code for working with TAK/ATAK and CoT:

- [docs/persentations/2026/atak-cot-introduction.md](docs/persentations/2026/atak-cot-introduction.md) — a primer on ATAK and the CoT protocol.
- [examples/cot-udp](examples/cot-udp) — C++ and Python examples that build and send various CoT event types over UDP.

### CoT Type Icons

The `type` code on a CoT event drives the affiliation frame ATAK/WinTAK draws on the map. The icons below (also used to tell the example events apart at a glance) correspond to the frame shapes/colours in the [MIL-STD-2525](docs/persentations/2026/atak-cot-introduction.md) affiliation scheme:

| Icon | CoT Type | Meaning |
|:---:|---|---|
| ![](images/CoT/a-f-G-U-C.png) | `a-f-G-U-C` | Friendly ground combat unit |
| ![](images/CoT/a-h-G-U-C-I.png) | `a-h-G-U-C-I` | Hostile ground infantry |
| ![](images/CoT/a-n-A-C-F.png) | `a-n-A-C-F` | Neutral fixed-wing civil aircraft |
| ![](images/CoT/a-u-S.png) | `a-u-S` | Unknown surface vessel |
| ![](images/CoT/a-f-A.png) | `a-f-A` | Friendly air track |
| ![](images/CoT/a-f-S.png) | `a-f-S` | Friendly surface/sea track |

