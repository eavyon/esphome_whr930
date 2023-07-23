#pragma once

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace whr930_component {

class WHR930Component : public uart::UARTDevice, public Component {
   public:
    void setup(void) override;
    void loop(void) override;
    void dump_config(void) override;

   private:
    void create_packet(uint8_t *buffer, uint8_t command, uint8_t *data, size_t len);
    uint8_t calculate_checksum(uint8_t *bytes, size_t len);

    bool uart_execute_request(uint8_t command_byte, uint8_t *data_bytes, size_t data_size, uint8_t expected_response_byte, uint8_t *response_data_bytes);
    bool uart_execute_command(uint8_t command_byte, uint8_t *data_bytes, size_t data_size);
    void uart_send_command(uint8_t command_byte, uint8_t *data_bytes, size_t data_size);
    bool uart_process_response(uint8_t expected_response_byte, uint8_t *response_data_bytes);
    bool uart_wait_and_verify_byte(uint8_t expected_byte);
    bool uart_received_ack(void);
    void uart_clear_buffer(void);
};
}  // namespace whr930_component
}  // namespace esphome
