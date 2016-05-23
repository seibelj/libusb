#include <stdlib.h>
#include <emscripten.h>

#include "libusbi.h"

static int chrome_priv_add_device(struct libusb_context *ctx, char* js_device);


static int chrome_init(struct libusb_context *ctx) {
    usbi_dbg("called chrome_init, getting current devices and registering callbacks");

    EM_ASM_({
        var createDeviceString = function(device) {
            return device.device + "||" + device.vendorId + "||" + device.productId +
                "||" + device.productName + "||" + device.manufacturerName +
                "||" + device.serialNumber;
        };

        chrome.usb.getDevices({}, function(devices) {
            var len = devices.length;
            for (var i = 0; i < len; i++) {
                var str = createDeviceString(device);
                var buffer = Module._malloc(str.length+1);
                Module.writeStringToMemory(str, buffer);
                var result = Module.ccall('chrome_priv_add_device', // name of C function
                    'number', // return type
                    ['number', 'number'], // argument types
                    [ctx, buffer]); // arguments
                }
        });

        var cb = function(device) {
            var str = createDeviceString(device);
            var buffer = Module._malloc(str.length+1);
            Module.writeStringToMemory(str, buffer);
            var result = Module.ccall('chrome_priv_add_device', // name of C function
                'number', // return type
                ['number', 'number'], // argument types
                [ctx, buffer]); // arguments
        };
        chrome.usb.onDeviceAdded.addListener(cb);
    }, ctx);
}

static void chrome_exit() {
    usbi_dbg("called chrome_exit");
}

static int chrome_priv_add_device(struct libusb_context *ctx, char* js_device) {
     /* get the first token */
    char *token, *saveptr;
    token = strtok_r(js_device, "||", &saveptr);
   
    /* walk through other tokens */
    while( token != NULL ) 
    {
      printf( " %s\n", token );

      token = strtok_r(NULL, "||", &saveptr);
    }

    free(js_device);

    return 0;
}

const struct usbi_os_backend chrome_backend = {
        .name = "Chrome",
        .caps = 0,
        .init = chrome_init,
        .exit = chrome_exit,
        .get_device_list = NULL, /* not needed */
        .get_device_descriptor = darwin_get_device_descriptor,
        .get_active_config_descriptor = darwin_get_active_config_descriptor,
        .get_config_descriptor = darwin_get_config_descriptor,
        .hotplug_poll = chrome_hotplug_poll,

        .open = darwin_open,
        .close = darwin_close,
        .get_configuration = darwin_get_configuration,
        .set_configuration = darwin_set_configuration,
        .claim_interface = darwin_claim_interface,
        .release_interface = darwin_release_interface,

        .set_interface_altsetting = darwin_set_interface_altsetting,
        .clear_halt = darwin_clear_halt,
        .reset_device = darwin_reset_device,

#if InterfaceVersion >= 550
        .alloc_streams = darwin_alloc_streams,
        .free_streams = darwin_free_streams,
#endif

        .kernel_driver_active = darwin_kernel_driver_active,
        .detach_kernel_driver = darwin_detach_kernel_driver,
        .attach_kernel_driver = darwin_attach_kernel_driver,

        .destroy_device = darwin_destroy_device,

        .submit_transfer = darwin_submit_transfer,
        .cancel_transfer = darwin_cancel_transfer,
        .clear_transfer_priv = darwin_clear_transfer_priv,

        .handle_transfer_completion = darwin_handle_transfer_completion,

        .clock_gettime = darwin_clock_gettime,

        .device_priv_size = sizeof(struct darwin_device_priv),
        .device_handle_priv_size = sizeof(struct darwin_device_handle_priv),
        .transfer_priv_size = sizeof(struct darwin_transfer_priv),
};


