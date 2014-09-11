cat /sys/kernel/adk_linux/aoa_init/version
echo "man=Texas Instruments" > /sys/kernel/adk_linux/aoa_init/identity 
echo "model=Test" > /sys/kernel/adk_linux/aoa_init/identity 
echo "version=1.0" > /sys/kernel/adk_linux/aoa_init/identity 
echo "aud" > /sys/kernel/adk_linux/aoa_init/start
echo "acc" > /sys/kernel/adk_linux/aoa_init/start
