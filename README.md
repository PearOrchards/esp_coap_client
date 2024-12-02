# esp_coap_client
A simple wrapper around libcoap, designed for use with ESP32s, but should be compatible with just about everything.

## Why?
Whenever you use libcoap, there is a significant amount of overhead and code required before you can even begin to work with requests themselves.
This simple class intends to cut down on the amount of code required, and get you working on the fun stuff faster.

I tried to emulate how Express.JS would look in C++, but to be honest it doesn't really do that. Still, I hope this helps you in some way.

## Limitations
libcoap has a million features and I've implemented basically none of them. **This class is only designed for the simplest use-cases.** For example, there is no:

- TCP Support
- Multicast Support
- TLS Support

... which you'd expect from any feature-complete CoAP library. Remember, this isn't that. It's just a simple wrapper around libcoap to get you moving faster.

## Basic Usage

```c++
#include "esp_log.h"

#include "coap3/coap.h" // You still have to import the library to make your own handlers.
#include "CoAPClient.h"

const static char *TAG = "CoAP_client";

// void *p is required for FreeRTOS tasks, can be omitted otherwise
static void client(void *p) {
    CoAPClient c;
    if (c.init()) {
        ESP_LOGI(TAG, "Client started successfully!");
    } else {
        ESP_LOGE(TAG, "Failed to start server.");
    }

    // This can be done in an anonymous manner if you wish.
    const coap_response_handler_t handler = [](coap_session_t *session, const coap_pdu_t *sent, 
                                               const coap_pdu_t *response, coap_mid_t) {
        size_t len;
        const uint8_t *data_buffer;
        size_t offset;
        size_t total;

        coap_show_pdu(COAP_LOG_INFO, response);
        if (coap_get_data_large(response, &len, &data_buffer, &offset, &total)) {
            const char* data = reinterpret_cast<const char *>(data_buffer);
            ESP_LOGI(TAG, "Data received! %.*s", len, data);
        }

        return COAP_RESPONSE_OK;
    };

    c.get("coap://10.42.0.1/hello", handler);
}
```

## Installation

First, you need to install espressif__coap into your workspace for your ESP project (if you aren't using ESPs, you can install libcoap in the regular way), which you can do with the following:
```commandline
idf.py add-dependency "espressif/coap"
```
(replace `idf.py` if you use IDF commands in a different way)

Then, clone this repository, and move it so that your project looks something like this (other structures are also likely fine but this is the one I tested with):
```commandline
project/
 |- components/
 | |- CoAPClient/
 | | |- CoAPClient.cpp
 | | |- README.md (the one you're reading!)
 | | |- ...
 | |- <any other components>
 |- main/
 | |- main.cpp
 | |- CMakeLists.txt
 | |- idf_component.yml
 | |- ...
 |- managed_components/
 | |- espressif__coap/
 | | |- ...
 |- CMakeLists.txt
 |- ...
```

### Updating CMakeLists.txt

The root `CMakeLists.txt` _should_ be okay left alone, though you may need to add something for CI. _An example was previously written here but, turns out, it doesn't work, so I've removed it._

The `main/CMakeLists.txt` should look like this (at the minimum):
```cmake
idf_component_register(SRCS main.cpp INCLUDE_DIRS "." REQUIRES CoAPClient)
```

And this repo's `CMakeLists.txt` should be left as is.

### Modifying espressif__coap
On some systems (mine included), the `espressif__coap` library is not set up correctly. You may need to modify the library itself, and make the following adjustments:
1. Navigate to the file located at This file is located at `managed_components/espressif__coap/port/include/coap3/coap.h`
2. Add this line to the top of the file:
```c++
#ifndef _COAP_H_
#define _COAP_H_

#ifdef __cplusplus
extern "C" {
#endif

// +++++++
#include "coap3/libcoap.h"
// +++++++

#if LWIP_IPV4
#define COAP_IPV4_SUPPORT 1

...
```
3. ... and remove the duplicate include file underneath struct definitions:
```c++
...
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 40
#endif /* INET6_ADDRSTRLEN */
#endif /* ! LWIP_IPV6 */

// --------
#include "coap3/libcoap.h"
// --------

#include "coap3/coap_forward_decls.h"
#include "coap3/coap_address.h"
#include "coap3/coap_async.h"
...
```

You will have the issue if, during compilation, you see errors to do with missing types, such as `coap_pdu_t` or `coap_session_t`.

_Note: the moved include statement MUST be inside `extern "C"` brackets, else you will likely get linker errors. I found this out the hard way._

## Contributing and Issues
If you have any problems using this class, please open an issue on this repository, and I'll try get back to you as soon as possible.

If you'd like to contribute, please open a pull request. I'm happy to accept any contributions, but remember the simplicity of this "library". If you're looking to add a lot of features, it may be better to fork this repository and create your own.