// send_cot.cpp — Send various Cursor-on-Target (CoT) event types over UDP.
//
// By default sends to ATAK's mesh SA multicast group (239.2.3.1:6969),
// which any ATAK/WinTAK client on the same LAN listens to out of the box.
// Pass a host/port to unicast to a single device or TAK Server UDP input.
//
// Build:   cmake -B build && cmake --build build        (or)
//          g++ -std=c++17 -Wall -O2 -o send_cot send_cot.cpp
// Usage:   ./send_cot [host] [port] [--verbose|-v]
//          ./send_cot                    # multicast to 239.2.3.1:6969
//          ./send_cot 192.168.1.50 4242  # unicast to one device
//          ./send_cot --verbose          # also print the raw XML sent

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// UTC timestamp in the ISO-8601 'Z' form CoT expects, offset by `offsetSec`.
static std::string isoTime(int offsetSec = 0) {
    using namespace std::chrono;
    auto now = system_clock::now() + seconds(offsetSec);
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    char out[40];
    std::snprintf(out, sizeof(out), "%s.%03lldZ", buf,
                  static_cast<long long>(ms.count()));
    return out;
}

// Short random hex suffix so repeated runs produce fresh UIDs.
static std::string randomSuffix() {
    static std::mt19937_64 rng{std::random_device{}()};
    char buf[9];
    std::snprintf(buf, sizeof(buf), "%08llx",
                  static_cast<unsigned long long>(rng() & 0xffffffffULL));
    return buf;
}

// Build a complete CoT <event> XML document.
// `detailXml` is the raw inner XML placed inside <detail>...</detail>.
static std::string buildCot(const std::string& type,
                            const std::string& uid,
                            double lat, double lon,
                            const std::string& detailXml,
                            double hae = 9999999.0,
                            double ce = 9999999.0,
                            double le = 9999999.0,
                            const std::string& how = "m-g",
                            int staleSec = 300) {
    std::ostringstream xml;
    xml << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
        << "<event version=\"2.0\""
        << " uid=\"" << uid << "\""
        << " type=\"" << type << "\""
        << " time=\"" << isoTime() << "\""
        << " start=\"" << isoTime() << "\""
        << " stale=\"" << isoTime(staleSec) << "\""
        << " how=\"" << how << "\">"
        << "<point"
        << " lat=\"" << lat << "\""
        << " lon=\"" << lon << "\""
        << " hae=\"" << hae << "\""
        << " ce=\"" << ce << "\""
        << " le=\"" << le << "\"/>"
        << "<detail>" << detailXml << "</detail>"
        << "</event>";
    return xml.str();
}

// ---------------------------------------------------------------------------
// Example events — one per common CoT type family
// ---------------------------------------------------------------------------

// a-f-G-U-C: friendly ground combat unit (blue rectangle in ATAK).
static std::string friendlyGroundUnit() {
    std::string detail =
        "<contact callsign=\"RAVEN-1\"/>"
        "<__group name=\"Cyan\" role=\"Team Member\"/>"
        "<status battery=\"87\"/>"
        "<track course=\"270.0\" speed=\"1.5\"/>";
    return buildCot("a-f-G-U-C", "RAVEN-1-" + randomSuffix(),
                    38.8895, -77.0353, detail,
                    20.0, 9999999.0, 9999999.0, "h-g-i-g-o");
}

// a-h-G-U-C-I: hostile ground infantry (red diamond).
static std::string hostileGroundTrack() {
    std::string detail = "<contact callsign=\"HOSTILE-01\"/>";
    return buildCot("a-h-G-U-C-I", "HOSTILE-" + randomSuffix(),
                    38.8990, -77.0250, detail, 15.0);
}

// a-n-A-C-F: neutral fixed-wing civil aircraft (green square).
static std::string neutralAircraft() {
    std::string detail =
        "<contact callsign=\"CIVAIR-22\"/>"
        "<track course=\"045.0\" speed=\"128.6\"/>";
    return buildCot("a-n-A-C-F", "CIVAIR-" + randomSuffix(),
                    38.9445, -77.4558, detail, 1200.0, 50.0, 50.0);
}

// a-u-S: unknown sea-surface contact (yellow quatrefoil).
static std::string unknownSurfaceVessel() {
    std::string detail = "<contact callsign=\"UNK-VESSEL\"/>";
    return buildCot("a-u-S", "VESSEL-" + randomSuffix(),
                    38.8000, -76.9500, detail);
}

// b-m-p-w: a simple waypoint / map marker (a 'b' bit = non-track object).
static std::string waypointMarker() {
    std::string detail =
        "<contact callsign=\"RALLY-POINT-ALPHA\"/>"
        "<remarks>Rally here if comms are lost.</remarks>";
    return buildCot("b-m-p-w", "WPT-" + randomSuffix(),
                    38.8710, -77.0560, detail,
                    9999999.0, 9999999.0, 9999999.0, "m-g", 3600);
}

// u-d-c-c: a circular Geo Fence shape (ATAK-specific, not core MITRE CoT).
// ATAK renders this as a filled/outlined circle and monitors it using the
// <geofence> detail: alerts when tracked items enter/exit the boundingSphere
// (radius in meters) around the center point. See ATAK-CIV's
// GeoFence.java / GeoFenceReceiver.java for the full attribute set.
static std::string geofence(double radiusM) {
    std::ostringstream detail;
    detail << "<contact callsign=\"Geofence Alpha\"/>"
           << "<shape><ellipse major=\"" << radiusM << "\" minor=\"" << radiusM
           << "\" angle=\"0.0\"/></shape>"
           << "<fillColor value=\"1091567616\"/>"   // ARGB, translucent green
           << "<strokeColor value=\"-1\"/>"          // ARGB, opaque white
           << "<strokeWeight value=\"4.0\"/>"
           << "<labels_on value=\"true\"/>"
           << "<geofence monitor=\"All\" trigger=\"Both\""
           << " boundingSphere=\"" << static_cast<long>(radiusM) << "\""
           << " minElevation=\"NaN\" maxElevation=\"NaN\"/>";
    return buildCot("u-d-c-c", "GEOFENCE-" + randomSuffix(),
                    38.8895, -77.0353, detail.str(),
                    9999999.0, 9999999.0, 9999999.0, "h-e", 3600);
}

// Wrapper matching the no-argument factory signature used by the examples table.
static std::string geofenceExample() {
    return geofence(500.0);
}

// b-a-o-tbl: 911/emergency alert — triggers a red alert in ATAK.
static std::string emergencyBeacon() {
    std::string detail =
        "<emergency type=\"911 Alert\">RAVEN-1</emergency>"
        "<contact callsign=\"RAVEN-1\"/>";
    return buildCot("b-a-o-tbl", "RAVEN-1-9-1-1",
                    38.8895, -77.0353, detail,
                    9999999.0, 9999999.0, 9999999.0, "h-e", 600);
}

// b-t-f: GeoChat text message to the All Chat Rooms group.
static std::string geoChatMessage() {
    const std::string senderUid = "SENDER-UID-1234";
    const std::string chatroom = "All Chat Rooms";
    const std::string msgUid =
        "GeoChat." + senderUid + "." + chatroom + "." + randomSuffix();
    std::string detail =
        "<__chat parent=\"RootContactGroup\" groupOwner=\"false\""
        " chatroom=\"" + chatroom + "\" id=\"" + chatroom + "\""
        " senderCallsign=\"RAVEN-1\">"
        "<chatgrp uid0=\"" + senderUid + "\" uid1=\"" + chatroom + "\""
        " id=\"" + chatroom + "\"/>"
        "</__chat>"
        "<link uid=\"" + senderUid + "\" type=\"a-f-G-U-C\" relation=\"p-p\"/>"
        "<remarks source=\"BAO.F.ATAK." + senderUid + "\""
        " to=\"" + chatroom + "\" time=\"" + isoTime() + "\">"
        "Hello from the CoT UDP example!"
        "</remarks>";
    return buildCot("b-t-f", msgUid, 38.8895, -77.0353, detail,
                    9999999.0, 9999999.0, 9999999.0, "h-g-i-g-o");
}

// ---------------------------------------------------------------------------
// UDP send
// ---------------------------------------------------------------------------

static bool isMulticast(const in_addr& addr) {
    return IN_MULTICAST(ntohl(addr.s_addr));
}

int main(int argc, char* argv[]) {
    std::vector<std::string> positional;
    bool verbose = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else {
            positional.push_back(arg);
        }
    }
    const std::string host = positional.size() > 0 ? positional[0] : "239.2.3.1";
    const int port = positional.size() > 1 ? std::atoi(positional[1].c_str()) : 6969;

    sockaddr_in dest{};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, host.c_str(), &dest.sin_addr) != 1) {
        std::cerr << "invalid address: " << host << "\n";
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Multicast TTL of 1 keeps packets on the local subnet (raise to cross routers).
    if (isMulticast(dest.sin_addr)) {
        unsigned char ttl = 1;
        setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    }

    const std::vector<std::pair<std::string, std::string (*)()>> examples = {
        {"Friendly ground unit (a-f-G-U-C)",   friendlyGroundUnit},
        {"Hostile ground track  (a-h-G-U-C-I)", hostileGroundTrack},
        {"Neutral aircraft      (a-n-A-C-F)",   neutralAircraft},
        {"Unknown vessel        (a-u-S)",       unknownSurfaceVessel},
        {"Waypoint marker       (b-m-p-w)",     waypointMarker},
        {"Geofence (circle)     (u-d-c-c)",     geofenceExample},
        {"Emergency beacon      (b-a-o-tbl)",   emergencyBeacon},
        {"GeoChat message       (b-t-f)",       geoChatMessage},
    };

    for (const auto& [name, factory] : examples) {
        const std::string payload = factory();
        ssize_t sent = sendto(sock, payload.data(), payload.size(), 0,
                              reinterpret_cast<sockaddr*>(&dest), sizeof(dest));
        if (sent < 0) {
            perror("sendto");
        } else {
            std::printf("sent %4zd bytes -> %s:%d  %s\n",
                        sent, host.c_str(), port, name.c_str());
        }
        if (verbose) {
            std::printf("  %s\n", payload.c_str());
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(sock);
    return 0;
}
