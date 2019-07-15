# SUPLA-DEV

Make a SUPLA-managed device from your RaspberryPI (or any other linux).

## Installation

```
sudo apt-get install libssl-dev
git clone https://github.com/SUPLA/supla-core
cd supla-core/supla-dev/Release
make all
cp supla.cfg.sample supla.cfg
```

Edit `supla.cfg` to match your needs.

## Run

```
./supla-core/supla-dev/Release/supla-dev -c ./supla-core/supla-dev/Release/supla.cfg
```

## Upgrade

Stop `supla-dev` if running and

```
cd supla-core
git pull
cd supla-dev/Release
make all
```