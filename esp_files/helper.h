#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstddef>

// prints the contents of a buffer
void dumpBuffers(const char *title, const uint8_t *buf, size_t start, size_t len)
{
  // *title: pointer to the name of the buffer (input a string)
  // *buf: pointer to the buffer of interest
  // start: lower index
  // len: number of bytes in the buffer
    // show title and range
    if (len == 1)
      printf("%s [%d]: ", title, start);
    // printd buffer name and index printed
    else
      printf("%s [%d-%d]: ", title, start, start + len - 1);
    // prints buffer name and range of indices printed

    // show data in the range
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", buf[start + i]);
	// %02X prints 2 hexadecimal digits, if only one digit a 0 is prepended
	// to make it 2 digits
    }
    printf("\n");
}

// compares 2 buffers and prints out differences
bool verifyAndDumpDifference(const char *a_title, const uint8_t *a_buf, size_t a_size, const char *b_title, const uint8_t *b_buf, size_t b_size)
{
  // a_title: name of buffer 'a'
  // a_buf: pointer to buffer 'a' contents
  // a_size: number of bytes in buffer 'a'
  // b_title: name of buffer 'b'
  // b_buf: pointer to buffer 'b' contents
  // b_size: number of bytes in buffer 'b'
  bool verified = true; // initialize return variable as true

    //printf("a_size: %d \n", a_size);
    //printf("b_size: %d \n", b_size);

    if (a_size != b_size) { // compares number of bytes in the buffers
        printf("received data size does not match: expected = %d / actual = %d\n", a_size, b_size);
        return false; // function terminates and returns false
    }

    for (size_t i = 0; i < a_size; i++) { // cycles through every index of buffer 'a'
        // if a_buf and b_buf is same, continue
        if (a_buf[i] == b_buf[i]) {
            continue;
        }

        verified = false; // updates return variable to be false

        // if a_buf[i] and b_buf[i] is not same, check the range that has difference
        size_t j = 1;
        //printf("j: %d \n", j);
        while (a_buf[i + j] != b_buf[i + j]) {
            j++;
            // loops until both buffers have the same data at the same index
	    // NOTE: may read beyond range of the buffer
        }
        //printf("j: %d \n", j);

        // dump the range of indices that have different data
        dumpBuffers(a_title, a_buf, i, j);
        dumpBuffers(b_title, b_buf, i, j);

        // restart from next same index (-1 considers i++ in for())
        i += j - 1;
    }
    return verified;
}

void initializeBuffers(uint8_t *tx, uint8_t *rx, size_t size, size_t offset = 0)
{
  // tx: pointer to buffer
  // rx: pointer to buffer
  // size: number of bytes in buffer
  // offset: number to start counting from
    if (tx) {
        for (size_t i = 0; i < size; i++) {
	  tx[i] = (i + offset) & 0xFF;
	  // overwrites contents of tx buffer with the offset + index within buffer
        }
    }
    if (rx) {
      memset(rx, 0, size); // fills rx buffer with 0's
    }
}
