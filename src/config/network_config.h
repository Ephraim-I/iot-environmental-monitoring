#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

/*
 * Default configuration.
 *
 * For local development, create:
 *     network_config.local.h
 *
 * That file is ignored by Git and can contain real credentials.
 */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define WIFI_CONNECTION_TIMEOUT_MS 15000

#define TELEMETRY_ENDPOINT "http://YOUR_SERVER_IP:5000/api/telemetry"

/*
 * Override the defaults when a local configuration exists.
 */
#if __has_include("network_config.local.h")
#include "network_config.local.h"
#endif

#endif
