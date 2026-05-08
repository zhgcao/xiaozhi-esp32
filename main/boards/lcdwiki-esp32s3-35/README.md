# LCDWiki 3.5inch ESP32-S3 Display

LCDWiki 3.5寸 ESP32-S3 显示屏开发板适配

## 产品链接

https://www.lcdwiki.com/zh/3.5inch_ESP32-S3_Display

## 硬件规格

- **芯片**: ESP32-S3 N16R8 (16MB Flash + 8MB PSRAM)
- **屏幕**: 3.5寸 320×480 IPS LCD, 驱动芯片 ST77922 (QSPI接口)
- **触摸**: 电容触摸屏 (I2C地址: 0x55)
- **音频**: ES8311 编解码芯片
- **电源**: 支持电池充电管理

## IO 分配

### 显示屏 (QSPI)
| 功能 | GPIO |
|------|------|
| CS | 10 |
| SCLK | 12 |
| DATA0 | 11 |
| DATA1 | 13 |
| DATA2 | 14 |
| DATA3 | 9 |
| 背光 | 41 |

### 音频 (ES8311)
| 功能 | GPIO |
|------|------|
| I2S_MCLK | 17 |
| I2S_BCLK | 18 |
| I2S_WS | 21 |
| I2S_DOUT | 16 |
| I2S_DIN | 15 |
| PA_EN | 1 (低电平使能) |

### I2C (共用)
| 功能 | GPIO |
|------|------|
| SDA | 38 |
| SCL | 39 |

### 触摸屏
| 功能 | GPIO |
|------|------|
| INT | 47 |
| RST | 48 |
