# Ground Station Ingestor

## Overview

The Ingestor is a **Linux daemon** (a background process running without direct user interaction) 
designed for a Mission Control System (MCS). It receives raw telemetry from a satellite ground 
station and parses it at high speed.

---

## Architecture

### Communication: Unix Domain Sockets

The ingestor uses a **Unix Domain Socket** — a local inter-process communication endpoint that uses the standard POSIX API without network stack overhead.

**Why a socket?**

- Simulates a real hardware interface or network feed
- Enables testing of **non-blocking I/O** using `poll()` or `epoll()`
- Keeps the standard POSIX API so the code transfers directly to TCP/IP sockets

**Components:**

| Role                      | Description                                                                   |
| ------------------------- | ----------------------------------------------------------------------------- |
| **Simulator** (Producer)  | A Bash script that opens the socket and writes binary packet blobs            |
| **Dispatcher** (Consumer) | The C++ program that `bind()`s to the socket and `recv()`s data into a buffer |

---

## Telemetry Packet Format

Following a simplified **ECSS** (European Cooperation for Space Standardization) structure, each packet is a flat binary blob:

| Field      | Size     | Contents                                                                |
| ---------- | -------- | ----------------------------------------------------------------------- |
| Header     | 6 bytes  | Version Number, Packet ID (PID), Sequence Count                         |
| Data Field | Variable | Sensor readings — e.g. Temperature (`float`), Battery Voltage (`float`) |
| Checksum   | 2 bytes  | Integrity check                                                         |

---

## Telemetry Flow: Spacecraft to Code

In a real-world Ground Segment (GS), telemetry follows a rigorous path:

1. **Generation & Transmission** 
   - Sensors on the satellite generate data.
   - The On-Board Computer (OBC) packages it into TM packets (typically CCSDS standard) and transmits via radio.
2. **Reception & Front-End Processing**
   - The ground station antenna captures the signal
   - A Front-End Processor (FEP) or Software Defined Radio (SDR) demodulates it back to a digital bitstream.
3. **Data Distribution**
   - The raw binary stream is sent over TCP/IP or UDP to the Mission Control Center.
4. **Ingestion (Your C++ Program)**
   - The ingestor listens on a socket, pulls the raw bytes, and begins high-speed parsing.

---

## Getting Started

### Step 1: Socket Setup

A Unix Domain Socket appears as a file in the filesystem (e.g. `/tmp/tm_socket`). If the daemon crashes, that file may persist and block a restart.

**Solution:** Call `unlink()` at program startup to remove any stale socket file before creating a new one.

```c
// Remove stale socket file from a previous run
unlink("/tmp/tm_socket");
```

### Step 2: Build the Dispatcher

> *Coming soon — C++20 Concepts, lock-free queues, and the full parsing pipeline.*

---

## Glossary

| Abbreviation | Meaning                                        |
| ------------ | ---------------------------------------------- |
| CCSDS        | Consultative Committee for Space Data Systems  |
| ECSS         | European Cooperation for Space Standardization |
| FEP          | Front-End Processor                            |
| GS           | Ground Segment                                 |
| MCS          | Mission Control System                         |
| OBC          | On-Board Computer                              |
| PID          | Packet ID                                      |
| POSIX        | Portable Operating System Interface            |
| SDR          | Software Defined Radio                         |
| TM           | Telemetry                                      |
