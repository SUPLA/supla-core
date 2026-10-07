# SupLAN production Server provisioning sources.
CPP_SRCS += ../src/suplan/peer_service.cpp ../src/suplan/peer_dao.cpp ../src/suplan/peer_provisioner.cpp ../src/suplan/server_peer_transport.cpp
CPP_DEPS += ./src/suplan/peer_service.d ./src/suplan/peer_dao.d ./src/suplan/peer_provisioner.d ./src/suplan/server_peer_transport.d
OBJS += ./src/suplan/peer_service.o ./src/suplan/peer_dao.o ./src/suplan/peer_provisioner.o ./src/suplan/server_peer_transport.o

# Each subdirectory must supply rules for building sources it contributes
src/suplan/%.o: ../src/suplan/%.cpp src/suplan/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -std=c++17 -D__DEBUG=1 -DMQTTC_PAL_FILE=../src/mqtt/mqtt_pal.h -DUSE_OS_TZDB=1 -D__SUPLA_SERVER=1 -DUSE_DEPRECATED_EMEV_V1 -DUSE_DEPRECATED_EMEV_V2 -D__TEST=1 -D__OPENSSL_TOOLS=1 -D__BCRYPT=1 -I../src -I../src/external/inja/include -I../src/external/MQTT-C/include -I../src/asynctask -I../src/mqtt -I$(INCMYSQL) -I../src/user -I../src/device -I../src/client -I$(SSLDIR)/include -I../src/test -I/usr/include/cjson -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '
