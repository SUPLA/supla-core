# supla-mqtt-client

Ten projekt służy połączeniu SUPLA z MQTT. Jego podstawowa funkcjonalność to publikowanie stanu kanałów SUPLA do serwera MQTT oraz reagowanie na wiadomości serwera MQTT w celu sterowania kanałami SUPLA. Schematy wiadomości wyjściowych oraz wejściowych są konfigurowalne w plikach state.yaml oraz command.yaml

# Building
```
git clone https://github.com/lukbek/supla-core.git
cd supla-core/supla-mqtt-client/Release
sudo make clean
sudo make all
```
# Configuration

Projekt zawiera trzy pliki konfiguracyjne. Znajdują się on w katalogu config.
Najpierw edytujemy plik config_template.yaml

```
sudo nano config_template.yaml 

mqtt:
  host: 'test.mosquitto.org' # host serwera MQTT - dla testów można użyć test.mosquitto.org
  port: 1883 # port serwera MQTT
  commands_file_path: '/home/pi/supla-mqtt-client/config/command.yaml' # ścieżka bezwzględna do pliku command.yaml
  states_file_path: '/home/pi/supla-mqtt-client/config/state.yaml' # ścieżka do pliku state.yaml
  username: '' # nazwa użytkownika MQTT jeśli serwer jest zabezpieczony w ten sposób
  password: '' # hasło użytkownika MQTT
supla:
  port: 2016 # port serwera SUPLA
  host: 'localhost' # hostname serwera SUPLA
  location: 2 # id dostępu pobrane z SUPLA Cloud
  password: 'password' #hasło podanego identyfikatora dostępu
```
Następnie modyfikujemy w razie potrzeby plik state.yaml. Schemat budowy tego pliku jest następujący:
```
 - channel_type: 40 # typ kanału SUPLA
    state_topic: 'supla/channels/status/thermometer/$id' #temat pod którym będzie publikowany stan kanału
    payload_template: '{"caption": "$caption$", "temperature": $temperature$}' #szablon zawartości komunikatu MQTT
```

W zależności od typu kanału dostępne są różne makra, które mogą być użyte w szablonach. Najważniejsze z nich to:
```
$id$ - pod to makro zostanie podstawiony id kanału SUPLA
$caption$ - pod to makro zostanie podstawiona nazwa kanału SUPLA
$temperature$ - pod to makro zostanie podstawiona wartość temperatury (dla kanału w którym występuje temperatura)
$humidity$ - pod to makro zostanie podstawiona wartość wilgotności (dla kanału w którym występuje wilgotność)
Reszta makr została przedstawiona w pliku state.yaml

# Running

```
./supla-mqtt-client -config config-template.yaml
```

