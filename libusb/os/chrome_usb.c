#include <stdlib.h>
#include <emscripten.h>

#include "libusbi.h"

// Populate information in an internal struct for easy reference later. Also
// makes passing lots of data from JavaScript to C easier
struct chrome_device_priv {
    int session_id;
    struct libusb_device_descriptor *ddesc;
};

static int chrome_priv_add_device(struct libusb_context *ctx, struct chrome_device_priv *dev_priv);
static struct chrome_device_priv *chrome_alloc_device_priv();
static int chrome_set_device_priv_value(struct chrome_device_priv *device_priv, int key, int intVal, char *strVal);

static int chrome_set_device_priv_value(struct chrome_device_priv *device_priv, int key, int intVal, char *strVal) {
    switch (key) {
        case 0: // session_id
            device_priv->session_id = intVal;
            usbi_dbg("set device_priv session_id to %d", intVal);
            break;
        case 1: // vendor ID
            (*(device_priv->ddesc))->idVendor = (uint16_t)intVal;
            usbi_dbg("set device_priv->ddesc->idVendor to %d", intVal);
            break;
        case 2: // product ID
            (*(device_priv->ddesc))->idProduct = (uint16_t)intVal;
            usbi_dbg("set device_priv->ddesc->idProduct to %d", intVal);
            break;
        default:
            usbi_dbg("unrecognized key [%d] to set a device_priv value", key);
            return -1;
    }
    return 0;
}

static struct chrome_device_priv *chrome_alloc_device_priv() {
    struct libusb_device_descriptor *ddesc = calloc(1, sizeof(*ddesc));
    struct chrome_device_priv *dev_priv = calloc(1, sizeof(*dev_priv));

    if (dev_priv == NULL || ddesc == NULL) {
        usbi_err("chrome_priv_add_device: Unable to allocate memory");
        return -1;
    }

    dev_priv->ddesc = ddesc;
    return dev_priv;
}

static int chrome_init(struct libusb_context *ctx) {
    usbi_dbg("called chrome_init, getting current devices and registering callbacks");

    EM_ASM_({

        // Get JS values into C struct
        var allocDeviceAndSetValues = function(device) {
            var devicePtr = Module.ccall('chrome_alloc_device_priv', // name of C function
                'number', // return type
                [], // argument types
                []); // arguments
            // session id
            Module.ccall('chrome_set_device_priv_value',
                'number',
                ['number', 'number', 'number', 'string'],
                [devicePtr, 0, device.device, null]);
            // vendor id
            Module.ccall('chrome_set_device_priv_value',
                'number',
                ['number', 'number', 'number', 'string'],
                [devicePtr, 1, device.vendorId, null]);
            // product id
            Module.ccall('chrome_set_device_priv_value',
                'number',
                ['number', 'number', 'number', 'string'],
                [devicePtr, 2, device.productId, null]);

            return devicePtr;
        };

        chrome.usb.getDevices({}, function(devices) {
            var len = devices.length;
            for (var i = 0; i < len; i++) {

                var devicePtr = allocDeviceAndSetValues(devices[i]);
                
                var result = Module.ccall('chrome_priv_add_device', // name of C function
                    'number', // return type
                    ['number', 'number'], // argument types
                    [ctx, devicePtr]); // arguments
                }
        });

        var cb = function(device) {
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

static int chrome_priv_add_device(struct libusb_context *ctx, struct chrome_device_priv *dev_priv) {

    struct libusb_device *dev = NULL;
    dev = usbi_get_device_by_session_id(ctx, dev_priv->session_id);

    if (dev == NULL) {
        dev = usbi_alloc_device(ctx, dev_priv->session_id);

        // inaccessible via chrome
        dev->port_number = 0;
        dev->bus_number = 0;
        dev->device_address = 0;
        dev->parent_dev = NULL;
    }

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

        .device_priv_size = sizeof(struct chrome_device_priv),
        .device_handle_priv_size = sizeof(struct darwin_device_handle_priv),
        .transfer_priv_size = sizeof(struct darwin_transfer_priv),
};


