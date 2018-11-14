################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/http/cJSON.c \
../src/http/cJSON_Utils.c 

CPP_SRCS += \
../src/http/alexaclient.cpp \
../src/http/oauthclient.cpp \
../src/http/trivialhttp.cpp \
../src/http/trivialhttps.cpp 

OBJS += \
./src/http/alexaclient.o \
./src/http/cJSON.o \
./src/http/cJSON_Utils.o \
./src/http/oauthclient.o \
./src/http/trivialhttp.o \
./src/http/trivialhttps.o 

C_DEPS += \
./src/http/cJSON.d \
./src/http/cJSON_Utils.d 

CPP_DEPS += \
./src/http/alexaclient.d \
./src/http/oauthclient.d \
./src/http/trivialhttp.d \
./src/http/trivialhttps.d 


# Each subdirectory must supply rules for building sources it contributes
src/http/%.o: ../src/http/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	$(CXX) -D__DEBUG=1 -D__OPENSSL_TOOLS=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I/usr/include/mysql -I../src/device -I../src/user -I../src -I$(SSLDIR)/include -I../src/client -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/http/%.o: ../src/http/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	$(CC) -D__OPENSSL_TOOLS=1 -D__DEBUG=1 -D__SSOCKET_WRITE_TO_FILE=$(SSOCKET_WRITE_TO_FILE) -D__BCRYPT=1 -I$(SSLDIR)/include -I../src/user -I../src/device -I../src/client -I../src -O2 -g3 -Wall -c -fmessage-length=0 -fstack-protector-all  -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


