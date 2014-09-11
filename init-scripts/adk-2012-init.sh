cat /sys/kernel/adk_linux/aoa_init/version
echo "man=Google, Inc." > /sys/kernel/adk_linux/aoa_init/identity 
echo "model=DemoKit" > /sys/kernel/adk_linux/aoa_init/identity 
echo "version=2.0" > /sys/kernel/adk_linux/aoa_init/identity 
echo "aud" > /sys/kernel/adk_linux/aoa_init/start
echo "acc" > /sys/kernel/adk_linux/aoa_init/start
