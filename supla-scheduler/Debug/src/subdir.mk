################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/abstract_rs_action.cpp \
../src/abstract_worker.cpp \
../src/action.cpp \
../src/action_copy.cpp \
../src/action_execute.cpp \
../src/action_interrupt.cpp \
../src/action_openclose.cpp \
../src/action_reveal.cpp \
../src/action_reveal_partially.cpp \
../src/action_rgb.cpp \
../src/action_set.cpp \
../src/action_shut.cpp \
../src/action_shut_partially.cpp \
../src/action_turn_onoff.cpp \
../src/database.cpp \
../src/dbcommon.cpp \
../src/ipcclient.cpp \
../src/queue.cpp \
../src/supla-scheduler.cpp \
../src/worker.cpp 

C_SRCS += \
../src/cfg.c \
../src/eh.c \
../src/ini.c \
../src/lck.c \
../src/log.c \
../src/safearray.c \
../src/schedulercfg.c \
../src/sthread.c \
../src/tools.c 

CPP_DEPS += \
./src/abstract_rs_action.d \
./src/abstract_worker.d \
./src/action.d \
./src/action_copy.d \
./src/action_execute.d \
./src/action_interrupt.d \
./src/action_openclose.d \
./src/action_reveal.d \
./src/action_reveal_partially.d \
./src/action_rgb.d \
./src/action_set.d \
./src/action_shut.d \
./src/action_shut_partially.d \
./src/action_turn_onoff.d \
./src/database.d \
./src/dbcommon.d \
./src/ipcclient.d \
./src/queue.d \
./src/supla-scheduler.d \
./src/worker.d 

C_DEPS += \
./src/cfg.d \
./src/eh.d \
./src/ini.d \
./src/lck.d \
./src/log.d \
./src/safearray.d \
./src/schedulercfg.d \
./src/sthread.d \
./src/tools.d 

OBJS += \
./src/abstract_rs_action.o \
./src/abstract_worker.o \
./src/action.o \
./src/action_copy.o \
./src/action_execute.o \
./src/action_interrupt.o \
./src/action_openclose.o \
./src/action_reveal.o \
./src/action_reveal_partially.o \
./src/action_rgb.o \
./src/action_set.o \
./src/action_shut.o \
./src/action_shut_partially.o \
./src/action_turn_onoff.o \
./src/cfg.o \
./src/database.o \
./src/dbcommon.o \
./src/eh.o \
./src/ini.o \
./src/ipcclient.o \
./src/lck.o \
./src/log.o \
./src/queue.o \
./src/safearray.o \
./src/schedulercfg.o \
./src/sthread.o \
./src/supla-scheduler.o \
./src/tools.o \
./src/worker.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__DEBUG=1 -I$(INCMYSQL) -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -D__DEBUG=1 -I$(INCMYSQL) -O2 -g3 -Wall -fsigned-char -c -fmessage-length=0 -fstack-protector-all -D_FORTIFY_SOURCE=2 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/abstract_rs_action.d ./src/abstract_rs_action.o ./src/abstract_worker.d ./src/abstract_worker.o ./src/action.d ./src/action.o ./src/action_copy.d ./src/action_copy.o ./src/action_execute.d ./src/action_execute.o ./src/action_interrupt.d ./src/action_interrupt.o ./src/action_openclose.d ./src/action_openclose.o ./src/action_reveal.d ./src/action_reveal.o ./src/action_reveal_partially.d ./src/action_reveal_partially.o ./src/action_rgb.d ./src/action_rgb.o ./src/action_set.d ./src/action_set.o ./src/action_shut.d ./src/action_shut.o ./src/action_shut_partially.d ./src/action_shut_partially.o ./src/action_turn_onoff.d ./src/action_turn_onoff.o ./src/cfg.d ./src/cfg.o ./src/database.d ./src/database.o ./src/dbcommon.d ./src/dbcommon.o ./src/eh.d ./src/eh.o ./src/ini.d ./src/ini.o ./src/ipcclient.d ./src/ipcclient.o ./src/lck.d ./src/lck.o ./src/log.d ./src/log.o ./src/queue.d ./src/queue.o ./src/safearray.d ./src/safearray.o ./src/schedulercfg.d ./src/schedulercfg.o ./src/sthread.d ./src/sthread.o ./src/supla-scheduler.d ./src/supla-scheduler.o ./src/tools.d ./src/tools.o ./src/worker.d ./src/worker.o

.PHONY: clean-src

