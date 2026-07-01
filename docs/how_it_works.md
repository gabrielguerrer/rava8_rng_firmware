# How it works

## Communication

Communication is implemented through a framed binary protocol that provides methods for
transmitting and receiving protocol messages. Incoming messages are processed using a byte-oriented
state-machine parser. For additional details, refer to `lib/RAVA_RNG/rava_comm.h`.

In the RAVA8, the USB interface is configured as a CDC device. Since most operating systems provide
native CDC drivers, communication only requires opening the device and exchanging messages
according to the protocol specification.

## Randomness Generation

Two avalanche noise channels are continuously compared using a hardware comparator IC that produces
a digital output indicating which channel has the greater instantaneous voltage. The comparator
output is connected to timer peripherals configured to count rising-edge pulses.

Single-bit generation proceeds as follows:

1. The timer counters are reset
2. The MCU performs a busy wait for the configured sampling interval
3. After the interval expires, the accumulated pulse counts are read from the timers
4. The output bit values are derived from the parity of the pulse counts:
   - Even counts produce bit 0
   - Odd counts produce bit 1

In `rng_gen_bit()` function, located in `rava8_rng.c`:
```
// Reset counters
timer1_reset_counter();
timer0_reset_counter();

// Wait for the sampling interval
device_delay_us(rng_cfg.sampling_interval);

// Measure counters
*bit_a = timer1_read_counter();
*bit_b = timer0_read_counter();

// Bit value derives from count parity
*bit_a &= 1;
*bit_b &= 1;
```

For single-byte generation, the process is repeated eight times. Each generated bit is inserted
into the corresponding position of the output byte using bit shifting.

In `rng_gen_byte()` function, located in `rava8_rng.c`:
```
uint8_t bit_lshift_mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
...

// Clear output bytes
*byte_a = 0;
*byte_b = 0;

// Bits loop
for (uint8_t i = 0; i < 8; i++)
  {
  ... // Pulse count measurements: pc_a, pc_b

  // Bit value derives from count parity
  // If odd, enable the corresponding bit in the byte output
  if (pc_a & 1) {
    *byte_a |= bit_lshift_mask[i]; // Equivalent to *byte_a |= 1 << i; but faster
  }
  if (pc_b & 1) {
    *byte_b |= bit_lshift_mask[i];
  }
```