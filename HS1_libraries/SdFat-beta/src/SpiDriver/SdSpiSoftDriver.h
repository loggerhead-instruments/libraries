/**
 * Copyright (c) 2011-2019 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
 * \file
 * \brief Class for software SPI.
 */
#ifndef SdSpiSoftDriver_h
#define SdSpiSoftDriver_h
#include "../DigitalIO/SoftSPI.h"
/**
 * \class SoftSPIClass
 * \brief Base class for external soft SPI.
 */
class SoftSPIClass {
 public:
  /** Initialize the SPI bus. */
  virtual void begin() = 0;
  /** Receive a byte.
   *
   * \return The byte.
   */
  virtual uint8_t receive() = 0;
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  virtual void send(uint8_t data) = 0;
};
//------------------------------------------------------------------------------
/**
 * \class SoftSpiDriver
 * \brief Class for external soft SPI.
 */
template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin>
class SoftSpiDriver : public SoftSPIClass {
 public:
  /** Initialize the SPI bus. */
  void begin() {m_spi.begin();}
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {return m_spi.receive();}
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {m_spi.send(data);}
 private:
  SoftSPI<MisoPin, MosiPin, SckPin, 0> m_spi;
};

//------------------------------------------------------------------------------
/**
 * \class SdSoftSpiDriver
 * \brief Class for SdSoftSpiDriver.
 */
class SdSoftSpiDriver {
 public:
  /** Activate SPI hardware. */
  void activate() {}
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig) {
    m_spi = spiConfig.spiPort;
    if (m_spi) m_spi->begin();
  }
  /** Deactivate SPI hardware. */
  void deactivate() {}
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {return m_spi ? m_spi->receive() : 0XFF;}
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] count Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      buf[i] = receive();
    }
    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {if (m_spi) m_spi->send(data);}
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] count Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      send(buf[i]);
    }
  }
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] maxSck Maximum SCK frequency.
   */
  void setSckSpeed(uint32_t maxSck) {
    (void)maxSck;
  }

 private:
  SoftSPIClass* m_spi;
};
/** Typedef for use of SdSoftSpiDriver */
typedef SdSoftSpiDriver SdSpiDriver;
#endif  // SdSpiSoftDriver_h
