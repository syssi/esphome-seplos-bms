# XZH BMS Modbus RTU Protocol

**Port Support:** RS485
**Hardware BMS:** BMS48100/48200
**Version:** V0.1
**Date:** 2023/02/09

---

## Revision History

| Index | Description | Version | Date | Author |
|-------|-------------|---------|------|--------|
| 0 | Document created | V0.1 | 2023-02-09 | |
| 1-10 | | | | |

---

## 1. Communication Parameters

### 1.1 Configuration:
- **Baud Rate:** 19200
- **Parity bit:** No
- **Data Bits:** 8
- **Stop Bit:** 1

### 1.2 Port features:
- **RS485:** BMS response which is self address only.

---

## 2. Frame format of communication data

### 2.1.1 List of function code supported:

| Function code | Meaning | Notes |
|---------------|---------|-------|
| 0X01 | Read Coil status | Supported data block PIC/SFA/EIC |
| 0X0F | Write Coil status | |
| 0X04 | Read command | Supported data block PIA/PIB/SPA/SCA/HIA/VIA/EIA/EIB/PCT |
| 0X10 | Write command | |

### 2.1.2 Device supported:

| Device Name | Device Id | Supported data block |
|-------------|-----------|---------------------|
| BMS | 0X00~0X7F | PIA/PIB/PIC/SPA/SCA/HIA/VIA/SFA |
| EMS | 0XB0~0XBF | |
| ECU | 0XC0 | EIA/EIB/EIC |
| 2.4'or 5'or7' TFT/LCD | 0XE0 | PIA/PIB/PIC/SCA |
| Bluetooth | 0XE0/0X00~0X10/0XC0 | PIA/PIB/PIC/EIA/EIB/EIC/SCA/PCT |

### 2.2 0X04 Command

#### 2.2.1 Host node sending

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|------|---|---|---|---|---|---|---|---|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | LSB | MSB |
| Explanation | BMS address | Type of command(0x04) | Beginning register address | Resister number n | CRC |

#### 2.2.2 Slave node Normal response

| Item | 0 | 1 | 2 | 3\|4… | 3+2n | 4+2n |
|------|---|---|---|-------|------|------|
| Field definition | ADDR | CMD | Length | … | LSB | MSB |
| Explanation | BMS address | Type of command | 2n | register value… | CRC |

### 2.3 0X10 Command

#### 2.3.1 Host node sending

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7\|8… | 7+2n | 8+2n |
|------|---|---|---|---|---|---|---|-------|------|------|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | Length | … | LSB | MSB |
| Explanation | BMS address | Type of command (0x10) | Beginning register address | Resister number n | 2n | Resister Value | … | CRC |

#### 2.3.2 Slave node Normal response

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|------|---|---|---|---|---|---|---|---|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | LSB | MSB |
| Explanation | BMS address | Type of command | Beginning register address | Resister number n | CRC |

### 2.4 0X01 Command

#### 2.4.1 Host node sending

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|------|---|---|---|---|---|---|---|---|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | LSB | MSB |
| Explanation | BMS address | Type of command(0x01) | Beginning coil address | Bits number n | CRC |

#### 2.4.2 Slave node Normal response

| Item | 0 | 1 | 2 | 3… | 4+N | 5+N |
|------|---|---|---|-----|-----|-----|
| Field definition | ADDR | CMD | Length | … | LSB | MSB |
| Explanation | BMS address | Type of command | Bytes length N | Coil value… | CRC |

**Bytes length N:** The request is the number of bits and the reply is the number of bytes. Fill in 0 for the extra part

### 2.5 0X0F Command

#### 2.5.1 Host node sending

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8+N | 9+N |
|------|---|---|---|---|---|---|---|---|-----|-----|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | Length | … | LSB | MSB |
| Explanation | BMS address | Type of command (0x0F) | Beginning coil address | Bits number n | Bytes number N | Coil Value | … | CRC |

#### 2.5.2 Slave node Normal response

| Item | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|------|---|---|---|---|---|---|---|---|
| Field definition | ADDR | CMD | MSB | LSB | MSB | LSB | LSB | MSB |
| Explanation | BMS address | Type of command | Beginning coil address | Bits number n | CRC |

### 2.6 Error Code

#### 2.6.1 Abnormal response of format from slave node

| Item | 0 | 1 | 2 | 3 | 4 |
|------|---|---|---|---|---|
| Field definition | ADDR | CMD+128 | Err Code | LSB | MSB |
| Explanation | Controller address | Type of command +128 | Error Code | CRC parity |

#### 2.6.2 Error code defined

| Error Code | Defined | Notes |
|------------|---------|-------|
| 0x01 | illegal function | Function that does not supported |
| 0x02 | Illegal data address | Register address that does not supported |
| 0x03 | Illegal data value | Data value is not allowed |
| 0x04 | Salve device failure | Salve node fault |
| 0x05 | Acknowledge | Need master waiting |
| 0x06 | Slave device busy | |
| 0x08 | Memory parity error | |
| 0x0A | Gateway path unavailable | |
| 0x0B | Gateway target device failed to respond | |
| 0x81 | No history record | |
| Others | Reservation | |

---

## 3. Data information

### TA01:

#### Pack Info. A (PIA)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1000 | Pack Voltage | R | UINT16 | 2 | 10mV |
| 1001 | Current | R | INT16 | 2 | 10mA |
| 1002 | Remaining capacity | R | UINT16 | 2 | 10mAH |
| 1003 | Total Capacity | R | UINT16 | 2 | 10mAH |
| 1004 | Total Discharge Capacity | R | UINT16 | 2 | 10AH |
| 1005 | SOC | R | UINT16 | 2 | 0.1% |
| 1006 | SOH | R | UINT16 | 2 | 0.1% |
| 1007 | Cycle | R | UINT16 | 2 | 1 |
| 1008 | Averag of Cell Votage | R | UINT16 | 2 | 1mV |
| 1009 | Averag of Cell Temperature | R | UINT16 | 2 | 0.1K |
| 100A | Max Cell Voltage | R | UINT16 | 2 | 1mV |
| 100B | Min Cell Voltage | R | UINT16 | 2 | 1mV |
| 100C | Max Cell Temperature | R | UINT16 | 2 | 0.1K |
| 100D | Min Cell Temperature | R | UINT16 | 2 | 0.1K |
| 100E | reserve | | | | |
| 100F | MaxDisCurt | R | UINT16 | 2 | 1A |
| 1010 | MaxChgCurt | R | UINT16 | 2 | 1A |

#### Pack Info. B (PIB)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1100 | Cell1 Voltage | R | UINT16 | 2 | 1mV |
| 1101 | Cell2 Voltage | R | UINT16 | 2 | 1mV |
| 1102 | Cell3 Voltage | R | UINT16 | 2 | 1mV |
| 1103 | Cell4 Voltage | R | UINT16 | 2 | 1mV |
| 1104 | Cell5 Voltage | R | UINT16 | 2 | 1mV |
| 1105 | Cell6 Voltage | R | UINT16 | 2 | 1mV |
| 1106 | Cell7 Voltage | R | UINT16 | 2 | 1mV |
| 1107 | Cell8 Voltage | R | UINT16 | 2 | 1mV |
| 1108 | Cell9 Voltage | R | UINT16 | 2 | 1mV |
| 1109 | Cell10 Voltage | R | UINT16 | 2 | 1mV |
| 110A | Cell11 Voltage | R | UINT16 | 2 | 1mV |
| 110B | Cell12 Voltage | R | UINT16 | 2 | 1mV |
| 110C | Cell13 Voltage | R | UINT16 | 2 | 1mV |
| 110D | Cell14 Voltage | R | UINT16 | 2 | 1mV |
| 110E | Cell15 Voltage | R | UINT16 | 2 | 1mV |
| 110F | Cell16 Voltage | R | UINT16 | 2 | 1mV |
| 1110 | Cell temperature 1 | R | UINT16 | 2 | 0.1K |
| 1111 | Cell temperature 2 | R | UINT16 | 2 | 0.1K |
| 1112 | Cell temperature 3 | R | UINT16 | 2 | 0.1K |
| 1113 | Cell temperature 4 | R | UINT16 | 2 | 0.1K |
| …… | reserve | | | | |
| 1118 | Environment Temperature | R | UINT16 | 2 | 0.1K |
| 1119 | Power temperature | R | UINT16 | 2 | 0.1K |

#### Pack Info. C (PIC)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1200 | Cells voltage 08-01low alarm state | R | HEX | 1 | 1：alarm |
| 1208 | Cells voltage 16-09low alarm state | R | HEX | 1 | 1：alarm |
| 1210 | Cells voltage 08-01high alarm state | R | HEX | 1 | 1：alarm |
| 1218 | Cells voltage 16-09 high alarm state | R | HEX | 1 | 1：alarm |
| 1220 | Cell 08-01 temperature Tlow alarm state | R | HEX | 1 | 1：alarm |
| 1228 | Cell 08-01 temperature high alarm state | R | HEX | 1 | 1：alarm |
| 1230 | Cell 08-01 equalization event code | R | HEX | 1 | 1:on 0:off |
| 1238 | Cell 16-09 equalization event code | R | HEX | 1 | 1:on 0:off |
| 1240 | System state code | R | HEX | 1 | See TB09 |
| 1248 | Voltage event code | R | HEX | 1 | See TB02 |
| 1250 | Cells Temperature event code | R | HEX | 1 | See TB03 |
| 1258 | Environment and power Temperature event code | R | HEX | 1 | See TB04 |
| 1260 | Current event code1 | R | HEX | 1 | See TB05 |
| 1268 | Current event code2 | R | HEX | 1 | See TB16 |
| 1270 | The residual capacity code | R | HEX | 1 | See TB06 |
| 1278 | The FET event code | R | HEX | 1 | See TB07 |
| 1280 | battery equalization state code | R | HEX | 1 | See TB08 |
| 1288 | Hard fault event code | R | HEX | 1 | See TB15 |

#### System Parameter (SPA)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1300 | Ntc number | R | UINT16 | 2 | ---- |
| 1301 | Cell number serial battery | R/W | UINT16 | 2 | ---- |
| 1302 | Battery high voltage recover | R/W | UINT16 | 2 | 10mV |
| 1303 | Battery High voltage alarm | R/W | UINT16 | 2 | 10mV |
| 1304 | Battery over voltage recover | R/W | UINT16 | 2 | 10mV |
| 1305 | Battery over voltage protection | R/W | UINT16 | 2 | 10mV |
| 1306 | Battery low voltage Recover | R/W | UINT16 | 2 | 10mV |
| 1307 | Battery low voltage alarm | R/W | UINT16 | 2 | 10mV |
| 1308 | Battery under voltage Recover | R/W | UINT16 | 2 | 10mV |
| 1309 | Battery under voltage protection | R/W | UINT16 | 2 | 10mV |
| 130A | Cell high voltage recover | R/W | UINT16 | 2 | 1mV |
| 130B | Cell high voltage alarm | R/W | UINT16 | 2 | 1mV |
| 130C | Cell over voltage recover | R/W | UINT16 | 2 | 1mV |
| 130D | Cell over voltage protection | R/W | UINT16 | 2 | 1mV |
| 130E | Cell low voltage Recover | R/W | UINT16 | 2 | 1mV |
| 130F | Cell low voltage alarm | R/W | UINT16 | 2 | 1mV |
| 1310 | Cell under voltage Recover | R/W | UINT16 | 2 | 1mV |
| 1311 | Cell under voltage protection | R/W | UINT16 | 2 | 1mV |
| 1312 | Cell under voltage Fault | R/W | UINT16 | 2 | 1mV |
| 1313 | Cell Diff protection | R/W | UINT16 | 2 | 1mV |
| 1314 | Secondary Charge current protection | R/W | INT16 | 2 | 1mV |
| 1315 | Charge high current recover | R/W | UINT16 | 2 | A |
| 1316 | Charge high current alarm | R/W | INT16 | 2 | A |
| 1317 | Charge over current protection | R/W | UINT16 | 2 | A |
| 1318 | Charge over current time delay | R/W | INT16 | 2 | 0.1s |
| 1319 | Secondary Charge current protection | R/W | INT16 | 2 | A |
| 131A | Secondary Charge current time dela | R/W | INT16 | 2 | ms |
| 131B | Discharge low current recover | R/W | INT16 | 2 | A |
| 131C | Discharge low current alarm | R/W | INT16 | 2 | A |
| 131D | Discharge over current protection | R/W | UINT16 | 2 | A |
| 131E | Discharge over current time delay | R/W | INT16 | 2 | 0.1s |
| 131F | Secondary discharge current protection | R/W | UINT16 | 2 | A |
| 1320 | Secondary discharge current time delay | R/W | INT16 | 2 | ms |
| 1321 | Output shortcut protection | R/W | UINT16 | 2 | A |
| 1322 | Output shortcut time delay | R/W | UINT16 | 2 | us |
| 1323 | Over current recover time delay | R/W | UINT16 | 2 | 0.1s |
| 1324 | Over current lock times | R/W | INT16 | 2 | times |
| 1325 | Charge High switch Limited time | R/W | UINT16 | 2 | 0.1s |
| 1326 | Pluse current | R/W | UINT16 | 2 | A |
| 1327 | Pluse time | R/W | UINT16 | 2 | 0.1s |
| …… | reserve | | | | |
| 132B | Precharge Short Percent | R/W | UINT16 | 2 | 0.1% |
| 132C | Precharge Stop Percent | R/W | UINT16 | 2 | 0.1% |
| 132D | Precharge Fault Percent | R/W | UINT16 | 2 | 0.1% |
| 132E | Precharge Over Time | R/W | UINT16 | 2 | s |
| 132F | Charge high temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1330 | Charge high temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1331 | Charge over temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1332 | Charge over temperature protection | R/W | UINT16 | 2 | 0.1K |
| 1333 | Charge low temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1334 | Charge low temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1335 | Charge under temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1336 | Charge under temperature protection | R/W | UINT16 | 2 | 0.1K |
| 1337 | Discharge high temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1338 | Discharge high temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1339 | Discharge over temperature recover | R/W | UINT16 | 2 | 0.1K |
| 133A | Discharge over temperature protection | R/W | UINT16 | 2 | 0.1K |
| 133B | Discharge low temperature recover | R/W | UINT16 | 2 | 0.1K |
| 133C | Discharge low temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 133D | Discharge under temperature recover | R/W | UINT16 | 2 | 0.1K |
| 133E | Discharge under temperature protection | R/W | UINT16 | 2 | 0.1K |
| 133F | High environment temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1340 | High environment temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1341 | Over environment temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1342 | Over environment temperature protection | R/W | UINT16 | 2 | 0.1K |
| 1343 | Low environment temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1344 | Low environment temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1345 | Under environment temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1346 | Under environment temperature protection | R/W | UINT16 | 2 | 0.1K |
| 1347 | High power temperature recover | R/W | UINT16 | 2 | 0.1K |
| 1348 | High power temperature alarm | R/W | UINT16 | 2 | 0.1K |
| 1349 | Over power temperature recover | R/W | UINT16 | 2 | 0.1K |
| 134A | Over power temperature protection | R/W | UINT16 | 2 | 0.1K |
| 134B | Cell heating stop | R/W | UINT16 | 2 | 0.1K |
| 134C | Cell heating open | R/W | UINT16 | 2 | 0.1K |
| 134D | Equalization high temperature prohibit | R/W | UINT16 | 2 | 0.1K |
| 134E | Equalization low temperature prohibit | R/W | UINT16 | 2 | 0.1K |
| 134F | static equilibrium time | R/W | UINT16 | 2 | H |
| 1350 | Equalization open voltage | R/W | UINT16 | 2 | mv |
| 1351 | Equalization open voltage difference | R/W | UINT16 | 2 | mv |
| 1352 | Equalization stop voltage difference | R/W | UINT16 | 2 | mv |
| 1353 | SOC Full Relese | R/W | UINT16 | 2 | 0.1% |
| 1354 | SOC low recover | R/W | UINT16 | 2 | 0.1% |
| 1355 | SOC low Alarm | R/W | UINT16 | 2 | 0.1% |
| 1356 | SOC Under recover | R/W | UINT16 | 2 | 0.1% |
| 1357 | SOC Under protection | R/W | UINT16 | 2 | 0.1% |
| 1358 | Battery rated capacity | R/W | UINT16 | 2 | 10mAH |
| 1359 | Battery total capacity | R/W | UINT16 | 2 | 10mAH |
| 135A | Residual capacity | R/W | UINT16 | 2 | 10mAH |
| 135B | Stand-by to sleep time | R/W | UINT16 | 2 | H |
| 135C | Focs Output Delay time | R/W | UINT16 | 2 | 0.1s |
| 135D | Focs Output Splite | R/W | UINT16 | 2 | Min |
| 135E | Pcs Output Timers | R/W | UINT16 | 2 | times |
| 135F | Compensating Position 1 | R/W | UINT16 | 2 | Cell |
| 1360 | Position 1 Resistance | R/W | UINT16 | 2 | mΩ |
| 1361 | Compensating Position 2 | R/W | UINT16 | 2 | Cell |
| 1362 | Position 2 Resistance | R/W | UINT16 | 2 | mΩ |
| 1363 | Cell Diff alarm | R/W | UINT16 | 2 | mv |
| 1364 | Diff alarm recover | R/W | UINT16 | 2 | mv |
| 1365 | PCS Request Charge Limit Voltage | R/W | UINT16 | 2 | 10mv |
| 1366 | PCS Request Charge Limit Current | R/W | UINT16 | 2 | A |
| 1367 | PCS Request Discharge Limit Current | R/W | UINT16 | 2 | A |

#### System Function (SFA)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1400 | Voltage function switch | R/W | HEX | 1 | See TB02 |
| 1408 | Cell Temperature function switch | R/W | HEX | 1 | See TB03 |
| 1410 | Environment and power Temperature function switch | R/W | HEX | 1 | See TB10 |
| 1418 | function switch | R/W | HEX | 1 | See TB17 |
| 1420 | Current function switch 1 | R/W | HEX | 1 | See TB05 |
| 1428 | Current function switch 2 | R/W | HEX | 1 | See TB11 |
| 1430 | Capacity and other function switch | R/W | HEX | 1 | See TB08 |
| 1438 | Equalization function switch | R/W | HEX | 1 | See TB12 |
| 1440 | Indicator function switch | R/W | HEX | 1 | See TB18 |
| 1448 | Hard fault function switch | R/W | HEX | 1 | See TB15 |

#### System Ctrol (SCA)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1500 | System Date | R/W | 8Btyes | 8 | See TB13 |
| 1504 | TIMING History | W | 18 Btyes | 18 | See TB14 |
| 150D | Calibration Zero | W | UINT16 | 2 | Fixed：55AA |
| 150E | Calibration Current | W | INT16 | 2 | 10mA |
| 150F | Calibration Voltage | W | UINT16 | 2 | 1mV |
| 1510 | Discharing FETs Control Off | W | UINT16 | 2 | Ack:55AA Nack:Oters |
| 1511 | Charing FETs Control Off | W | UINT16 | 2 | Ack:55AA Nack:Oters |
| 1512 | Current Limit FETs Control Off | W | UINT16 | 2 | Ack:55AA Nack:Oters |
| …… | reserve | | | | |
| 1514 | Heater FETs Control On | W | UINT16 | 2 | Ack:AA55 Nack:Oters |
| 1515 | Charing FETs Control On | W | UINT16 | 2 | Ack:AA55 Nack:Oters |
| 1516 | Parameter Reset | W | UINT16 | 2 | Fixed 55AA |
| 1517 | System Power Off | W | UINT16 | 2 | Fixed 55AA |
| 1518 | System Reset | W | UINT16 | 2 | Fixed 55AA |
| 1519 | Boot request | W | UINT16 | 2 | Fixed 55AA |

#### History Info (HIA)

**PS:** The request to obtain historical data uses non-standard, and the start register is fixed to 0X7000; Request the first history record when the number of registers is 55AA; The next history record is requested when the number of registers is AA55.

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1600 | Remaining record No. | R | UINT32 | 4 | --- |
| 1602 | Record Date | R | 8Btyes | 8 | See TB13 |
| 1606 | System state code | R | HEX | 1 | See TB09 |
| | Voltage event code | R | HEX | 1 | See TB02 |
| 1607 | Cells Temperature event code | R | HEX | 1 | See TB03 |
| | Environment and power Temperature event code | R | HEX | 1 | See TB04 |
| 1608 | Current event code 1 | R | HEX | 1 | See TB05 |
| | Current event code 2 | R | HEX | 1 | See TB16 |
| 1609 | The residual capacity code | R | HEX | 1 | See TB06 |
| | The FET event code | R | HEX | 1 | See TB07 |
| 160A | Battery equalization state code | R | HEX | 1 | See TB08 |
| | Hard failt event code | R | HEX | 1 | See TB15 |
| 160B | Pack Voltage | R | UINT16 | 2 | 10mV |
| 160C | Current | R | INT16 | 2 | 10mA |
| 160D | Remaining capacity | R | UINT16 | 2 | 10mHA |
| 160E | Cell1 Voltage | R | UINT16 | 2 | 1mV |
| 160F | Cell2 Voltage | R | UINT16 | 2 | 1mV |
| 1610 | Cell3 Voltage | R | UINT16 | 2 | 1mV |
| 1611 | Cell4 Voltage | R | UINT16 | 2 | 1mV |
| 1612 | Cell5 Voltage | R | UINT16 | 2 | 1mV |
| 1613 | Cell6 Voltage | R | UINT16 | 2 | 1mV |
| 1614 | Cell7 Voltage | R | UINT16 | 2 | 1mV |
| 1615 | Cell8 Voltage | R | UINT16 | 2 | 1mV |
| 1616 | Cell9 Voltage | R | UINT16 | 2 | 1mV |
| 1617 | Cell10 Voltage | R | UINT16 | 2 | 1mV |
| 1618 | Cell11 Voltage | R | UINT16 | 2 | 1mV |
| 1619 | Cell12 Voltage | R | UINT16 | 2 | 1mV |
| 161A | Cell13 Voltage | R | UINT16 | 2 | 1mV |
| 161B | Cell14 Voltage | R | UINT16 | 2 | 1mV |
| 161C | Cell15 Voltage | R | UINT16 | 2 | 1mV |
| 161D | Cell16 Voltage | R | UINT16 | 2 | 1mV |
| 161E | Cell temperature 1 | R | UINT16 | 2 | 0.1K |
| 161F | Cell temperature 2 | R | UINT16 | 2 | 0.1K |
| 1620 | Cell temperature 3 | R | UINT16 | 2 | 0.1K |
| 1621 | Cell temperature 4 | R | UINT16 | 2 | 0.1K |
| …… | reserve | | | | |
| 1626 | Environment temperature | R | UINT16 | 2 | 0.1K |
| 1627 | Power temperature | R | UINT16 | 2 | 0.1K |

#### Version Info (VIA)

**PS:** Fill in the small end first.

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1700 | Factory Names | R | ASCII | 20 | |
| 170A | Device Names | R/W | ASCII | 20 | |
| 1714 | Firmware Version | R | ASCII | 2 | |
| 1715 | Bms SN | R/W | ASCII | 30 | |
| 1724 | Pack SN | R/W | ASCII | 30 | |

#### PCS Control (PCT)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 1800 | PCS Protocol type Switch | R/W | UINT16 | 2 | |
| 1801 | PCS baud rate | R | UINT16 | 2 | Kbps/bps |
| 1802 | PCS name | R | ASCII | 32 | |
| 1812 | Protocol support name | R | ASCII | 32 | |
| 1822 | Protocol version | R | ASCII | 2 | |
| 1823 | PCS Protocol pre Switch | R/W | UINT16 | 2 | |

#### EMS Info.A (EIA)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 2000 | Pack Voltage | R | UINT32 | 4 | 10mV |
| 2002 | Current | R | INT32 | 4 | 100mA |
| 2004 | Remaining capacity | R | UINT32 | 4 | 10mAH |
| 2006 | Total Capacity | R | UINT32 | 4 | 10mAH |
| 2008 | Total Discharge Capacity | R | UINT32 | 4 | 10AH |
| 200A | Rated Capacity | R | UINT32 | 4 | 10mAH |
| 200C | Online Pack Flag | R | UINT32 | 4 | |
| 200E | Protected Pack bit | R | UINT32 | 4 | |
| 2010 | Max Discharge current | R | UINT32 | 4 | 100mA |
| 2012 | Max Charge current | R | UINT32 | 4 | 100mA |
| 2014 | Suggest Pack OV | R | UINT16 | 2 | 100mV |
| 2015 | Suggest Pack UV | R | UINT16 | 2 | 100mV |
| 2016 | System Pack No. | R | UINT16 | 2 | |
| 2017 | Cycle | R | UINT16 | 2 | |
| 2018 | Soc | R | UINT16 | 2 | 0.1% |
| 2019 | Soh | R | UINT16 | 2 | 0.1% |

#### EMS Info. B (EIB)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 2100 | Max Cell Voltage | R | UINT16 | 2 | 1mV |
| 2101 | Min Cell Voltage | R | UINT16 | 2 | 1mV |
| 2102 | Max Cell Voltage Id | R | UINT16 | 2 | |
| 2103 | Min Cell Voltage Id | R | UINT16 | 2 | |
| 2104 | Max Pack Voltage | R | UINT16 | 2 | 10mV |
| 2105 | Min Pack Voltage | R | UINT16 | 2 | 10mV |
| 2106 | Max Pack Voltage Id | R | UINT16 | 2 | |
| 2107 | Min Pack Voltage Id | R | UINT16 | 2 | |
| 2108 | Max Cell Temperature | R | INT16 | 2 | 1℃ |
| 2109 | Min Cell Temperature | R | INT16 | 2 | 1℃ |
| 210A | Avg Cell Temperature | R | INT16 | 2 | 1℃ |
| 210B | Max Cell Temperature Id | R | UINT16 | 2 | |
| 210C | Min Cell Temperature Id | R | UINT16 | 2 | |
| 210D | Max Pack Power temperature | R | INT16 | 2 | 1℃ |
| 210E | Min Pack Power temperature | R | INT16 | 2 | 1℃ |
| 210F | Avg Pack Power temperature | R | INT16 | 2 | 1℃ |
| 2110 | Max Pack Power temperature Id | R | INT16 | 2 | |
| 2111 | Min Pack Power temperature Id | R | INT16 | 2 | |
| 2112 | Max Pack Soc | R | UINT16 | 2 | 0.1% |
| 2113 | Min Pack Soc | R | UINT16 | 2 | 0.1% |
| 2114 | Max Pack Cycle | R | UINT16 | 2 | |
| 2115 | Max Pack Soh | R | UINT16 | 2 | 0.1% |

#### EMS Info. C (EIC)

| Relative Address | Name | R/W | Data type | Bytes | Unit |
|------------------|------|-----|-----------|-------|------|
| 2200 | System state code | R | HEX | 1 | See TB09 |
| 2208 | Voltage event code | R | HEX | 1 | See TB02 |
| 2210 | Cells Temperature event code | R | HEX | 1 | See TB03 |
| 2218 | Environment and power Temperature event code | R | HEX | 1 | See TB04 |
| 2220 | Current event code1 | R | HEX | 1 | See TB05 |
| 2228 | Current event code2 | R | HEX | 1 | See TB16 |
| 2230 | The residual capacity code | R | HEX | 1 | See TB06 |
| 2238 | The FET event code | R | HEX | 1 | See TB07 |
| 2240 | battery equalization state code | R | HEX | 1 | See TB08 |
| 2248 | Hard fault event code | R | HEX | 1 | See TB15 |

---

## Status Code Tables

### TB02: Voltage Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Cell high voltage alarm |
| Bit1 | Cell over voltage protection |
| Bit2 | Cell low voltage alarm |
| Bit3 | Cell under voltage protection |
| Bit4 | Pack high voltage alarm |
| Bit5 | Pack over voltage protection |
| Bit6 | Pack low voltage alarm |
| Bit7 | Pack under voltage protection |

### TB03: Temperature Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Charge high temperature alarm |
| Bit1 | Charge over temperature protection |
| Bit2 | Charge low temperature alarm |
| Bit3 | Charge under temperature protection |
| Bit4 | Discharge high temperature alarm |
| Bit5 | Discharge over temperature protection |
| Bit6 | Discharge low temperature alarm |
| Bit7 | Discharge under temperature protection |

### TB04: Environment Temperature Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | High environment temperature alarm |
| Bit1 | Over environment temperature protection |
| Bit2 | Low environment temperature alarm |
| Bit3 | Under environment temperature protection |
| Bit4 | High Power temperature alarm |
| Bit5 | Over Power temperature protection |
| Bit6 | Cell temperature low heating |
| Bit7 | Reservation |

### TB05: Current Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Charge current alarm |
| Bit1 | Charge over current protection |
| Bit2 | Charge second level current protection |
| Bit3 | Discharge current alarm |
| Bit4 | Discharge over current protection |
| Bit5 | Discharge second level over current protection |
| Bit6 | Output short circuit protection |
| Bit7 | Reservation |

### TB16: Current Event Code 2

| INDEX | Definition |
|-------|------------|
| Bit0 | Output short latch up |
| Bit1 | Reservation |
| Bit2 | Second Charge latch up |
| Bit3 | Second Discharge latch up |
| Bit4 | Reservation |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB06: Capacity Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Reservation |
| Bit1 | Reservation |
| Bit2 | Soc alarm |
| Bit3 | Soc protection |
| Bit4 | Cell Diff alarm |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB17: Function Switch

| INDEX | Definition |
|-------|------------|
| Bit0 | Focs Output |
| Bit1 | Reservation |
| Bit2 | Reservation |
| Bit3 | Reservation |
| Bit4 | Reservation |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB07: FET Event Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Discharge FET on |
| Bit1 | Charge FET on |
| Bit2 | Current limiting FET on |
| Bit3 | Heating on |
| Bit4 | Reservation |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB08: Battery State Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | low Soc alarm |
| Bit1 | Intermittent charge |
| Bit2 | External switch control |
| Bit3 | Static standby and sleep mode |
| Bit4 | History data recording |
| Bit5 | Under Soc protect |
| Bit6 | Acktive-Limited Current |
| Bit7 | Passive-Limited Current |

### TB09: System State Codes

| INDEX | Definition |
|-------|------------|
| Bit0 | Discharge |
| Bit1 | Charge |
| Bit2 | Floating charge |
| Bit3 | Full charge |
| Bit4 | Standby mode |
| Bit5 | Turn off |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB10: Environment Function Switch

| INDEX | Definition |
|-------|------------|
| Bit0 | High environment temperature alarm |
| Bit1 | Over environment temperature protection |
| Bit2 | Low environment temperature alarm |
| Bit3 | Under environment temperature protection |
| Bit4 | Power high temperature alarm |
| Bit5 | Power over temperature protection |
| Bit6 | Cell temperature low heating |
| Bit7 | Cell voltage Fault |

### TB11: Current Function Switch 2

| INDEX | Definition |
|-------|------------|
| Bit0 | Output short latch up |
| Bit1 | Reservation |
| Bit2 | Charge second level over current latch up |
| Bit3 | Discharge second level over current latch up |
| Bit4 | Reservation |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB12: Equalization Function Switch

| INDEX | Definition |
|-------|------------|
| Bit0 | Equilibrium module to open |
| Bit1 | Static equilibrium indicate |
| Bit2 | Static equilibrium overtime |
| Bit3 | Equalization temperature limit |
| Bit4 | Reservation |
| Bit5 | Reservation |
| Bit6 | Reservation |
| Bit7 | Reservation |

### TB13: Date Format

| INDEX | Definition | Data limited | Data type | Bytes | Unit |
|-------|------------|--------------|-----------|-------|------|
| 0 | Year_Low | 1—9999 | UINT16 | 1 | Year |
| 1 | Year_High | | | 1 | |
| 2 | Month | 1—12 | UINT8 | 1 | Mon |
| 3 | Day | 1—31 | UINT8 | 1 | Day |
| 4 | Hour | 0—23 | UINT8 | 1 | H |
| 5 | Minute | 0—59 | UINT8 | 1 | Min |
| 6 | Second | 0—59 | UINT8 | 1 | s |
| 7 | Reservation | | UINT8 | 1 | --- |

### TB14: Timing History Format

| INDEX | Definition | Data type | Bytes | Unit |
|-------|------------|-----------|-------|------|
| 0 | Set the start date | 8 Bytes | 8 | See TB13 |
| 8 | Set the end date | 8 Bytes | 8 | See TB13 |
| 16 | SpaceTime_Low | UINT16 | 1 | s |
| | SpaceTime_High | | 1 | |

### TB15: Hard Fault Event Codes

| INDEX | Definition | Note |
|-------|------------|------|
| Bit0 | NTC Fault | Wire break or short |
| Bit1 | AFE Fault | AFE Comm. Error |
| Bit2 | Charge Mosfets Fault | Mosfets short |
| Bit3 | Discharge Mosfets Fault | Mosfets short |
| Bit4 | Cell Fault | Large Voltage different |
| Bit5 | Break Line Fault | |
| Bit6 | Key Fault | |
| Bit7 | Aerosol Alarm | |

### TB18: Indicator Function Switch

| INDEX | Definition |
|-------|------------|
| Bit0 | Buzzer indicator |
| Bit1 | LCD display |
| Bit2 | Manual Focs Output |
| Bit3 | Auto Focs Output |
| Bit4 | Under Voltage recover |
| Bit5 | Aerosol Test Function |
| Bit6 | Aerosol Normally Disconnected Mode |
| Bit7 | Temp-Curt Adjust |
