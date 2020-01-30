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
cd supla-core/supla-dev/Release
./supla-dev -c ./supla.cfg
```

## Configure autostart

In order to reliably run `supla-dev` you should run it
by some process control system like [supervisor](http://supervisord.org/).

### Configuration on RaspberryPi

1. Install supervisor with `sudo apt-get install supervisor`.
1. Edit configuration file so you can manage the processes without root access:
   `sudo nano /etc/supervisor/supervisord.conf`. Set the following options in
   the `[unix_http_server]` section (replace `pi` with your username)
   and exit from the editor (<kbd>Ctrl</kbd>+<kbd>X</kbd>, <kbd>Y</kbd>, <kbd>Enter</kbd>):

    ```
    chmod=0770
    chown=root:pi
    ```

1. Create `supla-dev` process setup: `sudo nano /etc/supervisor/conf.d/supla-dev.conf`
   with the following contents (adjust path and user if needed, then save the changes and
   exit edit as before):

    ```
    [program:supla-dev]
    command=/home/pi/supla-core/supla-dev/Release/supla-dev -c supla.cfg
    directory=/home/pi/supla-core/supla-dev/Release
    autostart=true
    autorestart=true
    user=pi
    ```

1. Restart supervisor: `sudo service supervisor restart`

### Managing the process with supervisor

The `supla-dev` should start automatically after initial configuration
and after the system boots. From now on, you will use the `supervisorctl` 
command (without sudo) to manage the process.

* `supervisorctl status` shows if everything is ok
* `supervisorctl stop supla-dev` stops it
* `supervisorctl start supla-dev` starts it
* `supervisorctl tail supla-dev` shows error output (in case of problems)

## Upgrade

Stop `supla-dev` if running and

```
cd supla-core
git pull
cd supla-dev/Release
make all
```
