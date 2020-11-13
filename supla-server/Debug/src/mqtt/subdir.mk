################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mqtt/mqtt.c 

CPP_SRCS += \
../src/mqtt/mqtt_client.cpp \
../src/mqtt/mqtt_client_datasource.cpp \
../src/mqtt/mqtt_client_ini_settings.cpp \
../src/mqtt/mqtt_client_library_adapter.cpp \
../src/mqtt/mqtt_client_settings.cpp \
../src/mqtt/mqtt_client_suite.cpp \
../src/mqtt/mqtt_publisher.cpp \
../src/mqtt/mqtt_publisher_datasource.cpp \
../src/mqtt/mqtt_subscriber.cpp \
../src/mqtt/mqttc_library_adapter.cpp 

OBJS += \
./src/mqtt/mqtt.o \
./src/mqtt/mqtt_client.o \
./src/mqtt/mqtt_client_datasource.o \
./src/mqtt/mqtt_client_ini_settings.o \
./src/mqtt/mqtt_client_library_adapter.o \
./src/mqtt/mqtt_client_settings.o \
./src/mqtt/mqtt_client_suite.o \
./src/mqtt/mqtt_publisher.o \
./src/mqtt/mqtt_publisher_datasource.o \
./src/mqtt/mqtt_subscriber.o \
./src/mqtt/mqttc_library_adapter.o 

C_DEPS += \
./src/mqtt/mqtt.d 

CPP_DEPS += \
./src/mqtt/mqtt_client.d \
./src/mqtt/mqtt_client_datasource.d \
./src/mqtt/mqtt_client_ini_settings.d \
./src/mqtt/mqtt_client_library_adapter.d \
./src/mqtt/mqtt_client_settings.d \
./src/mqtt/mqtt_client_suite.d \
./src/mqtt/mqtt_publisher.d \
./src/mqtt/mqtt_publisher_datasource.d \
./src/mqtt/mqtt_subscriber.d \
./src/mqtt/mqttc_library_adapter.d 


# Each subdirectory must supply rules for building sources it contributes
src/mqtt/%.o: ../src/mqtt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/mqtt -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/mqtt/%.o: ../src/mqtt/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -DSERVER_VERSION_23 -DUSE_DEPRECATED_EMEV_V1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(INCMYSQL) -I../src/mqtt -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -std=c++11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


