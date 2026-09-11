#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

/*
 * Default configuration.
 *
 * For local development, create:
 *
 *     network_config.local.h
 *
 * That file should NOT be committed to Git.
 */

/*
 * Load local configuration first if it exists.
 */
#if __has_include("network_config.local.h")
#include "network_config.local.h"
#endif


/*
 * Default WiFi configuration.
 *
 * These are only used if the local configuration
 * did not define the values.
 */
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif


/*
 * Default connection timeout.
 */
#ifndef WIFI_CONNECTION_TIMEOUT_MS
#define WIFI_CONNECTION_TIMEOUT_MS 15000
#endif


/*
 * Default telemetry endpoint.
 */
#ifndef TELEMETRY_ENDPOINT
#define TELEMETRY_ENDPOINT "http://YOUR_SERVER_IP:5000/api/telemetry"
#endif


#endif
