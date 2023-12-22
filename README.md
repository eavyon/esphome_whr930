# WHR930 Component for ESPHome

This is a custom component for ESPHome that allows you to use the WHR930 with ESPHome.

## Installation

Add this to your ESPHome configuration file.

```yaml
external_components:
  - source:
      type: git
      url: http://github.com/eavyon/esphome_whr930
      ref: master
    components: [ whr930 ]

uart:
  id: uart_bus
  tx_pin: 16
  rx_pin: 17
  baud_rate: 9600

whr930:
  id: whr930_component_1
```

## License

MIT License
