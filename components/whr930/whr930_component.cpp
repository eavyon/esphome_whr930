#include "whr930_component.h"

#include "esphome/core/log.h"

namespace esphome {
namespace whr930_component {

static const char *TAG = "whr930_component.component";

void WHR930Component::setup(void) {
}

void WHR930Component::loop(void) {
}

void WHR930Component::dump_config(void) {
    ESP_LOGCONFIG(TAG, "WHR930 UART Component");
}

/// @brief Create a packet for the given command and data.
/// @details
///         Data length and checksum are automatically calculated and added to the packet.
///         Start and end bits are added as well.
///
///         A packet is build up as follow:
///
///             Start                : 2 bytes (0x07 0xF0)
///             Command              : 2 bytes
///             Number of data bytes : 1 byte
///             Data bytes           : 0-n bytes
///             Checksum             : 1 byte
///             End                  : 2 bytes (0x07 0x0F)
/// @param buffer
/// @param command
/// @param data
/// @param len
void WHR930Component::create_packet(uint8_t *buffer, uint8_t command, uint8_t *data, size_t len) {
    uint8_t bufferIndex = 0;

    buffer[bufferIndex++] = 0x07;  // default start bit
    buffer[bufferIndex++] = 0xF0;  // default start bit

    buffer[bufferIndex++] = command;

    buffer[bufferIndex++] = (uint8_t)len;
    for (size_t i = 0; i < len; ++i) {
        buffer[bufferIndex++] = data[i];
    }

    uint8_t checksum = calculate_checksum(buffer + 2, len + 2);
    buffer[bufferIndex++] = checksum;

    buffer[bufferIndex++] = 0x07;  // default end bit
    buffer[bufferIndex++] = 0x0F;  // default end bit
}

/// @brief Calculate the checksum for the given bytes.
/// @details
///     The checksum is obtained by adding all bytes (excluding start and end) plus 173 (0xAD).
///     If the stop byte value 0x07 appears twice in the data area, only one 0x07 is used for the checksum calculation.
///     If the checksum is larger than one byte, the least significant byte is used.
/// @param bytes
/// @param len
/// @return uint8_t the calculated checksum
uint8_t WHR930Component::calculate_checksum(uint8_t *bytes, size_t len) {
    uint8_t checksum = 0xAD;
    uint8_t stopByte = 0x07;
    bool foundStopByte = false;

    for (size_t i = 0; i < len; ++i) {
        uint8_t b = bytes[i];

        if ((b == stopByte && !foundStopByte) || b != stopByte) {
            checksum += b;
        }

        if (b == stopByte) {
            foundStopByte = true;
        }

        if (checksum > 0xFF) {
            checksum -= 0xFF + 1;
        }
    }

    return checksum;
}

bool WHR930Component::uart_execute_request(uint8_t command_byte, uint8_t *data_bytes, size_t data_size, uint8_t expected_response_byte, uint8_t *response_data_bytes) {
    this->uart_send_command(command_byte, data_bytes, data_size);
    return this->uart_received_ack() && this->uart_process_response(expected_response_byte, response_data_bytes);
}

bool WHR930Component::uart_execute_command(uint8_t command_byte, uint8_t *data_bytes, size_t data_size) {
    this->uart_send_command(command_byte, data_bytes, data_size);
    return this->uart_received_ack();
}

/// @brief Send the given command and data over the UART.
/// @details The command is wrapped in a packet and send over the UART.
/// @param command_byte
/// @param data_bytes
/// @param data_size
void WHR930Component::uart_send_command(uint8_t command_byte, uint8_t *data_bytes, size_t data_size) {
    uint8_t buffer[8 + data_size];
    this->create_packet(buffer, command_byte, data_bytes, data_size);

    this->uart_clear_buffer();
    this->write_array(buffer, sizeof(buffer));
    this->flush();
}

bool WHR930Component::uart_process_response(uint8_t expected_response_byte, uint8_t *response_data_bytes) {
    // check for start bytes
    if (!this->uart_wait_and_verify_byte(0x07) || !this->uart_wait_and_verify_byte(0xF0)) {
        return false;
    }

    // check for command
    uint8_t response[20];
    response[0] = 0x00;
    response[1] = expected_response_byte;
    if (!this->uart_wait_and_verify_byte(response[0]) || !this->uart_wait_and_verify_byte(response[1])) {
        return false;
    }

    // read data size
    if (!this->read_byte(&response[2])) {
        return false;
    }
    uint8_t data_size = response[2];

    // read data
    if (data_size > 0 && !this->read_array(&response[3], data_size)) {
        return false;
    }

    // validate checksum
    uint8_t checksum = calculate_checksum(response, 3 + data_size);
    if (!this->uart_wait_and_verify_byte(checksum)) {
        return false;
    }

    // check for end bytes
    if (!this->uart_wait_and_verify_byte(0x07) || !this->uart_wait_and_verify_byte(0x0F)) {
        return false;
    }

    for (int i = 0; i < data_size; i++) {
        *(response_data_bytes + i) = response[3 + i];
    }

    return true;
}

/// @brief Wait for the given byte to be available and verify it.
/// @details
///     This method will wait for the given byte to be available on the UART.
///     If the byte is available, it will be read and verified.
///     If the byte is not available or the read byte is not equal to the expected byte, false is returned.
/// @param expected_byte
/// @return bool true if the byte is available and equal to the expected byte, false otherwise
bool WHR930Component::uart_wait_and_verify_byte(uint8_t expected_byte) {
    uint32_t wait_count = 0;
    uint32_t max_wait_count = 1000;
    while (this->available() < 1 && ++wait_count < max_wait_count)
        ;

    if (this->available() < 1) {
        return false;
    }

    uint8_t received_byte;
    if (!this->peek_byte(&received_byte) || received_byte != expected_byte) {
        return false;
    }

    return this->read_byte(&received_byte);
}

bool WHR930Component::uart_received_ack(void) {
    return this->uart_wait_and_verify_byte(0x07) && this->uart_wait_and_verify_byte(0xF3);
}

void WHR930Component::uart_clear_buffer(void) {
    this->flush();

    int available = this->available();
    if (available > 0) {
        uint8_t discard_buffer[available] = {};
        this->read_array(discard_buffer, available);
    }
}

}  // namespace whr930_component
}  // namespace esphome
