#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("usb:v22B8p2E83d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D00d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D01d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D02d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D03d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D04d*dc*dsc*dp*ic*isc*ip*");
MODULE_ALIAS("usb:v18D1p2D05d*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "0F3D9B7811F5ADAD1E4700D");
