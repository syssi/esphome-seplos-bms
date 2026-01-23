# Battery Management System (BMS) for Communication

**MODBUS-ASCII Communication Protocol**

**Applicable interfaces:** RS485  
**Applicable models:** EMU10XX, 11XX Series

---

## Document Information

| Date | Version | Revision Notes |
|------|---------|----------------|
| 20191015 | V2.0 | First version |

---

## I. Communication instructions

EMU11XX series of BMS products communicate with FSU, PC or other upper controllers via RS485, for which MODBUS ASCII communication protocol is adopted, and information frames are established according to YD/T1363.3 specification.

### 1.1 Interface setting

EMU11XX series of BMS products adopt asynchronous serial communication interfaces. Start bit: 1; data bit: 8; stop bit: 1; check bit: none; default data transmission rate: 9600BPS.

### 1.2 Interface connection

```
FSU/PC -- A  B -- BMS (DIP address 1) -- A  B -- BMS (DIP address 2) -- ... -- BMS (DIP address 15)
          |  |                           |  |
       120Ω                           120Ω
       1/4w                           1/4w
```

**Note:** A 120Ω terminal matching resistor shall be added to the starting and final points of the communication connection respectively

### 1.3 Connection mode

The communication between the supervision unit (SU) and the supervision module (SM) is a point-to-multipoint master-slave mode. SU calls SM through the broadcast address and issues a command. The called SM at the corresponding address receives the command and returns the response information. If SU does not receive the response information of SM in 500ms or receives the wrong response information, the communication process is considered to have failed.

---

## II. Information structure

Information is organized according to a certain structure as a way to ensure that it can be transmitted correctly between SU and SM. See Table 1 for information structure. Information is composed of many bytes. One or more bytes make up a unit, and each unit has a name, expressing the defined meaning. Table 2 contains comments on the units in Table 1, Table 3 contains a further comments on CID1 in Table 2, and Table 4 and Table 5 contain further comments on CID2 in Table 2.

### Table 1 List of information structures

| Items | SOI | VER | ADR | CID1 | CID2 | LENGTH | INFO | CHKSUM | EOI |
|-------|-----|-----|-----|------|------|--------|------|--------|-----|
| ASCII byte | 1 | 2 | 2 | 2 | 2 | 4 | LENID | 4 | 1 |

### Table 2 Comments on information structures

| Items | Meaning | Remarks |
|-------|---------|---------|
| **Start code** | Start code SOI: Starting of a data frame | SOI =7EH(～) |
| **Version code (high)** | Version code VER: A communication protocol version code is composed of 2 ASCII codes | Protocol version V2.0 =32H 30H |
| **Version code (low)** | | |
| **Address code (high)** | Address code ADR: A device address identification code is composed of 2 ASCII codes | 00-15 valid, Address 1=30H 31H |
| **Address code (low)** | | |
| **Device code (high)** | Device code CID1: A device type identification code is composed of 2 ASCII codes | ID=34H 36H |
| **Device code (low)** | | |
| **Function code (high)** | Function code CID2: The command code CMD sent by SU to SM or the return code RTN returned by SM to SU is composed of 2 ASCII codes | See CMD in "Table 4" for details See RTN in "Table 5" for details |
| **Function code (low)** | | |
| **Length code MSB** | Length code LENGTH: Data information INFO length, including LENID and LCHKSUM It is composed of 4 ASCII codes | See "3.2" for details |
| **Length code 3** | | |
| **Length code 2** | | |
| **Length code LSB** | | |
| **Data code Data*LENID** | Data code INFO: Including Command data information COMMAND INFO sent by SU to SM Response data information RTNDATA INFO returned by SM to SU It is composed of "LENID" ASCII codes | |
| **Check code MSB** | Check code CHKSUM: It is composed of 4 ASCII codes | See "3.3" for details |
| **Check code 3** | | |
| **Check code 2** | | |
| **Check code LSB** | | |
| **End code** | End code EOI: Ending of a data frame | EOI =0DH( CR ) |

### Table 3 Device code CID1

| S/N | CID1 (HEX): Device code | Meaning |
|-----|-------------------------|---------|
| 1 | 46H | Lithium iron phosphate battery BMS |
| … | … | … |

### Table 4 Command code CID2

| S/N | CID2 (HEX): Command code | Meaning |
|-----|--------------------------|---------|
| 1 | 42H | Acquisition of telemetering information |
| 2 | 44H | Acquisition of telecommand information |
| 3 | 45H | Telecontrol command |
| 4 | 47H | Acquisition of teleregulation information |
| 5 | 49H | Setting of teleregulation information |
| 6 | 4FH | Acquisition of the communication protocol version number |
| 7 | 51H | Acquisition of device vendor information |
| 8 | 4BH | Acquisition of historical data |
| 9 | 4DH | Acquisition time |
| 10 | 4EH | Synchronization time |
| 11 | A0H | Production calibration |
| 12 | A1H | Production setting |
| 13 | A2H | Regular recording |

### Table 5 Return code CID2

| S/N | CID2 (HEX): Command code | Meaning |
|-----|--------------------------|---------|
| 1 | 00H | Normal |
| 2 | 01H | VER error |
| 3 | 02H | CHKSUM error |
| 4 | 03H | LCHKSUM error |
| 5 | 04H | CID2 invalid |
| 6 | 05H | Command format error |
| 7 | 06H | Data invalid (parameter setting) |
| 8 | 07H | No data (history) |
| 9 | E1H | CID1 invalid |
| 10 | E2H | Command execution failure |
| 11 | E3H | Device fault |
| 12 | E4H | Invalid permissions |

---

## III. Data format

### 3.1 Data transmission format

SOI and EOI are explained and transmitted in HEX. Other items are explained in HEX, transmitted in HEX-ASCII code, and each byte contains 2 ASCII codes. E.g. CID2=4BH, transmit in 2 bytes, 34H ('4' in ASCII code) and 42H ('B' in ASCII code).

### 3.2 LENGTH format

#### Table 6 LENGTH format

| Length check code LCHKSUM | LENID (number of bytes of ASCII code in INFO) |
|---------------------------|------------------------------------------------|
| D15 D14 D13 D12 | D11 D10 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0 |

#### 3.2.1 LENID
LENID represents the number of bytes of ASCII code in INFO. When LENID is equal to 0, the INFO is null, that is, this item does not exist. LENID has only 12 bits, so the data package cannot exceed 4,095 bytes.

#### 3.2.2 LCHKSUM calculation
To calculate LCHKSUM: D11D10D9D8+D7D6D5D4+D3D2D1D0, sum them up, mod 16, take the remainder, do a bitwise invert and then plus 1.

For example, the number of bytes of ASCII code in INFO is 18, then LENID=0000 0001 0010B.

D11D10D9D8+D7D6D5D4+D3D2D1D0=0000B+0001B+0010B=0011B, mod 16, remainder=0011B, do a bitwise invert and plus 1=1101B, then LCHKSUM=1101B.

#### 3.2.3 LENGTH transmission
LENGTH (in 3.2.2): 1101 0000 0001 0010B=D012H.

For LENGTH transmission, HIGH byte first, then LOW byte, and it is divided into 4 ASCII codes.

### 3.3 CHKSUM format

To calculate CHKSUM, except for SOI, EOI and CHKSUM, add values to get the sum of other characters in ASCII code, then mod 65536, take the remainder, do a bitwise invert and then plus 1.

E.g. information frame "~1203400456ABCEFEFC72CR",
CHKSUM='1'+'2'+'0'+...+'F'+'E'=31H+32H+30H+...+46H+45H=038EH, mod 65536, remainder=038EH, do a bitwise invert and plus 1= FC72H.

For CHKSUM transmission, HIGH byte first, then LOW byte, and it is divided into 4 ASCII codes.

---

## IV. Communication commands

### 4.1 Telemetry commands

#### 4.1.1 Telemetry command frame

CID2=42H, INFO is 1 byte COMMAND_GROUP.

- COMMAND_GROUP=0x01, acquire data of battery group 1;
- COMMAND_GROUP=0x02, acquire data of battery group 2;
- ...
- COMMAND_GROUP=0xFF, acquire data of all battery groups;

**Note:** GROUP=0xFF only for RS232, but not for RS485;

When RS485 is used for communication, SM checks whether the received COMMAND_GROUP matches the DIP address;

When RS232 is used for communication, COMMAND_GROUP is used to identify the number of SU addressing multi-group parallel batteries.

E.g. VER=20H and CID1=46H, the telemetry commands for different addresses are shown in Table 7

#### Table 7 Telemetry command examples

| Address | Telemetry Command Info Frame (ASCII) |
|---------|--------------------------------------|
| 00 | 7E 32 30 30 30 34 36 34 32 45 30 30 32 30 30 46 44 33 37 0D |
| 01 | 7E 32 30 30 31 34 36 34 32 45 30 30 32 30 31 46 44 33 35 0D |
| 02 | 7E 32 30 30 32 34 36 34 32 45 30 30 32 30 32 46 44 33 33 0D |
| 03 | 7E 32 30 30 33 34 36 34 32 45 30 30 32 30 33 46 44 33 31 0D |
| 04 | 7E 32 30 30 34 34 36 34 32 45 30 30 32 30 34 46 44 32 46 0D |
| 05 | 7E 32 30 30 35 34 36 34 32 45 30 30 32 30 35 46 44 32 44 0D |
| 06 | 7E 32 30 30 36 34 36 34 32 45 30 30 32 30 36 46 44 32 42 0D |
| 07 | 7E 32 30 30 37 34 36 34 32 45 30 30 32 30 37 46 44 32 39 0D |
| 08 | 7E 32 30 30 38 34 36 34 32 45 30 30 32 30 38 46 44 32 37 0D |
| 09 | 7E 32 30 30 39 34 36 34 32 45 30 30 32 30 39 46 44 32 35 0D |
| 10 | 7E 32 30 30 41 34 36 34 32 45 30 30 32 30 41 46 44 31 35 0D |
| 11 | 7E 32 30 30 42 34 36 34 32 45 30 30 32 30 42 46 44 31 33 0D |
| 12 | 7E 32 30 30 43 34 36 34 32 45 30 30 32 30 43 46 44 31 31 0D |
| 13 | 7E 32 30 30 44 34 36 34 32 45 30 30 32 30 44 46 44 30 46 0D |
| 14 | 7E 32 30 30 45 34 36 34 32 45 30 30 32 30 45 46 44 30 44 0D |
| 15 | 7E 32 30 30 46 34 36 34 32 45 30 30 32 30 46 46 44 30 42 0D |

#### 4.1.2 Telemetry return frame

CID2=00H, INFO is 75 bytes. See Table 8 and Table 9 for data content and conversion respectively.

#### Table 8 Comments on telemetry return

| S/N | Content | Number of bytes (HEX) |
|-----|---------|----------------------|
| 1 | DATA FLAG | 1 |
| 2 | COMMAND GROUP | 1 |
| 3 | Number of cells M=16 | 1 |
| 4 | Voltage of Cell 1 (mV) | 2 |
|   | Voltage of Cell 2 (mV) | 2 |
|   | ... | |
|   | Voltage of Cell M (mV) | 2 |
| 5 | Number of temperatures N=6 | 1 |
| 6 | Cell temperature 1 (0.1℃) | 2 |
|   | Cell temperature 2 (0.1℃) | 2 |
|   | Cell temperature 3 (0.1℃) | 2 |
|   | Cell temperature 4 (0.1℃) | 2 |
|   | Environment temperature (0.1℃) | 2 |
|   | Power temperature (0.1℃) | 2 |
| 7 | Charge/discharge current (0.01A) | 2 |
| 8 | Total battery voltage (0.01V) | 2 |
| 9 | Residual capacity (0.01Ah) | 2 |
| 10 | Custom number P=10 | 1 |
| 11 | Battery capacity (0.01Ah) | 2 |
| 12 | SOC (1‰) | 2 |
| 13 | Rated capacity (0.01Ah) | 2 |
| 14 | Number of cycles | 2 |
| 15 | SOH (1‰) | 2 |
| 16 | Port voltage (0.01V) | 2 |
| 17 | Reservation | 2 |
| 18 | Reservation | 2 |
| 19 | Reservation | 2 |
| 20 | Reservation | 2 |

#### Table 9 Methods of data conversion

| Parameter | Description |
|-----------|-------------|
| **Temperature** | Unsigned integer, in 0.1K, actual value=(transmission value-2731)/10(℃). E.g. 3032 means (3032-2731)/10(℃) =30.1℃ |
| **Total current** | Signed integer, in A, actual value=transmission value/100(A). E.g. 4500 means 45.00 A |
| **Total voltage** | Unsigned integer, in V, actual value=transmission value/100(V). E.g. 5400 means 54.00 V |
| **Capacity** | Unsigned integer, in Ah, actual value=transmission value/100(Ah). E.g. 4830 means 48.30Ah |

### 4.2 Telecommands

#### 4.2.1 Telecommand frame

CID2=44H, INFO is 1 byte COMMAND_GROUP.

- COMMAND_GROUP=0x01, acquire data of battery group 1;
- COMMAND_GROUP=0x02, acquire data of battery group 2;
- ...
- COMMAND_GROUP=0xFF, acquire data of all battery groups;

**Note:** GROUP=0xFF only for RS232, but not for RS485;

When RS485 is used for communication, SM checks whether the received COMMAND_GROUP matches the DIP address;

When RS232 is used for communication, COMMAND_GROUP is used to identify the number of SU addressing multi-group parallel batteries.

E.g. VER=20H and CID1=46H, the telecommands for different addresses are shown in Table 10

#### Table 10 Telecommand examples

| Address | Telemetry Command Info Frame (ASCII) |
|---------|--------------------------------------|
| 00 | 7E 32 30 30 30 34 36 34 34 45 30 30 32 30 30 46 44 33 35 0D |
| 01 | 7E 32 30 30 31 34 36 34 34 45 30 30 32 30 31 46 44 33 33 0D |
| 02 | 7E 32 30 30 32 34 36 34 34 45 30 30 32 30 32 46 44 33 31 0D |
| 03 | 7E 32 30 30 33 34 36 34 34 45 30 30 32 30 33 46 44 32 46 0D |
| 04 | 7E 32 30 30 34 34 36 34 34 45 30 30 32 30 34 46 44 32 44 0D |
| 05 | 7E 32 30 30 35 34 36 34 34 45 30 30 32 30 35 46 44 32 42 0D |
| 06 | 7E 32 30 30 36 34 36 34 34 45 30 30 32 30 36 46 44 32 39 0D |
| 07 | 7E 32 30 30 37 34 36 34 34 45 30 30 32 30 37 46 44 32 37 0D |
| 08 | 7E 32 30 30 38 34 36 34 34 45 30 30 32 30 38 46 44 32 35 0D |
| 09 | 7E 32 30 30 39 34 36 34 34 45 30 30 32 30 39 46 44 32 33 0D |
| 10 | 7E 32 30 30 41 34 36 34 34 45 30 30 32 30 41 46 44 31 33 0D |
| 11 | 7E 32 30 30 42 34 36 34 34 45 30 30 32 30 42 46 44 31 31 0D |
| 12 | 7E 32 30 30 43 34 36 34 34 45 30 30 32 30 43 46 44 30 46 0D |
| 13 | 7E 32 30 30 44 34 36 34 34 45 30 30 32 30 44 46 44 30 44 0D |
| 14 | 7E 32 30 30 45 34 36 34 34 45 30 30 32 30 45 46 44 30 42 0D |
| 15 | 7E 32 30 30 46 34 36 34 34 45 30 30 32 30 46 46 44 30 39 0D |

#### 4.2.2 Telecommand return frame

CID2=00H, INFO is 49 bytes. Please refer to Table 11 for INFO data, Table 12 for the meaning of 24 byte alarms, and Table 13 for the meaning of 20 bit alarms.

#### Table 11 Comments on telecommand return

| S/N | Content | Number of bytes (HEX) |
|-----|---------|----------------------|
| 1 | DATA FLAG | 1 |
| 2 | COMMAND GROUP | 1 |
| **The following are 24 byte alarms** | | |
| 3 | Number of cells M=16 | 1 |
| 4 | Cell 1 alarm | 1 |
|   | Cell 2 alarm | 1 |
|   | ... | |
|   | Cell M alarm | 1 |
| 5 | Number of temperatures N=6 | 1 |
| 6 | Cell temperature alarm 1 | 1 |
|   | Cell temperature alarm 2 | 1 |
|   | Cell temperature alarm 3 | 1 |
|   | Cell temperature alarm 4 | 1 |
|   | Environment temperature alarm | 1 |
|   | Power temperature alarm 1 | 1 |
| 7 | Charge/discharge current alarm | 1 |
| 8 | Total battery voltage alarm | 1 |
| **The following are 20 bit alarms** | | |
| 9 | Number of custom alarms P=20 | 1 |
| 10 | Alarm event 1 | 1 |
|    | Alarm event 2 | 1 |
|    | Alarm event 3 | 1 |
|    | Alarm event 4 | 1 |
|    | Alarm event 5 | 1 |
|    | Alarm event 6 | 1 |
|    | On-off state | 1 |
|    | Equilibrium state 1 | 1 |
|    | Equilibrium state 2 | 1 |
|    | System state | 1 |
|    | Disconnection state 1 | 1 |
|    | Disconnection state 2 | 1 |
|    | Alarm event 7 | 1 |
|    | Alarm event 8 | 1 |
|    | Reservation extension | 1 |
|    | Reservation extension | 1 |
|    | Reservation extension | 1 |
|    | Reservation extension | 1 |
|    | Reservation extension | 1 |
|    | Reservation extension | 1 |

#### Table 12 Comments on byte alarms

| S/N | Alarm | Value | Meaning |
|-----|-------|-------|---------|
| 1 | 0x00 | Normal, no alarm |
| 2 | 0x01 | Alarm that analog quantity reaches the lower limit |
| 3 | 0x02 | Alarm that analog quantity reaches the upper limit |
| 4 | 0xF0 | Other alarms |

#### Table 13 Comments on bit alarms

| Alarm | Flag bit information (1: trigger, 0: normal) |
|-------|-----------------------------------------------|
| **Alarm event 1** | |
| 0 | Voltage sensor fault |
| 1 | Temperature sensor fault |
| 2 | Current sensor fault |
| 3 | Key switch fault |
| 4 | Cell voltage dropout fault |
| 5 | Charge switch fault |
| 6 | Discharge switch fault |
| 7 | Current limit switch fault |
| **Alarm event 2** | |
| 0 | Monomer high voltage alarm |
| 1 | Monomer overvoltage protection |
| 2 | Monomer low voltage alarm |
| 3 | Monomer under voltage protection |
| 4 | High voltage alarm for total voltage |
| 5 | Overvoltage protection for total voltage |
| 6 | Low voltage alarm for total voltage |
| 7 | Under voltage protection for total voltage |
| **Alarm event 3** | **Cell temperature** |
| 0 | Charge high temperature alarm |
| 1 | Charge over temperature protection |
| 2 | Charge low temperature alarm |
| 3 | Charge under temperature protection |
| 4 | Discharge high temperature alarm |
| 5 | Discharge over temperature protection |
| 6 | Discharge low temperature alarm |
| 7 | Discharge under temperature protection |
| **Alarm event 4** | **Environment temperature** |
| 0 | Environment high temperature alarm |
| 1 | Environment over temperature protection |
| 2 | Environment low temperature alarm |
| 3 | Environment under temperature protection |
| 4 | Power over temperature protection | **Power temperature** |
| 5 | Power high temperature alarm |
| 6 | Cell low temperature heating | **Cell temperature** |
| 7 | Reservation bit |
| **Alarm event 5** | |
| 0 | Charge over current alarm |
| 1 | Charge over current protection |
| 2 | Discharge over current alarm |
| 3 | Discharge over current protection |
| 4 | Transient over current protection |
| 5 | Output short circuit protection |
| 6 | Transient over current lockout |
| 7 | Output short circuit lockout |
| **Alarm event 6** | |
| 0 | Charge high voltage protection |
| 1 | Intermittent recharge waiting |
| 2 | Residual capacity alarm |
| 3 | Residual capacity protection |
| 4 | Cell low voltage charging prohibition |
| 5 | Output reverse polarity protection |
| 6 | Output connection fault |
| 7 | Inside bit |
| **On-off state** | **Flag bit information (1: on, 0: off)** |
| 0 | Discharge switch state |
| 1 | Charge switch state |
| 2 | Current limit switch state |
| 3 | Heating switch state |
| 4-7 | Reservation bit |
| **Equilibrium state 1** | **Flag bit information (1: on, 0: off)** |
| 0 | Cell 01 equilibrium |
| 1 | Cell 02 equilibrium |
| 2 | Cell 03 equilibrium |
| 3 | Cell 04 equilibrium |
| 4 | Cell 05 equilibrium |
| 5 | Cell 06 equilibrium |
| 6 | Cell 07 equilibrium |
| 7 | Cell 08 equilibrium |
| **Equilibrium state 2** | **Flag bit information (1: on, 0: off)** |
| 0 | Cell 09 equilibrium |
| 1 | Cell 10 equilibrium |
| 2 | Cell 11 equilibrium |
| 3 | Cell 12 equilibrium |
| 4 | Cell 13 equilibrium |
| 5 | Cell 14 equilibrium |
| 6 | Cell 15 equilibrium |
| 7 | Cell 16 equilibrium |
| **System state** | **Flag bit information (1: access, 0: exit)** |
| 0 | Discharge |
| 1 | Charge |
| 2 | Floating charge |
| 3 | Reservation bit |
| 4 | Standby |
| 5 | Shutdown |
| 6 | Reservation bit |
| 7 | Reservation bit |
| **Disconnection state 1** | **Flag bit information (1: trigger, 0: normal)** |
| 0 | Cell 01 disconnection |
| 1 | Cell 02 disconnection |
| 2 | Cell 03 disconnection |
| 3 | Cell 04 disconnection |
| 4 | Cell 05 disconnection |
| 5 | Cell 06 disconnection |
| 6 | Cell 07 disconnection |
| 7 | Cell 08 disconnection |
| **Disconnection state 2** | **Flag bit information (1: trigger, 0: normal)** |
| 0 | Cell 09 disconnection |
| 1 | Cell 10 disconnection |
| 2 | Cell 11 disconnection |
| 3 | Cell 12 disconnection |
| 4 | Cell 13 disconnection |
| 5 | Cell 14 disconnection |
| 6 | Cell 15 disconnection |
| 7 | Cell 16 disconnection |
| **Alarm event 7** | **Flag bit information (1: trigger, 0: normal)** |
| 0 | Inside bit |
| 1 | Inside bit |
| 2 | Inside bit |
| 3 | Inside bit |
| 4 | Automatic charging waiting |
| 5 | Manual charging waiting |
| 6 | Inside bit |
| 7 | Inside bit |
| **Alarm event 8** | **Flag bit information (1: trigger, 0: normal)** |
| 0 | EEP storage fault |
| 1 | RTC error |
| 2 | Voltage calibration not performed |
| 3 | Current calibration not performed |
| 4 | Zero calibration not performed |
| 5 | Inside bit |
| 6 | Inside bit |
| 7 | Inside bit |
