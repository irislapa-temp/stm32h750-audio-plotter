################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/adc.c \
../Core/Src/app_timers.c \
../Core/Src/audio_plotter.c \
../Core/Src/audio_plotter_draw.c \
../Core/Src/audio_plotter_sample.c \
../Core/Src/audio_queue.c \
../Core/Src/dma.c \
../Core/Src/main.c \
../Core/Src/measureCycles.c \
../Core/Src/plot_adc.c \
../Core/Src/plot_mems.c \
../Core/Src/stm32h7xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32h7xx.c \
../Core/Src/tim.c 

OBJS += \
./Core/Src/adc.o \
./Core/Src/app_timers.o \
./Core/Src/audio_plotter.o \
./Core/Src/audio_plotter_draw.o \
./Core/Src/audio_plotter_sample.o \
./Core/Src/audio_queue.o \
./Core/Src/dma.o \
./Core/Src/main.o \
./Core/Src/measureCycles.o \
./Core/Src/plot_adc.o \
./Core/Src/plot_mems.o \
./Core/Src/stm32h7xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32h7xx.o \
./Core/Src/tim.o 

C_DEPS += \
./Core/Src/adc.d \
./Core/Src/app_timers.d \
./Core/Src/audio_plotter.d \
./Core/Src/audio_plotter_draw.d \
./Core/Src/audio_plotter_sample.d \
./Core/Src/audio_queue.d \
./Core/Src/dma.d \
./Core/Src/main.d \
./Core/Src/measureCycles.d \
./Core/Src/plot_adc.d \
./Core/Src/plot_mems.d \
./Core/Src/stm32h7xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32h7xx.d \
./Core/Src/tim.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32H750xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../PDM2PCM/App -I../Middlewares/ST/STM32_Audio/Addons/PDM/Inc -I../Drivers/CMSIS_DSP/Include -I../Drivers/CMSIS_DSP/Include/dsp -I../Drivers/CMSIS_DSP/PrivateInclude -I../Drivers/BSP/STM32H750B-DK -I/bsp-mems/Drivers/BSP -I../Utilities/lcd -I../Utilities/CPU -I../Drivers/BSP/Components/Common -I../Utilities/Fonts -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/adc.cyclo ./Core/Src/adc.d ./Core/Src/adc.o ./Core/Src/adc.su ./Core/Src/app_timers.cyclo ./Core/Src/app_timers.d ./Core/Src/app_timers.o ./Core/Src/app_timers.su ./Core/Src/audio_plotter.cyclo ./Core/Src/audio_plotter.d ./Core/Src/audio_plotter.o ./Core/Src/audio_plotter.su ./Core/Src/audio_plotter_draw.cyclo ./Core/Src/audio_plotter_draw.d ./Core/Src/audio_plotter_draw.o ./Core/Src/audio_plotter_draw.su ./Core/Src/audio_plotter_sample.cyclo ./Core/Src/audio_plotter_sample.d ./Core/Src/audio_plotter_sample.o ./Core/Src/audio_plotter_sample.su ./Core/Src/audio_queue.cyclo ./Core/Src/audio_queue.d ./Core/Src/audio_queue.o ./Core/Src/audio_queue.su ./Core/Src/dma.cyclo ./Core/Src/dma.d ./Core/Src/dma.o ./Core/Src/dma.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/measureCycles.cyclo ./Core/Src/measureCycles.d ./Core/Src/measureCycles.o ./Core/Src/measureCycles.su ./Core/Src/plot_adc.cyclo ./Core/Src/plot_adc.d ./Core/Src/plot_adc.o ./Core/Src/plot_adc.su ./Core/Src/plot_mems.cyclo ./Core/Src/plot_mems.d ./Core/Src/plot_mems.o ./Core/Src/plot_mems.su ./Core/Src/stm32h7xx_it.cyclo ./Core/Src/stm32h7xx_it.d ./Core/Src/stm32h7xx_it.o ./Core/Src/stm32h7xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32h7xx.cyclo ./Core/Src/system_stm32h7xx.d ./Core/Src/system_stm32h7xx.o ./Core/Src/system_stm32h7xx.su ./Core/Src/tim.cyclo ./Core/Src/tim.d ./Core/Src/tim.o ./Core/Src/tim.su

.PHONY: clean-Core-2f-Src

