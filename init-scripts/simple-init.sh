
cat /sys/kernel/adk_linux/aoa_init/version
echo "manufacturer=Nexus-Computing GmbH" > /sys/kernel/adk_linux/aoa_init/identity 
echo "model=Simple Demo" > /sys/kernel/adk_linux/aoa_init/identity 
echo "ver=1.0" > /sys/kernel/adk_linux/aoa_init/identity 
echo "url=http://www.nexus-computing.ch/SimpleAccessory.apk" > /sys/kernel/adk_linux/aoa_init/identity
#first three letters of string name are significant for adk-aos-skeleton module,
#also its case insensitive
echo "aud" > /sys/kernel/adk_linux/aoa_init/start
echo "acc" > /sys/kernel/adk_linux/aoa_init/start
#first three letters are significant for adk-aos-skeleton module,also its case insensitive
