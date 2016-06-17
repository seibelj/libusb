#include <stdlib.h>
#include <emscripten.h>

#include "libusbi.h"

// Populate information in an internal struct for easy reference later
struct chrome_device_priv {
    int session_id;
    struct libusb_device_descriptor *ddesc;
};

static int chrome_priv_add_device(struct libusb_context *ctx, struct chrome_device_priv *dev_priv);
static struct chrome_device_priv *chrome_alloc_device_priv(int device_id);
static int chrome_set_device_priv_value(struct chrome_device_priv *device_priv, int key, int intVal, char *strVal);
static void fixme();

#define CHROME_CODE_SESSION_ID 0

static int chrome_set_device_priv_value(struct chrome_device_priv *device_priv, int key, int intVal, char *strVal) {
    switch (key) {
        case CHROME_CODE_SESSION_ID: // session_id
            device_priv->session_id = intVal;
            usbi_dbg("set device_priv session_id to %d", intVal);
            break;
        case 1: // vendor ID
            (device_priv->ddesc)->idVendor = (uint16_t)intVal;
            usbi_dbg("set device_priv->ddesc->idVendor to %d", intVal);
            break;
        case 2: // product ID
            (device_priv->ddesc)->idProduct = (uint16_t)intVal;
            usbi_dbg("set device_priv->ddesc->idProduct to %d", intVal);
            break;
        default:
            usbi_dbg("unrecognized key [%d] to set a device_priv value", key);
            return -1;
    }
    return 0;
}

static struct chrome_device_priv * EMSCRIPTEN_KEEPALIVE chrome_alloc_device_priv(int device_id) {
    struct libusb_device_descriptor *ddesc = calloc(1, sizeof(*ddesc));
    struct chrome_device_priv *dev_priv = calloc(1, sizeof(*dev_priv));

    if (dev_priv == NULL || ddesc == NULL) {
        usbi_dbg("chrome_priv_add_device: Unable to allocate memory");
        return NULL;
    }

    dev_priv->session_id = device_id;
    dev_priv->ddesc = ddesc;

    (dev_priv->ddesc)->idVendor = (uint16_t)EM_ASM_INT({
        return window.libusbDevices[$0].device.vendorId;
    }, device_id);

    printf("printfing the shit out id idVendor: %d\n", (dev_priv->ddesc)->idVendor);

    return dev_priv;
}

static int chrome_init(struct libusb_context *ctx) {
    printf("called chrome_init, getting current devices and registering callbacks: %p\n", ctx);

    EM_ASM_({

        var ctx = $0;

        // Get JS values into C struct
        /*var allocDeviceAndSetValues = function(device) {
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
        chrome.usb.onDeviceAdded.addListener(cb);*/

        chrome.usb.getDevices({}, function(devices){
            console.log("XXXXX", devices);
            window.libusbDevices = {};
            for (var device of devices) {
                window.libusbDevices[device.device] = {
                    device: device,
                    configDescriptors: null
                };
                chrome.usb.getConfigurations(device, function(cs) {
                    console.log("YYYYY", cs);
                    window.libusbDevices[device.device].configDescriptors = cs;
                    var result = Module.ccall('chrome_alloc_device_priv',
                        'number',
                        ['number',],
                        [device.device]);
                    console.log("CCALL RESULT: " + result);
                });
            }
        });

        console.log("CONTEXT", ctx)
            
    }, ctx);

    return 0;
}

static void chrome_exit() {
    usbi_dbg("called chrome_exit");
}

static void fixme() {
    return;
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

        int ret = usbi_sanitize_device(dev);
        if (ret < 0) {
          //break;
        }
    }

    return 0;
}

static int chrome_get_device_descriptor(struct libusb_device *dev, unsigned char *buffer, int *host_endian) {
    return 777;
}

static int chrome_get_active_config_descriptor(struct libusb_device *dev, unsigned char *buffer, size_t len, int *host_endian) {
    return 777;
}

static int chrome_get_config_descriptor(struct libusb_device *dev, uint8_t config_index, unsigned char *buffer, size_t len, int *host_endian) {
    return 777;
}

static void chrome_hotplug_poll (void) {

}

static int chrome_open (struct libusb_device_handle *dev_handle) {
    return 777;
}

static void chrome_close (struct libusb_device_handle *dev_handle) {

}

static int chrome_get_configuration(struct libusb_device_handle *dev_handle, int *config) {
    return 777;
}

static int chrome_set_configuration(struct libusb_device_handle *dev_handle, int config) {
    return 777;
}

static int chrome_claim_interface(struct libusb_device_handle *dev_handle, int iface) {
    return 777;
}

static int chrome_release_interface(struct libusb_device_handle *dev_handle, int iface) {
    return 777;
}

static int chrome_set_interface_altsetting(struct libusb_device_handle *dev_handle, int iface, int altsetting) {
    return 777;
}

static int chrome_clear_halt(struct libusb_device_handle *dev_handle, unsigned char endpoint) {
    return 777;
}

static int chrome_reset_device(struct libusb_device_handle *dev_handle) {
    return 777;
}

static int chrome_kernel_driver_active(struct libusb_device_handle *dev_handle, int interface) {
    return 777;
}

static int chrome_detach_kernel_driver (struct libusb_device_handle *dev_handle, int interface) {
    return 777;
}

static int chrome_attach_kernel_driver (struct libusb_device_handle *dev_handle, int interface) {
    return 777;
}

static void chrome_destroy_device(struct libusb_device *dev) {

}

static int chrome_submit_transfer(struct usbi_transfer *itransfer) {
    return 777;
}

static int chrome_cancel_transfer(struct usbi_transfer *itransfer) {
    return 777;
}

static void chrome_clear_transfer_priv (struct usbi_transfer *itransfer) {

}

static int chrome_handle_transfer_completion (struct usbi_transfer *itransfer) {
    return 777;
}

static int chrome_clock_gettime(int clk_id, struct timespec *tp) {
    return 777;
}

const struct usbi_os_backend chrome_backend = {
        .name = "Chrome",
        .caps = 0,
        .init = chrome_init,
        .exit = chrome_exit,
        .get_device_list = NULL, /* not needed */
        .get_device_descriptor = chrome_get_device_descriptor,// TODO
        .get_active_config_descriptor = chrome_get_active_config_descriptor,// TODO
        .get_config_descriptor = chrome_get_config_descriptor,// TODO
        .hotplug_poll = chrome_hotplug_poll,// TODO

        .open = chrome_open,// TODO
        .close = chrome_close,// TODO
        .get_configuration = chrome_get_configuration,// TODO
        .set_configuration = chrome_set_configuration,// TODO
        .claim_interface = chrome_claim_interface,// TODO
        .release_interface = chrome_release_interface,// TODO

        .set_interface_altsetting = chrome_set_interface_altsetting,// TODO
        .clear_halt = chrome_clear_halt,// TODO
        .reset_device = chrome_reset_device,// TODO

//#if InterfaceVersion >= 550
//        .alloc_streams = fixme,//darwin_alloc_streams,
//        .free_streams = fixme,//darwin_free_streams,
//#endif

        .kernel_driver_active = chrome_kernel_driver_active,// TODO
        .detach_kernel_driver = chrome_detach_kernel_driver,// TODO
        .attach_kernel_driver = chrome_attach_kernel_driver,// TODO

        .destroy_device = chrome_destroy_device,// TODO

        .submit_transfer = chrome_submit_transfer,// TODO
        .cancel_transfer = chrome_cancel_transfer,// TODO
        .clear_transfer_priv = chrome_clear_transfer_priv,// TODO

        .handle_transfer_completion = chrome_handle_transfer_completion,// TODO

        .clock_gettime = chrome_clock_gettime,// TODO

        .device_priv_size = sizeof(struct chrome_device_priv),
        .device_handle_priv_size = 0,//sizeof(struct darwin_device_handle_priv),
        .transfer_priv_size = 0,//sizeof(struct darwin_transfer_priv),
};


