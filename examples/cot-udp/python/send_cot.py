#!/usr/bin/env python3
"""Send various Cursor-on-Target (CoT) event types over UDP.

By default sends to ATAK's mesh SA multicast group (239.2.3.1:6969),
which any ATAK/WinTAK client on the same LAN listens to out of the box.
Can also unicast straight to a device or TAK Server UDP input.

Usage:
    python3 send_cot.py                        # multicast to 239.2.3.1:6969
    python3 send_cot.py --host 192.168.1.50    # unicast to one device
    python3 send_cot.py --host 239.2.3.1 --port 6969 --interval 5 --loop
"""

import argparse
import socket
import time
import uuid
from datetime import datetime, timedelta, timezone
from xml.etree import ElementTree as ET


def iso_time(offset_seconds: float = 0.0) -> str:
    """UTC timestamp in the ISO-8601 'Z' form CoT expects."""
    t = datetime.now(timezone.utc) + timedelta(seconds=offset_seconds)
    return t.strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3] + "Z"


def build_cot(
    cot_type: str,
    uid: str,
    lat: float,
    lon: float,
    hae: float = 9999999.0,
    ce: float = 9999999.0,
    le: float = 9999999.0,
    how: str = "m-g",
    stale_seconds: int = 300,
    detail: ET.Element | None = None,
) -> bytes:
    """Build a CoT <event> document and return it as UTF-8 XML bytes."""
    event = ET.Element(
        "event",
        version="2.0",
        uid=uid,
        type=cot_type,
        time=iso_time(),
        start=iso_time(),
        stale=iso_time(stale_seconds),
        how=how,
    )
    ET.SubElement(
        event,
        "point",
        lat=f"{lat:.7f}",
        lon=f"{lon:.7f}",
        hae=f"{hae:.1f}",
        ce=f"{ce:.1f}",
        le=f"{le:.1f}",
    )
    event.append(detail if detail is not None else ET.Element("detail"))
    return ET.tostring(event, encoding="utf-8", xml_declaration=True)


# ---------------------------------------------------------------------------
# Example events — one per common CoT type family
# ---------------------------------------------------------------------------

def friendly_ground_unit() -> bytes:
    """a-f-G-U-C: friendly ground combat unit (blue rectangle in ATAK)."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="RAVEN-1")
    ET.SubElement(detail, "__group", name="Cyan", role="Team Member")
    ET.SubElement(detail, "status", battery="87")
    ET.SubElement(detail, "track", course="270.0", speed="1.5")
    return build_cot(
        "a-f-G-U-C",
        uid="RAVEN-1-" + uuid.uuid4().hex[:8],
        lat=38.8895, lon=-77.0353, hae=20.0,
        how="h-g-i-g-o",
        detail=detail,
    )


def hostile_ground_track() -> bytes:
    """a-h-G-U-C-I: hostile ground infantry (red diamond)."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="HOSTILE-01")
    return build_cot(
        "a-h-G-U-C-I",
        uid="HOSTILE-" + uuid.uuid4().hex[:8],
        lat=38.8990, lon=-77.0250, hae=15.0,
        detail=detail,
    )


def neutral_aircraft() -> bytes:
    """a-n-A-C-F: neutral fixed-wing civil aircraft (green square)."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="CIVAIR-22")
    ET.SubElement(detail, "track", course="045.0", speed="128.6")
    return build_cot(
        "a-n-A-C-F",
        uid="CIVAIR-" + uuid.uuid4().hex[:8],
        lat=38.9445, lon=-77.4558, hae=1200.0, ce=50.0, le=50.0,
        detail=detail,
    )


def unknown_surface_vessel() -> bytes:
    """a-u-S: unknown sea-surface contact (yellow quatrefoil)."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="UNK-VESSEL")
    return build_cot(
        "a-u-S",
        uid="VESSEL-" + uuid.uuid4().hex[:8],
        lat=38.8000, lon=-76.9500,
        detail=detail,
    )


def waypoint_marker() -> bytes:
    """b-m-p-w: a simple waypoint / map marker (a 'b' bit = non-track object)."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="RALLY-POINT-ALPHA")
    remarks = ET.SubElement(detail, "remarks")
    remarks.text = "Rally here if comms are lost."
    return build_cot(
        "b-m-p-w",
        uid="WPT-" + uuid.uuid4().hex[:8],
        lat=38.8710, lon=-77.0560,
        stale_seconds=3600,
        detail=detail,
    )


def geofence(radius_m: float = 500.0) -> bytes:
    """u-d-c-c: a circular Geo Fence shape (ATAK-specific, not core MITRE CoT).

    ATAK renders this as a filled/outlined circle and monitors it using the
    <geofence> detail: alerts when tracked items enter/exit the boundingSphere
    (radius in meters) around the center point. See ATAK-CIV's
    GeoFence.java / GeoFenceReceiver.java for the full attribute set.
    """
    detail = ET.Element("detail")
    ET.SubElement(detail, "contact", callsign="Geofence Alpha")
    ET.SubElement(detail, "shape").append(
        ET.Element("ellipse", major=f"{radius_m:.1f}", minor=f"{radius_m:.1f}", angle="0.0")
    )
    ET.SubElement(detail, "fillColor", value="1091567616")   # ARGB, translucent green
    ET.SubElement(detail, "strokeColor", value="-1")          # ARGB, opaque white
    ET.SubElement(detail, "strokeWeight", value="4.0")
    ET.SubElement(detail, "labels_on", value="true")
    ET.SubElement(
        detail, "geofence",
        monitor="All", trigger="Both",
        boundingSphere=f"{radius_m:.0f}",
        minElevation="NaN", maxElevation="NaN",
    )
    return build_cot(
        "u-d-c-c",
        uid="GEOFENCE-" + uuid.uuid4().hex[:8],
        lat=38.8895, lon=-77.0353,
        how="h-e",
        stale_seconds=3600,
        detail=detail,
    )


def emergency_beacon() -> bytes:
    """b-a-o-tbl: 911/emergency alert — triggers a red alert in ATAK."""
    detail = ET.Element("detail")
    ET.SubElement(detail, "emergency", type="911 Alert").text = "RAVEN-1"
    ET.SubElement(detail, "contact", callsign="RAVEN-1")
    return build_cot(
        "b-a-o-tbl",
        uid="RAVEN-1-9-1-1",
        lat=38.8895, lon=-77.0353,
        how="h-e",
        stale_seconds=600,
        detail=detail,
    )


def geochat_message(sender_uid: str = "SENDER-UID-1234") -> bytes:
    """b-t-f: GeoChat text message to the All Chat Rooms group."""
    chatroom = "All Chat Rooms"
    msg_uid = f"GeoChat.{sender_uid}.{chatroom}.{uuid.uuid4()}"
    detail = ET.Element("detail")
    chat = ET.SubElement(
        detail, "__chat",
        parent="RootContactGroup", groupOwner="false",
        chatroom=chatroom, id=chatroom, senderCallsign="RAVEN-1",
    )
    ET.SubElement(chat, "chatgrp", uid0=sender_uid, uid1=chatroom, id=chatroom)
    ET.SubElement(detail, "link", uid=sender_uid, type="a-f-G-U-C", relation="p-p")
    remarks = ET.SubElement(
        detail, "remarks",
        source=f"BAO.F.ATAK.{sender_uid}", to=chatroom, time=iso_time(),
    )
    remarks.text = "Hello from the CoT UDP example!"
    return build_cot(
        "b-t-f",
        uid=msg_uid,
        lat=38.8895, lon=-77.0353,
        how="h-g-i-g-o",
        detail=detail,
    )


EXAMPLES = [
    ("Friendly ground unit (a-f-G-U-C)", friendly_ground_unit),
    ("Hostile ground track  (a-h-G-U-C-I)", hostile_ground_track),
    ("Neutral aircraft      (a-n-A-C-F)", neutral_aircraft),
    ("Unknown vessel        (a-u-S)", unknown_surface_vessel),
    ("Waypoint marker       (b-m-p-w)", waypoint_marker),
    ("Geofence (circle)     (u-d-c-c)", geofence),
    ("Emergency beacon      (b-a-o-tbl)", emergency_beacon),
    ("GeoChat message       (b-t-f)", geochat_message),
]


def make_socket(host: str, ttl: int) -> socket.socket:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # 239.0.0.0/8 etc. — set multicast TTL so packets cross the LAN
    if ipaddress_is_multicast(host):
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)
    return sock


def ipaddress_is_multicast(host: str) -> bool:
    try:
        first_octet = int(host.split(".")[0])
        return 224 <= first_octet <= 239
    except ValueError:
        return False


def main() -> None:
    parser = argparse.ArgumentParser(description="Send example CoT events over UDP.")
    parser.add_argument("--host", default="239.2.3.1", help="destination IP (default: ATAK mesh SA multicast)")
    parser.add_argument("--port", type=int, default=6969, help="destination UDP port (default: 6969)")
    parser.add_argument("--ttl", type=int, default=1, help="multicast TTL (default: 1)")
    parser.add_argument("--interval", type=float, default=1.0, help="seconds between events")
    parser.add_argument("--loop", action="store_true", help="keep re-sending the whole set")
    parser.add_argument("--verbose", "-v", action="store_true", help="print the raw XML of each event sent")
    args = parser.parse_args()

    sock = make_socket(args.host, args.ttl)
    dest = (args.host, args.port)

    try:
        while True:
            for name, factory in EXAMPLES:
                payload = factory()
                sock.sendto(payload, dest)
                print(f"sent {len(payload):4d} bytes -> {args.host}:{args.port}  {name}")
                if args.verbose:
                    print(f"  {payload.decode('utf-8')}")
                time.sleep(args.interval)
            if not args.loop:
                break
    except KeyboardInterrupt:
        pass
    finally:
        sock.close()


if __name__ == "__main__":
    main()
