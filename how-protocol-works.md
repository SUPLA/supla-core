# Introduction

Supla protocol is based on sending C structs between (S)erver, (C)lient (i.e. Android device) and (D)evice (micro controllers). 
Every struct is described in [proto.h](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h). In names of these structs
we can find common prefixes ```DCS```, ```SDC```, ```DS```, ```SD```, ```CS```, ```SC``` - each of them describes which direction, struct should be send:

* DCS = Device/Client -> Server
* SDC = Server -> Device/Client
* DS = Device -> Server
* SD = Server -> Device
* CS = Client -> Server
* SC = Server -> Client

# Packet sending

Only struct that is send directly is [TSuplaDataPacket](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L228) all others are serialized into ```TSuplaDataPacket.data``` field.

```C
typedef struct {

  char tag[SUPLA_TAG_SIZE];
  unsigned char version;
  unsigned _supla_int_t rr_id; // Request/Response ID
  unsigned _supla_int_t call_type;
  unsigned _supla_int_t data_size;
  char data[SUPLA_MAX_DATA_SIZE]; // Last variable in struct!

}TSuplaDataPacket;
```

Each packet start with ```tag``` that is always equal to ```S``` ```U``` ```P``` ```L``` ```A``` letters in ASCII coding (```83```, ```85```, ```80```, ```76```, ```65```).

To deserialize sent struct (that is in filed ```data```) you need to do switch on ```call_type``` field, i.e. if ```call_type``` is equal to
```67``` ([SUPLA_DS_CALL_REGISTER_DEVICE_C](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L91)) that means that packet contains [TDS_SuplaRegisterDevice_C](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L368) struct.

## How to deserialize?

Deserializing is simply just changing array of bytes into respective C type, i.e. int is just 4 bytes written in little endian.

Below I put some examples:

| Bytes in binary                     | Bytes in decimal | type                 | value          | Note           |
| ----------------------------------- | ---------------- | -------------------- | -------------- | -------------- |
| 01100101                            | 101              | byte                 | 101            |                |
| 11001100                            | -52              | byte                 | -52            |                |
| 10011010                            | 154              | char                 | 154            |                |
| 01101101 01110101 11100101 01011010 | 109  117 -27  90 | signed int           | 1 524 987 245  | Little endian! |
| 01111101 10001001 00011010 10100101 | 125 -119  26 -91 | signed int           | -1 524 987 523 | Little endian! |
| 11000010 11101001 10100111 11000010 | -62  -23 -89 -62 | unsigned int  / uint | 3 265 784 258  | Little endian! |

Data types that I use above:

| Name | Short name | Min value | Max value | signed? |
| ---- | ---------- | --------- | --------- | ------- |
| byte | byte | -128 | 127 | yes |
| char | char | 0 | 255 | no |
| signed int | int | –2 147 483 648 | 2 147 483 647 | yes |
| unsigned int | uint | 0 | 4 294 967 295 | no

Some structs has array of chars, but they are not encoded neither in ASCII nor in UTF-8. As an example I can show ```GUID``` field in 
[TDS_SuplaRegisterDevice](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L330). To parse this into String you need to read 
byte by byte and change each of them into two digit Hex number. Let's say we got this bytes: 

| Numeral system     | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
| --------------     | ------   | ------   | ------   | ------   |
| **BIN**            | 11011010 | 00001010 | 01010010 | 11111111 | 
| **DEC**            | 218      | 10       | 82       | 255      |
| **HEX**            | DA       | 0A       | 52       | FF       |

So in our case ```GUID``` value is equals to ```DA0A52ff```. 

## Communication wire

All communication that was described before is done via **TCP/IP** on port either **2016** or **2015**. Port 2016 is used for HTTPs communication. 

# Examples of communication

## Device registers

First thing each device does after turning on is an attempt to register to server. To do this device sends ```TDS_SuplaRegisterDevice```, ```TDS_SuplaDeviceChannel_B``` or
 ```TDS_SuplaRegisterDevice_C``` depending which version of Supla it has. As a response server sends ```TSD_SuplaRegisterDeviceResult```.

Life example:

Device wants to register, to do it it sends this bytes:

```
[
  83 85, 80, 76, 65, // SUPLA tag 
  5, // version = 5
  1, 0, 0, 0, // rr_id = 1 
  65, 0, 0, 0, // call_type = 65
  41, 1, 0, 0, // dataSize = 297
  -32, 4, 0, 0, 113, 97, 122, 119, 115, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -33, 22, -81, 125, -64, 116, -19, 2, 101, 103, -127, 16, -6, -66, 96, 60, 90, 65, 77, 69, 76, 32, 82, 79, 87, 45, 48, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 46, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 84, 11, 0, 0, 96, 0, 0, 0, -116, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
]
```

Server finds ```SUPLA``` tag and decode rest to this ```TSuplaDataPacket``` struct:

``` 
TSuplaDataPacket{
  version=5, 
  rrId=1, 
  callType=65, 
  dataSize=297, 
  data=[-32, 4, 0, 0, 113, 97, 122, 119, 115, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -33, 22, -81, 125, -64, 116, -19, 2, 101, 103, -127, 16, -6, -66, 96, 60, 90, 65, 77, 69, 76, 32, 82, 79, 87, 45, 48, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 46, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 84, 11, 0, 0, 96, 0, 0, 0, -116, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
}
```
 
Server finds that ```call_type``` is equal to ```65``` [SUPLA_ACCESSID_PWDHEX_MAXSIZE](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L90) what means in ```data``` field there is ```TDS_SuplaRegisterDevice_B``` struct
 
Server parses ```data``` field into ```TDS_SuplaRegisterDevice_B```

``` 
TDS_SuplaRegisterDevice_B{
  locationId=1248, 
  locationPwd=[113, 97, 122, 119, 115, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  guid=[-33, 22, -81, 125, -64, 116, -19, 2, 101, 103, -127, 16, -6, -66, 96, 60],
  name=[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 46, 48, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  softVer=[90, 65, 77, 69, 76, 32, 82, 79, 87, 45, 48, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  channelCount=1,
  channels=[...]
} 
```

**Here comes your logic!** From this struct you can build your own objects. What you gonna do from this point is up to you.
Just for this example, let's say that we gonna parse it to more friendly object (with Strings instead of arrays of chars) and we will send back 
response that confirms register.

```
DeviceRegisterRequest{
  locationId=1248, 
  locationPassword='qazwsx', 
  guid='DF16AF7DC074ED0265678110FABE603C', 
  name='ZAMEL ROW-01', 
  softVersion='2.0', 
  channels=[...]
}
```

As a response let's send ```70``` [SUPLA_SD_CALL_REGISTER_DEVICE_RESULT](https://github.com/magx2/supla-core/blob/master/supla-common/proto.h#L92):

```
TSD_SuplaRegisterDeviceResult{
  resultCode=70, 
  activityTimeout=100, 
  version=2, 
  versionMin=2
}
```

Let's encode it into ```data``` field in ```TSuplaDataPacket```

```
TSuplaDataPacket{
  version=2, 
  rrId=1, // this is just number that server increments each time it sends a message 
  callType=90, 
  dataSize=7, 
  data=[70, 0, 0, 0, 100, 2, 2] // here is encoded out TSD_SuplaRegisterDeviceResult
}
```

Change ```TSuplaDataPacket``` into bytes and attach at the beginning ```SUPLA``` tag

```
[83 85, 80, 76, 65, 2, 1, 0, 0, 0, 90, 0, 0, 0, 7, 0, 0, 0, 70, 0, 0, 0, 100, 2, 2]
```

## Channel value changes

### Device notifies about channel value change

> User pressed light switch and devices sent notification to server that his channel status changed from OFF to ON.

TODO @przemyslawzygmunt

### Server wants device to change channel value

> User click button to turn off light channel, so server is sending notification to device that it should change channel value from ON to OFF.

TODO @przemyslawzygmunt