/**
 * Copyright (c) 2026 Gabriel Guerrer
 *
 * Distributed under the MIT license - See LICENSE for details
 */

#include <stddef.h>
#include <string.h>
#include "rava_device.h"
#include "rava_health_continuous.h"
#include "rava_rng.h"

uint16_t device_request_count = 0;
uint32_t device_gen_bytes_count = 0;

/* ===========================
 * RAVA DEVICE
 * =========================== */

/* ===========================
 * COMM
 * =========================== */

/*
Processes the ping request. Command used to verify that the device is responsive and communication
is operational.
*/
void comm_device_ping(comm_interface_t *const comm_if)
{
  // Send
  send_rava_msg_header(comm_if, CE_OK, 0, 0, NULL);
}

/*
Processes the request, returning the number of processed device requests and the total number of
generated random bytes accumulated since the previous call. After transmitting the response, both
usage counters are reset to zero.
*/
void comm_device_get_usage(comm_interface_t *const comm_if)
{
  // IO Structure
  //typedef struct {} data_in_t;
  typedef struct {uint16_t request_count; uint32_t gen_bytes_count;} data_out_t;
  //data_in_t  data_in;
  data_out_t data_out;

  // Process Output
  data_out.request_count = device_request_count;
  data_out.gen_bytes_count = device_gen_bytes_count;

  // Send
  send_rava_msg_header(comm_if, CE_OK, 0, sizeof(data_out), &data_out);

  // Reset counters
  device_request_count = 0;
  device_gen_bytes_count = 0;
}

/*
Processes the request and returns the device usage statistics, the number of continuous health-test
errors, and the requested pulse counts and random bytes for device monitoring purposes.
*/
void comm_device_monitor(comm_interface_t *const comm_if)
{
  // IO Structure
  typedef struct {uint8_t n_pulse_counts, n_bytes;} data_in_t;
  typedef struct {
    uint16_t request_count; uint32_t gen_bytes_count;
    bool has_error; health_continuous_tests_t nrc_error, nap_error;
    uint8_t n_pulse_counts, n_bytes;
  } data_out_t;
  data_in_t  data_in;
  data_out_t data_out;
  uint16_t n_bytes;

  // Input Deserialization
  if (comm_if->msg.data_len != sizeof(data_in)) {
    send_rava_msg_header(comm_if, CE_INVALID_INPUT_TYPES, 0, 0, NULL);
    return;
    }
  memcpy(&data_in, comm_if->msg.data, sizeof(data_in));

  // Validate Input

  // Process Output
  n_bytes = 2 * data_in.n_pulse_counts + 2 * data_in.n_bytes;

  // Device usage
  data_out.request_count = device_request_count;
  data_out.gen_bytes_count = device_gen_bytes_count;

  // Continuous Health tests
  data_out.has_error = health_continuous_get_errors(&data_out.nrc_error, &data_out.nap_error);

  // PCs & Bytes
  data_out.n_pulse_counts = data_in.n_pulse_counts;
  data_out.n_bytes = data_in.n_bytes;

  // Send Header
  send_rava_msg_header(comm_if, CE_OK, n_bytes, sizeof(data_out), &data_out);

  // Write PCs
  rng_write_pulse_counts(comm_if, data_in.n_pulse_counts, RNG_CORES_AB_DUAL);

  // Write Bytes
  rng_write_bytes(comm_if, data_in.n_bytes, RNG_CORES_AB_DUAL, RNG_PP_NONE);

  // Reset usage counters
  device_request_count = 0;
  device_gen_bytes_count = 0;
}