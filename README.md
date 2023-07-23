# WHR930 Component for ESPHome

This is a custom component for ESPHome that allows you to use the WHR930 with ESPHome.

## Installation

Copy `custom_components/whr930_component` next to your ESPHome configuration file.

```yaml
whr930_component:
  id: whr930_component_1

uart:
  id: uart_bus
  tx_pin: 16
  rx_pin: 17
  baud_rate: 9600
```

## License

MIT License
