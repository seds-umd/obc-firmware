#pragma once

/**
 * @brief Send a log message.
 * 
 * Takes about 6us to run, not including DMA or interrupt time.
 * 
 * @param msg Message string
 */
void log_msg(const char *msg);

/**
 * @brief Send a log message with formatting.
 * 
 * Arguments and formatting is the same as printf. The message length (after
 * formatting) is limited to 250 characters. Any characters after this will be
 * dropped. Takes about 10-100us, depending on the complexity of the format.
 * 
 * @param format Message and format string
 * @param ... Additional arguments
 * @return int 
 */
int log_fmt(const char *format, ...);
