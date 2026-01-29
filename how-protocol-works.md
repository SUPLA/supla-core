# Introduction

The SUPLA protocol is based on exchanging C structures between three parties: the **Server (S)**, **Client (C)** (e.g. Android or iOS apps), and **Device (D)** (microcontrollers).

All protocol structures are defined in `proto.h`:
[https://github.com/magx2/supla-core/blob/master/supla-common/proto.h](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h)

Structure names use fixed prefixes that indicate the direction of communication:

* **DCS** – Device / Client → Server
* **SDC** – Server → Device / Client
* **DS** – Device → Server
* **SD** – Server → Device
* **CS** – Client → Server
* **SC** – Server → Client

This convention is used consistently across the entire protocol.

# Packet format

The only structure sent directly over the wire is `TSuplaDataPacket`. All other protocol structures are serialized into its `data` field.

```
typedef struct {
  char tag[SUPLA_TAG_SIZE];
  unsigned char version;
  unsigned _supla_int_t rr_id;      // Request / Response ID
  unsigned _supla_int_t call_type;
  unsigned _supla_int_t data_size;
  char data[SUPLA_MAX_DATA_SIZE];   // Must be the last field
} TSuplaDataPacket;
```

Each packet starts with a fixed ASCII tag: `SUPLA` (bytes `83 85 80 76 65`). This allows fast protocol identification on the receiving side.

The payload format is determined by the `call_type` field. For example, if `call_type == 67` (`SUPLA_DS_CALL_REGISTER_DEVICE_C`), the `data` field contains a serialized `TDS_SuplaRegisterDevice_C` structure.

# Deserialization

Deserialization consists of interpreting the raw byte array as C data types. All multi-byte values are encoded in **little-endian** format.

Examples:

| Binary                              | Decimal         | Type                | Value          | Notes         |
| ----------------------------------- | --------------- | ------------------- | -------------- | ------------- |
| 01100101                            | 101             | byte                | 101            | signed        |
| 11001100                            | -52             | byte                | -52            | signed        |
| 10011010                            | 154             | char                | 154            | unsigned      |
| 01101101 01110101 11100101 01011010 | 109 117 -27 90  | signed int          | 1 524 987 245  | little-endian |
| 01111101 10001001 00011010 10100101 | 125 -119 26 -91 | signed int          | -1 524 987 523 | little-endian |
| 11000010 11101001 10100111 11000010 | -62 -23 -89 -62 | unsigned int (uint) | 3 265 784 258  | little-endian |

Supported data types:

| Name         | Short | Min value      | Max value     | Signed |
| ------------ | ----- | -------------- | ------------- | ------ |
| byte         | byte  | -128           | 127           | yes    |
| char         | char  | 0              | 255           | no     |
| signed int   | int   | -2 147 483 648 | 2 147 483 647 | yes    |
| unsigned int | uint  | 0              | 4 294 967 295 | no     |

Some structures contain raw byte arrays that are neither ASCII nor UTF-8 encoded. A common example is the `GUID` field in `TDS_SuplaRegisterDevice`.

To convert such fields into a string representation, each byte should be converted into a two-digit hexadecimal value.

Example:

| System | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
| ------ | -------- | -------- | -------- | -------- |
| BIN    | 11011010 | 00001010 | 01010010 | 11111111 |
| DEC    | 218      | 10       | 82       | 255      |
| HEX    | DA       | 0A       | 52       | FF       |

Resulting GUID fragment: `DA0A52FF`.

# Transport layer

All communication described above uses **TCP/IP**.

* Port **2015** – plain TCP
* Port **2016** – TLS (encrypted connection)

# Example: device registration

After startup, each device attempts to register itself on the server. Depending on protocol version, it sends one of:

* `TDS_SuplaRegisterDevice`
* `TDS_SuplaRegisterDevice_B`
* `TDS_SuplaRegisterDevice_C`
* or newer

The server responds with `TSD_SuplaRegisterDeviceResult` or `TSD_SuplaRegisterDeviceResult_B`.

Below is a real-world example of a device registration request sent as raw bytes:

```
[83, 85, 80, 76, 65, 5, 1, 0, 0, 0, 65, 0, 0, 0, 41, 1, 0, 0, ...]
```

The server detects the `SUPLA` tag and decodes the payload into a `TSuplaDataPacket`:

```
TSuplaDataPacket {
  version = 5,
  rrId = 1,
  callType = 65,
  dataSize = 297,
  data = [...]
}
```

`callType == 65` indicates that the payload contains a `TDS_SuplaRegisterDevice_B` structure, which is then parsed:

```
TDS_SuplaRegisterDevice_B {
  locationId = 1248,
  locationPwd = [...],
  guid = [...],
  name = [...],
  softVer = [...],
  channelCount = 1,
  channels = [...]
}
```

At this point, server-side logic takes over. Typically, the raw structure is mapped into a more convenient representation:

```
DeviceRegisterRequest {
  locationId = 1248,
  locationPassword = "qazwsx",
  guid = "DF16AF7DC074ED0265678110FABE603C",
  name = "ZAMEL ROW-01",
  softVersion = "2.0",
  channels = [...]
}
```

# Registration response

The server confirms successful registration by sending `SUPLA_SD_CALL_REGISTER_DEVICE_RESULT`:

```
TSD_SuplaRegisterDeviceResult {
  resultCode = 70,
  activityTimeout = 100,
  version = 2,
  versionMin = 2
}
```

Encoded into `TSuplaDataPacket`:

```
TSuplaDataPacket {
  version = 2,
  rrId = 1,
  callType = 90,
  dataSize = 7,
  data = [70, 0, 0, 0, 100, 2, 2]
}
```

Serialized to bytes:

```
[83, 85, 80, 76, 65, 2, 1, 0, 0, 0, 90, 0, 0, 0, 7, 0, 0, 0, 70, 0, 0, 0, 100, 2, 2]
```

# Channel value changes

## Device → server

A device notifies the server when a channel value changes, e.g. a physical light switch toggles from OFF to ON.

*(to be documented)*

## Server → device

The server requests a channel value change, e.g. when a user taps a button in the mobile app.

*(to be documented)*

