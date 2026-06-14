#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stddef.h>

void parser_neon_scan(const uint8_t* stream, size_t length);

#endif
