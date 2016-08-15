/*
 * base64.h
 *
 *  Created on: 8. velj 2016.
 *      Author: Marko
 */

#ifndef SRC_INCLUDE_DRIVER_BASE64_H_
#define SRC_INCLUDE_DRIVER_BASE64_H_

char *base64_encode2(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length);

unsigned char *base64_decode2(const char *data,
                             size_t input_length,
                             size_t *output_length);

#endif /* SRC_INCLUDE_DRIVER_BASE64_H_ */
