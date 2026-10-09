# XZH BMS Modbus RTU Protocol V0.3 (unofficial English translation)

- **Original:** 上海鑫智恒 Modbus_RTU 通信协议 (中文版), Shanghai Xinzhiheng Electronic Technology Co., Ltd (Seplos), www.xzhpower.com
- **Version:** V0.3 (revised 2024-10-15), document no.: none
- **Port:** RS485, 19200 8N1
- **Original file:** `XZH BMS Modbus-RTU Protocol V0.3 (Chinese).pdf` in this folder
- **Official English edition:** `XZH BMS Modbus-RTU Protocol V0.3 (English, official).pdf` in this folder (see below for how it differs)

This is a community translation of the Chinese edition of Seplos's own protocol document, which Seplos supplied in September 2026. It is not published or endorsed by Seplos. I've kept the structure, register addresses, units and example values exactly as in the original. Where the Chinese text contains an obvious slip, the translation says what it clearly means and flags the slip with [sic].

It covers the RS485 Modbus RTU interface of the Seplos BMS 3.0 generation (10E and later boards). It does not cover the CAN inverter protocols.

## The official English edition

Seplos has since sent its own English edition of V0.3 (PDF dated 2024-11-28), included in this folder. This translation follows the Chinese edition, which is the more complete of the two. Where they differ:

- **EIA / EIB / EIC device ID:** the English edition gives the EMS as **0xB0-0xBF** and an "ECU" as **0xC0**. The Chinese edition says 0x00 (section 2.2). Both list 0xC0 among the Bluetooth IDs.
- **0x1011:** the English edition names it **"Precharge percent"** (UINT16, 0.1%). The Chinese edition leaves it reserved, though its worked example shows `03 E8` = 100.0% there.
- **Exception codes:** the English edition gives **0x06** for "slave device busy", the standard Modbus code, where the Chinese edition has 0x07. It also adds **0x81, "No history record"**.
- **What the English edition doesn't have:**
  - the float lock registers 0x1328-0x132A
  - the network block (SPB, 0x1900-0x1974), despite V0.3's revision note
  - it also keeps V0.1's label "Secondary Charge current" for 0x1314, which the Chinese edition corrects to cell voltage difference protection recovery
- **Current unit:** Seplos told me (October 2026) that 400 A boards aren't in use yet. So V0.2's 0.1 A PIA current unit concerns 300 A boards.

## Revision history

| Date | Version | Change |
|---|---|---|
| 2023-02-09 | V0.1 | First draft. |
| 2024-06-12 | V0.2 | PIA current unit is 0.1 A on 300 A and 400 A boards, and 0.01 A on boards of 200 A and below. |
| 2024-10-15 | V0.3 | X02 and X03 boards add SNMP, Wi-Fi and gyro functions. |

## 1. Communication

### 1.1 Serial settings

| Baud rate | Parity | Data bits | Stop bits |
|---|---|---|---|
| 19200 | None | 8 | 1 |

### 1.2 Port

RS485: the BMS only responds to requests addressed to its own address.

## 2. Message structure

### 2.1 Supported function codes

| Function code | Meaning | Data blocks |
|---|---|---|
| 0x01 | Read coils | PIC / SFA / EIC |
| 0x0F | Write multiple coils | PIC / SFA / EIC |
| 0x04 | Read input registers | PIA / PIB / SPA / SCA / HIA / VIA / EIA / EIB / PCT |
| 0x10 | Write multiple registers | PIA / PIB / SPA / SCA / HIA / VIA / EIA / EIB / PCT |

### 2.2 Supported devices

| Device | Device ID | Data blocks |
|---|---|---|
| BMS | 0x00 – 0x7F | PIA / PIB / PIC / SPA / SFA / SCA / HIA / VIA |
| EMS | 0x00 (the official English edition gives EMS 0xB0-0xBF and ECU 0xC0) | EIA / EIB / EIC |
| 2.4″ / 5.0″ / 7.0″ display (TFT/LCD) | 0xE0 | PIA / PIB / PIC / VIA / PCT |
| Bluetooth | 0xE0 / 0x00 – 0x10 / 0xC0 | PIA / PIB / PIC / EIA / EIB / EIC / SCA / PCT |

### 2.3 Function code 0x04: read input registers

**Request**

| Slave addr | Function | Start addr | Register count | CRC |
|---|---|---|---|---|
| xx | 0x04 | Hi Lo | Hi Lo (N) | Lo Hi |

**Response**

| Slave addr | Function | Byte count | Register 1 | … | Register N | CRC |
|---|---|---|---|---|---|---|
| xx | 0x04 | 2N | Hi Lo | … | Hi Lo | Lo Hi |

### 2.4 Function code 0x10: write multiple registers

**Request**

| Slave addr | Function | Start addr | Register count | Byte count | Values | CRC |
|---|---|---|---|---|---|---|
| xx | 0x10 | Hi Lo | Hi Lo (N) | 2N | 2N bytes | Lo Hi |

**Response**

| Slave addr | Function | Start addr | Registers written | CRC |
|---|---|---|---|---|
| xx | 0x10 | Hi Lo | Hi Lo (N) | Lo Hi |

### 2.5 Function code 0x01: read coils

**Request**

| Slave addr | Function | Start addr | Coil count | CRC |
|---|---|---|---|---|
| xx | 0x01 | Hi Lo | Hi Lo (8N) | Lo Hi |

**Response**

| Slave addr | Function | Byte count | Coil status 1 | … | Coil status N | CRC |
|---|---|---|---|---|---|---|
| xx | 0x01 | N | xx | … | xx | Lo Hi |

### 2.6 Function code 0x0F: write multiple coils

**Request**

| Slave addr | Function | Start addr | Coil count | Byte count | Values | CRC |
|---|---|---|---|---|---|---|
| xx | 0x0F | Hi Lo | Hi Lo (8N) | N | N bytes | Lo Hi |

**Response**

| Slave addr | Function | Start addr | Coils written | CRC |
|---|---|---|---|---|
| xx | 0x0F | Hi Lo | Hi Lo (N) | Lo Hi |

### 2.7 Exception response

| Slave addr | Function | Exception code | CRC |
|---|---|---|---|
| xx | code + 128 | see 2.8 | Lo Hi |

### 2.8 Exception codes

| Code | Name | Meaning |
|---|---|---|
| 0x01 | Illegal function | The function code received in the query is not an allowed action for the server (slave). The function code may only apply to newer devices and not be implemented in the selected unit. It can also mean the server is in the wrong state to process such a request, for example because it is unconfigured and is being asked to return register values. |
| 0x02 | Illegal data address | The data address received in the query is not an allowed address for the server; specifically, the combination of reference number and transfer length is invalid. For a controller with 100 registers, a request with offset 96 and length 4 succeeds, while a request with offset 96 and length 5 produces exception 02. |
| 0x03 | Illegal data value | A value in the query is not an allowed value for the server. This indicates a fault in the structure of the rest of a complex request, for example an incorrect implied length. Modbus attaches no meaning to any particular value of any particular register; the register was given a value the application did not expect. |
| 0x04 | Slave device failure | An unrecoverable error occurred while the server was attempting to perform the requested action. |
| 0x05 | Acknowledge | Used with programming commands. The server has accepted the request and is processing it, but this will take a long time. The response is returned to prevent a timeout error in the client (master). The client can then send poll-program-complete messages to find out whether processing has finished. |
| 0x07 [sic: the official English edition and standard Modbus give 0x06] | Slave device busy | Used with programming commands. The server is processing a long-duration program command. The client should retransmit the message later, when the server is free. |
| 0x08 | Memory parity error | Used with function codes 20 and 21 and reference type 6, to indicate that the extended file area failed a consistency check. The server read the record file but found a parity error in memory. The client can retry the request, but the server device may need servicing. |
| 0x0A | Gateway path unavailable | Used with gateways. The gateway could not allocate an internal communication path from the input port to the output port to process the request. Usually means the gateway is misconfigured or overloaded. |
| 0x0B | Gateway target device failed to respond | Used with gateways. No response was obtained from the target device. Usually means the device is not on the network. |

## 3. Data

Temperatures are in 0.1 K. To convert: °C = (raw − 2731) / 10. Registers are big-endian (high byte first). “…” rows are reserved.

### Pack information A (PIA) · FC 0x04

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| 0x1000 | Pack voltage | 14 A1 = 52.81 V | R | UINT16 | 2 | 0.01 V |
| 0x1001 | Current (charge / discharge) | 05 FA = 15.30 A | R | INT16 | 2 | 0.01 A ¹ |
| 0x1002 | Remaining capacity | 44 5C = 175.00 Ah | R | UINT16 | 2 | 0.01 Ah |
| 0x1003 | Battery capacity | 4E 20 = 200.00 Ah | R | UINT16 | 2 | 0.01 Ah |
| 0x1004 | Total discharged capacity | 00 03 = 30 Ah | R | UINT16 | 2 | 10 Ah |
| 0x1005 | SOC | 03 6B = 87.5 % | R | UINT16 | 2 | 0.1 % |
| 0x1006 | SOH | 03 E8 = 100.0 % | R | UINT16 | 2 | 0.1 % |
| 0x1007 | Cycle count | 00 00 = 0 | R | UINT16 | 2 | times |
| 0x1008 | Average cell voltage | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1009 | Average cell temperature | 0B 80 = 294.4 K = 21.3 °C | R | UINT16 | 2 | 0.1 K |
| 0x100A | Highest cell voltage | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x100B | Lowest cell voltage | 0C E0 = 3.296 V | R | UINT16 | 2 | 0.001 V |
| 0x100C | Highest cell temperature | 0B 82 = 294.6 K = 21.5 °C | R | UINT16 | 2 | 0.1 K |
| 0x100D | Lowest cell temperature | 0B 7F = 294.3 K = 21.2 °C | R | UINT16 | 2 | 0.1 K |
| 0x100E | Reserved | … | … | … | … | … |
| 0x100F | Maximum discharge current | 00 B4 = 180 A | R | UINT16 | 2 | 1 A |
| 0x1010 | Maximum charge current | 00 B4 = 180 A | R | UINT16 | 2 | 1 A |
| … | Reserved | … | … | … | … | … |

¹ From V0.2: 0.1 A on 300 A and 400 A boards; 0.01 A on boards of 200 A and below. The example read in section 5 requests 0x12 (18) registers.

### Pack information B (PIB) · FC 0x04

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| 0x1100 | Cell voltage 01 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x1101 | Cell voltage 02 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1102 | Cell voltage 03 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1103 | Cell voltage 04 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1104 | Cell voltage 05 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1105 | Cell voltage 06 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1106 | Cell voltage 07 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1107 | Cell voltage 08 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x1108 | Cell voltage 09 | 0C E7 = 3.303 V | R | UINT16 | 2 | 0.001 V |
| 0x1109 | Cell voltage 10 | 0C E7 = 3.303 V | R | UINT16 | 2 | 0.001 V |
| 0x110A | Cell voltage 11 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x110B | Cell voltage 12 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x110C | Cell voltage 13 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x110D | Cell voltage 14 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x110E | Cell voltage 15 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x110F | Cell voltage 16 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1110 | Cell temperature 01 | 0B 82 = 294.6 K = 21.5 °C | R | UINT16 | 2 | 0.1 K |
| 0x1111 | Cell temperature 02 | 0B 7F = 294.3 K = 21.2 °C | R | UINT16 | 2 | 0.1 K |
| 0x1112 | Cell temperature 03 | 0B 81 = 294.5 K = 21.4 °C [original: 295.6 K] | R | UINT16 | 2 | 0.1 K |
| 0x1113 | Cell temperature 04 | 0B 7F = 294.3 K = 21.2 °C | R | UINT16 | 2 | 0.1 K |
| … | Reserved (0x1114 – 0x1117) | … | … | … | … | … |
| 0x1118 | Ambient temperature | 0B 91 = 296.1 K = 23.0 °C | R | UINT16 | 2 | 0.1 K |
| 0x1119 | Power (MOSFET) temperature | 0B 83 = 294.7 K = 21.6 °C | R | UINT16 | 2 | 0.1 K |
| … | Reserved | … | … | … | … | … |

### Pack information C (PIC) · FC 0x01 (coils)

Each entry is one byte (8 coils). For per-cell entries, 0 = normal / off and 1 = alarm / on.

| Address | Name | R/W | Type | Bytes | Values |
|---|---|---|---|---|---|
| 0x1200 | Cells 01 – 08 low voltage | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1208 | Cells 09 – 16 low voltage | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1210 | Cells 01 – 08 high voltage | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1218 | Cells 09 – 16 high voltage | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1220 | Cell temperature 01 – 08 low | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1228 | Cell temperature 01 – 08 high | R | HEX | 1 | 0 normal, 1 alarm |
| 0x1230 | Cells 01 – 08 balancing | R | HEX | 1 | 0 off, 1 on |
| 0x1238 | Cells 09 – 16 balancing | R | HEX | 1 | 0 off, 1 on |
| 0x1240 | System state | R | HEX | 1 | Table 4.1 |
| 0x1248 | Voltage events | R | HEX | 1 | Table 4.2 |
| 0x1250 | Cell temperature events | R | HEX | 1 | Table 4.3 |
| 0x1258 | Ambient and power temperature events | R | HEX | 1 | Table 4.4 |
| 0x1260 | Current events 1 | R | HEX | 1 | Table 4.5 |
| 0x1268 | Current events 2 | R | HEX | 1 | Table 4.6 |
| 0x1270 | Remaining capacity events | R | HEX | 1 | Table 4.7 |
| 0x1278 | FET state events | R | HEX | 1 | Table 4.8 |
| 0x1280 | Balancing state events | R | HEX | 1 | Table 4.9 |
| 0x1288 | Hardware failure events | R | HEX | 1 | Table 4.10 |
| … | Reserved | … | … | … | … |

### Battery configuration parameters (SPA) · FC 0x04 read, 0x10 write

Example values are those given in the document; a shipped pack may be configured differently.

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| **Pack and voltage** |  |  |  |  |  |  |
| 0x1300 | Number of cell temperature sensors | 00 04 = 4 | R/W | UINT16 | 2 | pcs |
| 0x1301 | Number of cells in series | 00 10 = 16 | R/W | UINT16 | 2 | S |
| 0x1302 | Pack high-voltage alarm recovery | 15 18 = 54.00 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1303 | Pack high-voltage alarm | 15 E0 = 56.00 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1304 | Pack over-voltage protection recovery | 15 18 = 54.00 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1305 | Pack over-voltage protection | 16 80 = 57.60 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1306 | Pack low-voltage alarm recovery | 12 C0 = 48.00 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1307 | Pack low-voltage alarm | 12 20 = 46.40 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1308 | Pack under-voltage protection recovery [original: “under-temperature”] | 12 C0 = 48.00 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1309 | Pack under-voltage protection | 12 0C = 43.20 V [sic: 12 0C is 46.20 V; 43.20 V would be 10 E0] | R/W | UINT16 | 2 | 0.01 V |
| 0x130A | Cell high-voltage alarm recovery | 0D 48 = 3.400 V | R/W | UINT16 | 2 | 0.001 V |
| 0x130B | Cell high-voltage alarm | 0D AC = 3.500 V | R/W | UINT16 | 2 | 0.001 V |
| 0x130C | Cell over-voltage protection recovery | 0D 48 = 3.400 V | R/W | UINT16 | 2 | 0.001 V |
| 0x130D | Cell over-voltage protection | 0E 42 = 3.650 V | R/W | UINT16 | 2 | 0.001 V |
| 0x130E | Cell low-voltage alarm recovery | 0C 1C = 3.100 V | R/W | UINT16 | 2 | 0.001 V |
| 0x130F | Cell low-voltage alarm | 0B 54 = 2.900 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1310 | Cell under-voltage protection recovery | 0C 1C = 3.100 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1311 | Cell under-voltage protection | 0A 8C = 2.700 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1312 | Cell under-voltage failure | 07 D0 = 2.000 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1313 | Cell voltage difference protection | 00 C8 = 0.200 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1314 | Cell voltage difference protection recovery | 00 64 = 0.100 V | R/W | UINT16 | 2 | 0.001 V |
| **Current** |  |  |  |  |  |  |
| 0x1315 | Charge over-current recovery | 00 67 = 103 A | R/W | INT16 | 2 | A |
| 0x1316 | Charge over-current alarm | 00 69 = 105 A | R/W | INT16 | 2 | A |
| 0x1317 | Charge over-current protection | 00 6E = 110 A | R/W | INT16 | 2 | A |
| 0x1318 | Charge over-current delay | 00 64 = 10.0 s | R/W | UINT16 | 2 | 0.1 s |
| 0x1319 | Charge over-current level 2 protection | 00 C8 = 200 A | R/W | INT16 | 2 | A |
| 0x131A | Charge over-current level 2 delay | 01 2C = 300 ms | R/W | UINT16 | 2 | ms |
| 0x131B | Discharge over-current recovery | FF 99 = −103 A | R/W | INT16 | 2 | A |
| 0x131C | Discharge over-current alarm | FF 97 = −105 A | R/W | INT16 | 2 | A |
| 0x131D | Discharge over-current protection | FF 92 = −110 A | R/W | INT16 | 2 | A |
| 0x131E | Discharge over-current delay | 00 64 = 10.0 s | R/W | UINT16 | 2 | 0.1 s |
| 0x131F | Discharge over-current level 2 protection | FF 06 = −250 A | R/W | INT16 | 2 | A |
| 0x1320 | Discharge over-current level 2 delay | 01 2C = 300 ms | R/W | UINT16 | 2 | ms |
| 0x1321 | Output short-circuit protection | FE D4 = −300 A | R/W | INT16 | 2 | A |
| 0x1322 | Output short-circuit delay | 01 2C = 300 µs | R/W | UINT16 | 2 | µs |
| 0x1323 | Over-current recovery delay | 02 58 = 60.0 s | R/W | UINT16 | 2 | 0.1 s |
| 0x1324 | Over-current lockout count | 00 05 = 5 | R/W | UINT16 | 2 | times |
| 0x1325 | Charge current-limit duration | 0B B8 = 300.0 s | R/W | UINT16 | 2 | 0.1 s |
| 0x1326 | Pulse current-limit current | 00 64 = 100 A | R/W | INT16 | 2 | A |
| 0x1327 | Pulse current-limit time | 00 0A = 1.0 s | R/W | UINT16 | 2 | 0.1 s |
| **Float charge and pre-charge** |  |  |  |  |  |  |
| 0x1328 | Float-charge lock voltage | 0D AC = 3.500 V | / | UINT16 | 2 | 0.001 V |
| 0x1329 | Float-charge release voltage | 0D 48 = 3.400 V | / | UINT16 | 2 | 0.001 V |
| 0x132A | Float-charge lock current | 05 DC = 1500 mA | / | INT16 | 2 | mA |
| 0x132B | Pre-charge completion ratio, short circuit | 00 64 = 10.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x132C | Pre-charge completion ratio, normal | 03 20 = 80.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x132D | Pre-charge completion ratio, abnormal | 00 C8 = 20.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x132E | Pre-charge timeout | 00 1E = 3.0 s | R/W | UINT16 | 2 | 0.1 s |
| **Charge temperature** |  |  |  |  |  |  |
| 0x132F | Charge high-temperature alarm recovery | 0C 81 = 320.1 K = 47.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1330 | Charge high-temperature alarm | 0C 9F = 323.1 K = 50.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1331 | Charge over-temperature recovery | 0C 9F = 323.1 K = 50.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1332 | Charge over-temperature protection | 0C D1 = 328.1 K = 55.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1333 | Charge low-temperature alarm recovery | 0A DD = 278.1 K = 5.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1334 | Charge low-temperature alarm | 0A BF = 275.1 K = 2.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1335 | Charge under-temperature recovery | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x1336 | Charge under-temperature protection | 0A 47 = 263.1 K = −10.0 °C | R/W | UINT16 | 2 | 0.1 K |
| **Discharge temperature** |  |  |  |  |  |  |
| 0x1337 | Discharge high-temperature alarm recovery | 0C 9F = 323.1 K = 50.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1338 | Discharge high-temperature alarm | 0C D1 = 328.1 K = 55.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1339 | Discharge over-temperature recovery | 0C D1 = 328.1 K = 55.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x133A | Discharge over-temperature protection | 0D 03 = 333.1 K = 60.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x133B | Discharge low-temperature alarm recovery | 0A C9 = 276.1 K = 3.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x133C | Discharge low-temperature alarm | 0A 47 = 263.1 K = −10.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x133D | Discharge under-temperature recovery | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x133E | Discharge under-temperature protection | 0A 15 = 258.1 K = −15.0 °C | R/W | UINT16 | 2 | 0.1 K |
| **Ambient temperature** |  |  |  |  |  |  |
| 0x133F | Ambient high-temperature alarm recovery | 0C 81 = 320.1 K = 47.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1340 | Ambient high-temperature alarm | 0C 9F = 323.1 K = 50.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1341 | Ambient over-temperature recovery | 0C D1 = 328.1 K = 55.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1342 | Ambient over-temperature protection | 0C D1 = 328.1 K = 55.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1343 | Ambient low-temperature alarm recovery | 0A C9 = 276.1 K = 3.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1344 | Ambient low-temperature alarm | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x1345 | Ambient under-temperature recovery | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x1346 | Ambient under-temperature protection | 0A 47 = 263.1 K = −10.0 °C | R/W | UINT16 | 2 | 0.1 K |
| **Power (MOSFET) temperature** |  |  |  |  |  |  |
| 0x1347 | Power high-temperature alarm recovery | 0D FD = 358.1 K = 85.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1348 | Power high-temperature alarm | 0E 61 = 368.1 K = 95.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x1349 | Power over-temperature recovery | 0D FD = 358.1 K = 85.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x134A | Power over-temperature protection | 0E F7 = 383.1 K = 110.0 °C | R/W | UINT16 | 2 | 0.1 K |
| **Heating and balancing** |  |  |  |  |  |  |
| 0x134B | Cell heating stop | 0B 0F = 283.1 K = 10.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x134C | Cell heating start | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x134D | Balancing disabled above (high temperature) | 0C 9F = 323.1 K = 50.0 °C | R/W | UINT16 | 2 | 0.1 K |
| 0x134E | Balancing disabled below (low temperature) | 0A BB = 273.1 K = 0 °C [sic: 0A BB is 274.7 K; 273.1 K would be 0A AB] | R/W | UINT16 | 2 | 0.1 K |
| 0x134F | Static balancing timer | 00 0A = 10 h | R/W | UINT16 | 2 | h |
| 0x1350 | Balancing start voltage | 0D 48 = 3.400 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1351 | Balancing start voltage difference | 00 32 = 0.050 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1352 | Balancing end voltage difference | 00 1E = 0.030 V | R/W | UINT16 | 2 | 0.001 V |
| **SOC and capacity** |  |  |  |  |  |  |
| 0x1353 | Top-up charge SOC | 03 C0 = 96.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x1354 | Low-SOC alarm recovery | 00 96 = 15.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x1355 | Low-SOC alarm | 00 64 = 10.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x1356 | Low-SOC protection recovery | 00 46 = 7.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x1357 | Low-SOC protection | 00 32 = 5.0 % | R/W | UINT16 | 2 | 0.1 % |
| 0x1358 | Rated capacity | 4E 20 = 200.00 Ah | R/W | UINT16 | 2 | 0.01 Ah |
| 0x1359 | Total (full) capacity | 4E 20 = 200.00 Ah | R/W | UINT16 | 2 | 0.01 Ah |
| 0x135A | Remaining capacity | 4E 20 = 200.00 Ah | R/W | UINT16 | 2 | 0.01 Ah |
| **Standby, forced output and compensation** |  |  |  |  |  |  |
| 0x135B | Standby sleep timer | 00 30 = 48 h | R/W | UINT16 | 2 | h |
| 0x135C | Forced output delay | 00 3C = 6.0 s | R/W | UINT16 | 2 | 0.1 s |
| 0x135D | Forced output interval | 00 F0 = 240 min | R/W | UINT16 | 2 | min |
| 0x135E | Forced output count | 00 0A = 10 | R/W | UINT16 | 2 | times |
| 0x135F | Compensation point 1 (cell position) | 00 07 = 7 | R/W | UINT16 | 2 | cell |
| 0x1360 | Compensation point 1 resistance | 00 00 = 0 mΩ | R/W | UINT16 | 2 | 0.1 mΩ |
| 0x1361 | Compensation point 2 (cell position) | 00 0D = 13 | R/W | UINT16 | 2 | cell |
| 0x1362 | Compensation point 2 resistance | 00 00 = 0 mΩ | R/W | UINT16 | 2 | 0.1 mΩ |
| 0x1363 | Cell voltage difference alarm | 01 F4 = 0.500 V | R/W | UINT16 | 2 | 0.001 V |
| 0x1364 | Cell voltage difference alarm recovery | 01 2C = 0.300 V | R/W | UINT16 | 2 | 0.001 V |
| **Requests to the inverter** |  |  |  |  |  |  |
| 0x1365 | Charge request voltage | 16 80 = 57.60 V | R/W | UINT16 | 2 | 0.01 V |
| 0x1366 | Charge request current | 00 50 = 80 A | R/W | INT16 | 2 | A |
| 0x1367 | Discharge request current | FF B0 = −80 A | R/W | INT16 | 2 | A |
| … | Reserved | … | … | … | … | … |

### Battery function parameters (SFA) · FC 0x01 read, 0x0F write (coils)

| Address | Name | R/W | Type | Bytes | Bits |
|---|---|---|---|---|---|
| 0x1400 | Voltage function switches | R/W | HEX | 1 | Table 4.11 |
| 0x1408 | Cell temperature function switches | R/W | HEX | 1 | Table 4.12 |
| 0x1410 | Ambient and power temperature function switches | R/W | HEX | 1 | Table 4.13 |
| 0x1418 | Function switches | R/W | HEX | 1 | Table 4.14 |
| 0x1420 | Current function switches 1 | R/W | HEX | 1 | Table 4.15 |
| 0x1428 | Current function switches 2 | R/W | HEX | 1 | Table 4.16 |
| 0x1430 | Capacity and other function switches | R/W | HEX | 1 | Table 4.17 |
| 0x1438 | Balancing function switches | R/W | HEX | 1 | Table 4.18 |
| 0x1440 | Indication function switches | R/W | HEX | 1 | Table 4.19 |
| 0x1448 | Failure detection function switches | R/W | HEX | 1 | Table 4.20 |
| … | Reserved | … | … | … | … |

### System control (SCA)

Caution. These registers switch the MOSFETs, shut down, reset and factory-reset the BMS. A monitoring tool should never write them.

| Address | Name | R/W | Type | Bytes | Value |
|---|---|---|---|---|---|
| 0x1500 | System calendar | R/W | 8 bytes | 8 | Table 4.21 |
| 0x1504 | History record timing | W | 18 bytes | 18 | Table 4.22 |
| 0x150D | Zero-current calibration | W | UINT16 | 2 | fixed 0x55AA |
| 0x150E | Current calibration | W | INT16 | 2 | 0.01 A |
| 0x150F | Cell voltage calibration | W | UINT16 | 2 | 0.001 V |
| 0x1510 | Discharge MOSFET control | W | UINT16 | 2 | 0x55AA off · 0xAA55 on |
| 0x1511 | Charge MOSFET off | W | UINT16 | 2 | fixed 0x55AA |
| 0x1512 | Current-limit MOSFET off | W | UINT16 | 2 | fixed 0x55AA |
| 0x1513 | Reserved | … | … | … | … |
| 0x1514 | Heater on | W | UINT16 | 2 | fixed 0xAA55 |
| 0x1515 | Charge MOSFET on | W | UINT16 | 2 | fixed 0xAA55 |
| 0x1516 | Clear (running) data | W | UINT16 | 2 | fixed 0x55AA |
| 0x1517 | System shutdown | W | UINT16 | 2 | fixed 0x55AA |
| 0x1518 | System reset | W | UINT16 | 2 | fixed 0x55AA |
| 0x1519 | Reserved | W | UINT16 | 2 | fixed 0x55AA |
| 0x151B | Restore factory settings | W | UINT16 | 2 | fixed 0x55AA |
| … | Reserved | … | … | … | … |

### History records (HIA)

Special notes from the document:

1. Reading history records is non-standard: the start address is always 0x7000.
1. A register count of 0x55AA requests the first history record; a register count of 0xAA55 requests the next one.

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| 0x1600 | Records remaining | / | R | UINT32 | 4 | / |
| 0x1602 | Date and time | / | R | 8 bytes | 8 | Table 4.21 |
| 0x1606 | System state · Voltage events | / | R | HEX | 1 + 1 | 4.1 · 4.2 |
| 0x1607 | Cell temperature events · Ambient and power temperature events | / | R | HEX | 1 + 1 | 4.3 · 4.4 |
| 0x1608 | Current events 1 · Current events 2 | / | R | HEX | 1 + 1 | 4.5 · 4.6 |
| 0x1609 | Remaining capacity events · FET state events | / | R | HEX | 1 + 1 | 4.7 · 4.8 |
| 0x160A | Balancing state events · Hardware failure events | / | R | HEX | 1 + 1 | 4.9 · 4.10 |
| 0x160B | Pack voltage | 14 A1 = 52.81 V | R | UINT16 | 2 | 0.01 V |
| 0x160C | Current | 05 FA = 15.30 A | R | INT16 | 2 | 0.01 A |
| 0x160D | Remaining capacity | 44 5C = 175.00 Ah | R | UINT16 | 2 | 0.01 Ah |
| 0x160E | Cell voltage 01 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x160F | Cell voltage 02 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1610 | Cell voltage 03 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1611 | Cell voltage 04 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1612 | Cell voltage 05 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x1613 | Cell voltage 06 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1614 | Cell voltage 07 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x1615 | Cell voltage 08 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x1616 | Cell voltage 09 | 0C E7 = 3.303 V | R | UINT16 | 2 | 0.001 V |
| 0x1617 | Cell voltage 10 | 0C E7 = 3.303 V | R | UINT16 | 2 | 0.001 V |
| 0x1618 | Cell voltage 11 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x1619 | Cell voltage 12 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x161A | Cell voltage 13 | 0C E5 = 3.301 V | R | UINT16 | 2 | 0.001 V |
| 0x161B | Cell voltage 14 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x161C | Cell voltage 15 | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x161D | Cell voltage 16 | 0C E4 = 3.300 V | R | UINT16 | 2 | 0.001 V |
| 0x161E | Cell temperature 01 | 0B 82 = 294.6 K = 21.5 °C | R | UINT16 | 2 | 0.1 K |
| 0x161F | Cell temperature 02 | 0B 7F = 294.3 K = 21.2 °C | R | UINT16 | 2 | 0.1 K |
| 0x1620 | Cell temperature 03 | 0B 81 = 294.5 K = 21.4 °C [original: 295.6 K] | R | UINT16 | 2 | 0.1 K |
| 0x1621 | Cell temperature 04 | 0B 7F = 294.3 K = 21.2 °C | R | UINT16 | 2 | 0.1 K |
| … | Reserved | … | … | … | … | … |
| 0x1626 | Ambient temperature | 0B 91 = 23.0 °C | R | UINT16 | 2 | 0.1 K |
| 0x1627 | Power temperature | 0B 83 = 21.6 °C | R | UINT16 | 2 | 0.1 K |

### Version information (VIA)

| Address | Name | R/W | Type | Bytes |
|---|---|---|---|---|
| 0x1700 | Manufacturer name | R | ASCII | 20 |
| 0x170A | Device name | R/W | ASCII | 20 |
| 0x1714 | Firmware version | R | ASCII | 2 |
| 0x1715 | BMS QR code information | R/W | ASCII | 30 |
| 0x1724 | Pack QR code information | R/W | ASCII | 30 |
| … | Reserved | … | … | … |

### Inverter information (PCT)

| Address | Name | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|
| 0x1800 | Inverter protocol selection | R/W | UINT16 | 2 | / |
| 0x1801 | Inverter link rate | R | UINT16 | 2 | kbps / bps |
| 0x1802 | Inverter name | R | ASCII | 32 | / |
| 0x1812 | Protocol name | R | ASCII | 32 | / |
| 0x1822 | Protocol version | R | ASCII | 2 | / |
| 0x1823 | Inverter protocol pre-fetch | R/W | UINT16 | 2 | / |
| … | Reserved | … | … | … | … |

### Network configuration parameters (SPB)

Examples show the raw bytes as sent: each 16-bit word has its two bytes swapped relative to the natural order (for example A8 C0 05 64 = 192.168.100.5).

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| 0x1900 | Server reporting period | 00 3C = 60 | R/W | UINT16 | 2 | s |
| 0x1901 | SNMP MAC address | 02 01 04 03 06 05 = 1 2 3 4 5 6 | R/W | Hex | 6 |  |
| 0x1904 | SNMP IPv4 address | A8 C0 05 64 = 192.168.100.5 | R/W | Hex | 4 |  |
| 0x1906 | SNMP subnet mask | FF FF 00 FF = 255.255.255.0 | R/W | Hex | 4 |  |
| 0x1908 | SNMP default gateway | A8 C0 01 64 = 192.168.100.1 | R/W | Hex | 4 |  |
| 0x190A | SNMP server | A8 C0 80 64 = 192.168.100.128 | R/W | Hex | 4 |  |
| 0x190C | SNMP DNS address | 04 04 04 04 = 4.4.4.4 | R/W | Hex | 4 |  |
| 0x190E | SNMP remote port | 17 70 = 6000 | R/W | UINT16 | 2 |  |
| 0x190F | SNMP local port | 13 88 = 5000 | R/W | UINT16 | 2 |  |
| 0x1910 | Tilt angle alarm | 00 C8 = 200 | R/W | UINT16 | 2 |  |
| 0x1911 | Tilt delay time | 00 32 = 50 | R/W | UINT16 | 2 |  |
| 0x1912 | Gyro timeout | 00 64 = 100 | R/W | UINT16 | 2 |  |
| 0x1913 | Server domain name | 65 74 65 6C = “tele…” | R/W | ASCII | 64 |  |
| 0x1933 | Wi-Fi name (SSID) | 20 20 00 20 = spaces, 0 | R/W | ASCII | 32 |  |
| 0x1943 | Wi-Fi password | 20 20 00 20 = spaces, 0 | R/W | ASCII | 32 |  |
| 0x1953 | Server port | 07 5B = 1883 | R/W | UINT16 | 2 |  |
| 0x1954 | MQTT key | 6F 67 66 30 = “go0f…” | R/W | ASCII | 32 |  |
| 0x1964 | MQTT secret | 77 35 65 75 = “5wue…” | R/W | ASCII | 32 |  |
| 0x1974 | Server address | / | R/W | Hex | 4 | reserved |
| … | Reserved | … | … | … | … | … |

### EMS system information A (EIA) · FC 0x04

Summary of all packs in parallel, served by the master pack at ID 0x00. 32-bit values are sent low word first (14 A4 00 00 = 0x000014A4 = 52.84 V).

| Address | Name | Example | R/W | Type | Bytes | Unit |
|---|---|---|---|---|---|---|
| 0x2000 | Pack voltage | 14 A4 00 00 = 52.84 V | R | UINT32 | 4 | 0.01 V |
| 0x2002 | Current (charge / discharge) | FF 9C FF FF = −10.0 A | R | INT32 | 4 | 0.1 A |
| 0x2004 | Remaining capacity | 05 8C 00 00 = 14.20 Ah | R | UINT32 | 4 | 0.01 Ah |
| 0x2006 | Total capacity | 27 10 00 00 = 100.00 Ah | R | UINT32 | 4 | 0.01 Ah |
| 0x2008 | Total discharged capacity | 00 03 00 00 = 30 Ah | R | UINT32 | 4 | 10 Ah |
| 0x200A | Total rated capacity | 27 10 00 00 = 100.00 Ah | R | UINT32 | 4 | 0.01 Ah |
| 0x200C | Parallel (online pack) flags | 00 01 00 00 = 1 | R | UINT32 | 4 | / |
| 0x200E | Protection flags | / | R | UINT32 | 4 | / |
| 0x2010 | Suggested maximum discharge current | 03 20 00 00 = 80.0 A | R | UINT32 | 4 | 0.1 A |
| 0x2012 | Suggested maximum charge current | 03 20 00 00 = 80.0 A | R | UINT32 | 4 | 0.1 A |
| 0x2014 | Suggested pack over-voltage value | 02 40 = 57.6 V | R | UINT16 | 2 | 0.1 V |
| 0x2015 | Suggested pack under-voltage value | 01 D0 = 46.4 V | R | UINT16 | 2 | 0.1 V |
| 0x2016 | Number of packs in parallel | 00 01 = 1 | R | UINT16 | 2 | / |
| 0x2017 | Average cycle count | 00 03 = 3 | R | UINT16 | 2 | / |
| 0x2018 | SOC | 00 8E = 14.2 % | R | UINT16 | 2 | 0.1 % |
| 0x2019 | SOH | 03 E8 = 100.0 % [original: “100.0AH”] | R | UINT16 | 2 | 0.1 % |
| … | Reserved | … | … | … | … | … |

### EMS system information B (EIB) · FC 0x04

| Address | Name | Example | R/W | Type | Bytes | Unit / note |
|---|---|---|---|---|---|---|
| 0x2100 | Highest cell voltage | 0C E7 = 3.303 V | R | UINT16 | 2 | 0.001 V |
| 0x2101 | Lowest cell voltage | 0C E6 = 3.302 V | R | UINT16 | 2 | 0.001 V |
| 0x2102 | Highest cell voltage position | 00 07 = 7 | R | UINT16 | 2 | cells counted from 0 |
| 0x2103 | Lowest cell voltage position | 00 00 = 0 | R | UINT16 | 2 | cells counted from 0 |
| 0x2104 | Highest pack voltage | 14 A4 = 52.84 V | R | UINT16 | 2 | 0.01 V |
| 0x2105 | Lowest pack voltage | 14 A4 = 52.84 V | R | UINT16 | 2 | 0.01 V |
| 0x2106 | Highest pack voltage position | 00 00 = 0 | R | UINT16 | 2 | packs counted from 0 |
| 0x2107 | Lowest pack voltage position | 00 00 = 0 | R | UINT16 | 2 | packs counted from 0 |
| 0x2108 | Highest cell temperature | 00 B3 = 17.9 °C | R | INT16 | 2 | 0.1 °C |
| 0x2109 | Lowest cell temperature | 00 AD = 17.3 °C | R | INT16 | 2 | 0.1 °C |
| 0x210A | Average cell temperature | 00 AF = 17.5 °C | R | INT16 | 2 | 0.1 °C |
| 0x210B | Highest cell temperature position | 00 03 = 3 | R | UINT16 | 2 | sensors counted from 0 |
| 0x210C | Lowest cell temperature position | 00 01 = 1 | R | UINT16 | 2 | sensors counted from 0 |
| 0x210D | Highest pack SOC | 00 8E = 14.2 % | R | UINT16 | 2 | 0.1 % |
| 0x210E | Lowest pack SOC | 00 8E = 14.2 % | R | UINT16 | 2 | 0.1 % |
| 0x210F | Highest pack cycle count | 00 00 = 0 | R | UINT16 | 2 | / |
| 0x2110 | Highest SOH | 03 E8 = 100.0 % | R | UINT16 | 2 | 0.1 % |
| 0x2111 | Highest power temperature | 00 A4 = 16.4 °C | R | INT16 | 2 | 0.1 °C |
| 0x2112 | Lowest power temperature | 00 A4 = 16.4 °C | R | INT16 | 2 | 0.1 °C |
| 0x2113 | Average power temperature | 00 A4 = 16.4 °C | R | INT16 | 2 | 0.1 °C |
| 0x2114 | Highest power temperature position | 00 00 = 0 | R | UINT16 | 2 | counted from 0 |
| 0x2115 | Lowest power temperature position | 00 00 = 0 | R | UINT16 | 2 | counted from 0 |
| … | Reserved | … | … | … | … | … |

### EMS system information C (EIC) · FC 0x01 (coils)

| Address | Name | R/W | Type | Bytes | Bits |
|---|---|---|---|---|---|
| 0x2200 | System state | R | HEX | 1 | Table 4.1 |
| 0x2208 | Voltage events | R | HEX | 1 | Table 4.2 |
| 0x2210 | Cell temperature events | R | HEX | 1 | Table 4.3 |
| 0x2218 | Ambient and power temperature events | R | HEX | 1 | Table 4.4 |
| 0x2220 | Current events 1 | R | HEX | 1 | Table 4.5 |
| 0x2228 | Current events 2 | R | HEX | 1 | Table 4.6 |
| 0x2230 | Remaining capacity events | R | HEX | 1 | Table 4.7 |
| 0x2238 | FET state events | R | HEX | 1 | Table 4.8 |
| 0x2240 | Balancing state events | R | HEX | 1 | Table 4.9 |
| 0x2248 | Hardware failure events | R | HEX | 1 | Table 4.10 |
| … | Reserved | … | … | … | … |

## 4. Appendix: bit tables

Event tables: 1 = present (entered), 0 = cleared. Switch tables: 1 = enabled, 0 = disabled.

#### 4.1 System state (1 enter, 0 exit)

| Bit | Meaning |
|---|---|
| 0 | Discharging |
| 1 | Charging |
| 2 | Float charge |
| 3 | Fully charged |
| 4 | Standby |
| 5 | Shut down |
| 6 | Reserved |
| 7 | Reserved |

#### 4.2 Voltage events (1 present)

| Bit | Meaning |
|---|---|
| 0 | Cell high-voltage alarm |
| 1 | Cell over-voltage protection |
| 2 | Cell low-voltage alarm |
| 3 | Cell under-voltage protection |
| 4 | Pack high-voltage alarm |
| 5 | Pack over-voltage protection |
| 6 | Pack low-voltage alarm |
| 7 | Pack under-voltage protection |

#### 4.3 Cell temperature events (1 present)

| Bit | Meaning |
|---|---|
| 0 | Charge high-temperature alarm |
| 1 | Charge over-temperature protection |
| 2 | Charge low-temperature alarm |
| 3 | Charge under-temperature protection |
| 4 | Discharge high-temperature alarm |
| 5 | Discharge over-temperature protection [original: “under-temperature”] |
| 6 | Discharge low-temperature alarm |
| 7 | Discharge under-temperature protection |

#### 4.4 Ambient and power temperature events (1 present)

| Bit | Meaning |
|---|---|
| 0 | Ambient high-temperature alarm |
| 1 | Ambient over-temperature protection |
| 2 | Ambient low-temperature alarm |
| 3 | Ambient under-temperature protection |
| 4 | Power high-temperature alarm |
| 5 | Power over-temperature protection |
| 6 | Cell low-temperature heating |
| 7 | Reserved |

#### 4.5 Current events 1 (1 present)

| Bit | Meaning |
|---|---|
| 0 | Charge over-current alarm |
| 1 | Charge over-current protection |
| 2 | Charge level 2 over-current protection |
| 3 | Discharge over-current alarm |
| 4 | Discharge over-current protection |
| 5 | Discharge level 2 over-current protection |
| 6 | Output short-circuit protection |
| 7 | Reserved |

#### 4.6 Current events 2 (1 present)

| Bit | Meaning |
|---|---|
| 0 | Output short-circuit lockout |
| 1 | Reverse-connection protection |
| 2 | Charge level 2 lockout |
| 3 | Discharge level 2 lockout |
| 4 | Reserved |
| 5 | Reserved |
| 6 | Reserved |
| 7 | Reserved |

#### 4.7 Remaining capacity events (1 present)

| Bit | Meaning |
|---|---|
| 0 | Reserved |
| 1 | Reserved |
| 2 | Low-SOC alarm |
| 3 | Low-SOC protection |
| 4 | Cell voltage difference alarm |
| 5 | Reserved |
| 6 | Gyro lockout |
| 7 | Reserved |

#### 4.8 Switch (FET) state events (1 on, 0 off)

| Bit | Meaning |
|---|---|
| 0 | Discharge switch on |
| 1 | Charge switch on |
| 2 | Current-limit switch on |
| 3 | Temperature regulation (heating) on |
| 4 | Reserved |
| 5 | Reserved |
| 6 | Reserved |
| 7 | Reserved |

#### 4.9 Balancing state events (1 present)

| Bit | Meaning |
|---|---|
| 0 | Balancing module on |
| 1 | Static balancing indication |
| 2 | Static balancing timeout |
| 3 | Balancing inhibited by over-temperature |
| 4 | Cell failure alarm |
| 5 | Reserved |
| 6 | Reserved |
| 7 | Reserved |

#### 4.10 Hardware failure events (1 present)

| Bit | Meaning |
|---|---|
| 0 | NTC failure |
| 1 | AFE failure |
| 2 | Charge MOSFET failure |
| 3 | Discharge MOSFET failure |
| 4 | Cell failure |
| 5 | Open-wire (disconnection) failure |
| 6 | Button failure |
| 7 | Aerosol (fire suppression) alarm |

#### 4.11 Voltage function switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Cell high-voltage alarm |
| 1 | Cell over-voltage protection |
| 2 | Cell low-voltage alarm |
| 3 | Cell under-voltage protection |
| 4 | Pack high-voltage alarm |
| 5 | Pack over-voltage protection |
| 6 | Pack low-voltage alarm |
| 7 | Pack under-voltage protection |

#### 4.12 Cell temperature function switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Charge high-temperature alarm |
| 1 | Charge over-temperature protection |
| 2 | Charge low-temperature alarm |
| 3 | Charge under-temperature protection |
| 4 | Discharge high-temperature alarm |
| 5 | Discharge over-temperature protection |
| 6 | Discharge low-temperature alarm |
| 7 | Discharge under-temperature protection |

#### 4.13 Ambient and power temperature switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Ambient high-temperature alarm |
| 1 | Ambient over-temperature protection |
| 2 | Ambient low-temperature alarm |
| 3 | Ambient under-temperature protection |
| 4 | Power high-temperature alarm |
| 5 | Power over-temperature protection |
| 6 | Cell low-temperature heating |
| 7 | Cell voltage failure |

#### 4.14 Function switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Bluetooth module |
| 1 | Cooling (fan) |
| 2 | Capacity LEDs always on |
| 3 | SNMP module |
| 4 | Wi-Fi module |
| 5 | DHCP |
| 6 | Gyro module |
| 7 | Reserved |

#### 4.15 Current function switches 1 (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Charge over-current alarm |
| 1 | Charge over-current protection |
| 2 | Charge level 2 over-current protection |
| 3 | Discharge over-current alarm |
| 4 | Discharge over-current protection |
| 5 | Discharge level 2 over-current protection |
| 6 | Output short-circuit protection |
| 7 | Reserved |

#### 4.16 Current function switches 2 (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Output short-circuit lockout |
| 1 | Reverse-connection protection |
| 2 | Charge level 2 over-current lockout |
| 3 | Discharge level 2 over-current lockout |
| 4 | Reserved |
| 5 | Reserved |
| 6 | Reserved |
| 7 | Reserved |

#### 4.17 Capacity and other switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Low-SOC alarm |
| 1 | Intermittent top-up charge |
| 2 | External switch control |
| 3 | Standby sleep |
| 4 | History recording |
| 5 | Low-SOC protection |
| 6 | Active current limiting |
| 7 | Passive current limiting |

#### 4.18 Balancing function switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Balancing function |
| 1 | Static balancing indication |
| 2 | Static balancing timeout |
| 3 | Balancing temperature limit |
| 4 | Reserved |
| 5 | Reserved |
| 6 | Reserved |
| 7 | Reserved |

#### 4.19 Indication function switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | Buzzer / indicator lights |
| 1 | LCD display |
| 2 | Manual forced output |
| 3 | Automatic forced output |
| 4 | Under-voltage recovery function |
| 5 | Aerosol test function |
| 6 | Aerosol disconnect mode |
| 7 | Current-sensor temperature compensation |

#### 4.20 Failure detection switches (1 enabled)

| Bit | Meaning |
|---|---|
| 0 | NTC failure (open or short circuit) |
| 1 | AFE failure (communication error) |
| 2 | Charge MOSFET failure (MOSFET short) |
| 3 | Discharge MOSFET failure (MOSFET short) |
| 4 | Cell failure (excessive voltage deviation) |
| 5 | Open-wire error |
| 6 | Button error |
| 7 | Aerosol alarm failure |

### 4.21 System calendar

| Byte | Field | Range | Type | Bytes | Unit |
|---|---|---|---|---|---|
| 0 | Year (low 8 bits) | 1 – 9999 | UINT16 | 2 | year |
| 1 | Year (high 8 bits) | 1 – 9999 | UINT16 | 2 | year |
| 2 | Month | 1 – 12 | UINT8 | 1 | month |
| 3 | Day | 1 – 31 | UINT8 | 1 | day |
| 4 | Hour | 0 – 23 | UINT8 | 1 | h |
| 5 | Minute | 0 – 59 | UINT8 | 1 | min |
| 6 | Second | 0 – 59 | UINT8 | 1 | s |
| 7 | Reserved | … | … | … | … |

### 4.22 History record timing

| Byte | Field | Type | Bytes | Unit |
|---|---|---|---|---|
| 0 | Start date | UINT8 | 1 | see 4.21 |
| 8 | End date | UINT8 | 1 | see 4.21 |
| 16 | Idle time (low 8 bits, high 8 bits) | UINT16 | 2 | s |

The original lists each date as UINT8 / 1 byte; by the byte offsets, each date is a full 8-byte calendar entry in the format of table 4.21.

## 5. Communication examples

This section was in English in the original. Comments in Chinese have been translated.

### Single pack data

**Get PIA**

```
00 04 10 00 00 12 75 16
Return data:
00      ADDR
04      CMD
24      Byte count
14 A1   Pack voltage                52.81 V
00 00   Current                     0.00 A
4E 20   Remaining capacity          200.00 Ah
4E 20   Total capacity              200.00 Ah
00 00   Total discharge capacity    (1 = 10 Ah)
03 E8   SOC                         100.0 %
03 E8   SOH                         100.0 %
00 00   Cycles                      0
0C E4   Average cell voltage        3.300 V
0B 80   Average cell temperature    2944 − 2731 = 21.3 °C
0C E6   Max cell voltage            3.302 V
0C E4   Min cell voltage            3.300 V
0B 82   Max cell temperature        2946 − 2731 = 21.5 °C
0B 7F   Min cell temperature        2943 − 2731 = 21.2 °C
00 00   Reserved
00 B4   Max discharge current       180 A
00 B4   Max charge current          180 A
03 E8   Reserved
DB F6   CRC
```

**Get PIB**

```
00 04 11 00 00 1A 75 2C
Return data:
00      ADDR
04      CMD
34      Byte count
0C E6   Cell 1 voltage    3.302 V
0C E4   Cell 2 voltage    3.300 V
0C E5   Cell 3 voltage    3.301 V
0C E4   Cell 4 voltage    3.300 V
0C E4   Cell 5 voltage    3.300 V
0C E5   Cell 6 voltage    3.301 V
0C E5   Cell 7 voltage    3.301 V
0C E4   Cell 8 voltage    3.300 V
0C E4   Cell 9 voltage    3.300 V
0C E4   Cell 10 voltage   3.300 V
0C E5   Cell 11 voltage   3.301 V
0C E5   Cell 12 voltage   3.301 V
0C E4   Cell 13 voltage   3.300 V
0C E5   Cell 14 voltage   3.301 V
0C E4   Cell 15 voltage   3.300 V
0C E4   Cell 16 voltage   3.300 V
0B 81   Cell temperature 1       2945 − 2731 = 21.4 °C
0B 82   Cell temperature 2       2946 − 2731 = 21.5 °C
0B 7F   Cell temperature 3       2943 − 2731 = 21.2 °C
0B 7F   Cell temperature 4       2943 − 2731 = 21.2 °C
0A AB   Reserved
0A AB   Reserved
0A AB   Reserved
0A AB   Reserved
0B 91   Ambient temperature      2961 − 2731 = 23.0 °C
0B 83   Power temperature        2947 − 2731 = 21.6 °C
34 DE   CRC
```

**Get PIC**

```
00 01 12 00 00 90 38 CF
Return data:
00      ADDR
01      CMD
12      Byte count
00      Cells 08–01 low-voltage alarm state     (8 bits, 1 on, 0 off)
00      Cells 16–09 low-voltage alarm state
00      Cells 08–01 high-voltage alarm state
00      Cells 16–09 high-voltage alarm state
00      Cell temperatures 08–01 low alarm state
00      Cell temperatures 08–01 high alarm state
00      Cells 08–01 balancing event code
00      Cells 16–09 balancing event code
10      System state code
00      Voltage event code
00      Cell temperature event code
00      Ambient and power temperature event code
00      Current event code 1
00      Current event code 2
00      Remaining capacity code
03      FET event code          0011: charge FET on, discharge FET on
00      Balancing state code
00      Hardware fault event code
6A 24   CRC
```

### EMS summary data

**Get EIA**

```
00 04 20 00 00 1A 7B D0
00            ADDR
04            CMD
34            Byte count
14 A4 00 00   Pack voltage              52.84 V
FF 9C FF FF   Current                   −10.0 A
05 8C 00 00   Remaining capacity        14.20 Ah
27 10 00 00   Total capacity            100.00 Ah
00 03 00 00   Total discharge capacity  30 Ah
27 10 00 00   Rated capacity            100.00 Ah
00 01 00 00   Online pack flags         0001 (1 pack in parallel)
00 00 00 00   Protected pack bits
03 20 00 00   Max discharge current     80.0 A
03 20 00 00   Max charge current        80.0 A
02 40         Suggested pack OV         57.6 V
01 D0         Suggested pack UV         46.4 V
00 01         System pack count         1
00 03         Cycles                    3
00 8E         SOC                       14.2 %
03 E8         SOH                       100.0 %
9D F2         CRC
```

**Get EIB**

```
00 04 21 00 00 16 7A 29
00      ADDR
04      CMD
2C      Byte count
0C E7   Max cell voltage          3.303 V
0C E6   Min cell voltage          3.302 V
00 07   Max cell voltage ID       7   (pack 1, cell 8)
00 00   Min cell voltage ID       0   (pack 1, cell 1)
14 A4   Max pack voltage          52.84 V
14 A4   Min pack voltage          52.84 V
00 00   Max pack voltage ID
00 00   Min pack voltage ID
00 B3   Max cell temperature      17.9 °C
00 AD   Min cell temperature      17.3 °C
00 AF   Avg cell temperature      17.5 °C
00 03   Max cell temperature ID   3   (pack 1, sensor 4)
00 01   Min cell temperature ID   1   (pack 1, sensor 2)
00 8E   Max pack SOC              14.2 %
00 8E   Min pack SOC              14.2 %
00 00   Max pack cycles           0
03 E8   Max pack SOH              100.0 %
00 A4   Max pack power temp       16.4 °C
00 A4   Min pack power temp       16.4 °C
00 A4   Avg pack power temp       16.4 °C
00 00   Max pack power temp ID    0   (pack 1)
00 00   Min pack power temp ID    0   (pack 1)
01 07   CRC
```

**Get EIC**

```
00 01 22 00 00 50 37 9F
00      ADDR
01      CMD
0A      Byte count
01      System state code        0001 0000: standby [byte shown as 01; the bit pattern means 0x10]
00      Voltage event code
00      Cell temperature event code
00      Ambient and power temperature event code
00      Current event code 1
00      Current event code 2
00      Remaining capacity code
03      FET event code           0011: charge FET on, discharge FET on
00      Balancing state code
00      Hardware fault event code
7E 35   CRC
```

Unofficial community translation of “上海鑫智恒 Modbus_RTU 通信协议 (中文版)” V0.3, Shanghai Xinzhiheng Electronic Technology Co., Ltd (Seplos), www.xzhpower.com. Translated September 2026. The register data is reproduced as published; always check values against your own BMS before writing any register.
