/* Arduino SdCard Library
 * Copyright (C) 2011..2017 by William Greiman
 *
 * This file is part of the Arduino SdCard Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdioCard_h
#define SdioCard_h
#include "SysCall.h"
#include "SdCardInterface.h"

#define FIFO_SDIO 0
#define DMA_SDIO 1
/**
 * \class SdioConfig
 * \brief SDIO card configuration.
 */
class SdioConfig {
 public:
  SdioConfig() : m_options(FIFO_SDIO) {}
  /**
   * SdioConfig constructor.
   * \param[in] opt SDIO options.
   */
  explicit SdioConfig(uint8_t opt) : m_options(opt) {}
  /** \return SDIO card options. */
  uint8_t options() {return m_options;}
  /** \return true if DMA_SDIO. */
  bool useDma() {return m_options & DMA_SDIO;}
 private:
  uint8_t m_options;
};
//------------------------------------------------------------------------------
/**
 * \class SdioCard
 * \brief Raw SDIO access to SD and SDHC flash memory cards.
 */
class SdioCard : public SdCardInterface {
 public:
  /** Initialize the SD card.
   * \param[in] sdioConfig SDIO card configuration.
   * \return true for success else false.
   */
  bool begin(SdioConfig sdioConfig);
  /** Disable an SDIO card.
   * \return false - not implemented.
   */
  bool end() {return false;}
  /**
   * Determine the size of an SD flash memory card.
   *
   * \return The number of 512 byte data sectors in the card
   *         or zero if an error occurs.
   */
  uint32_t sectorCount();
  /** Erase a range of sectors.
   *
   * \param[in] firstSector The address of the first sector in the range.
   * \param[in] lastSector The address of the last sector in the range.
   *
   * \note This function requests the SD card to do a flash erase for a
   * range of sectors.  The data on the card after an erase operation is
   * either 0 or 1, depends on the card vendor.  The card must support
   * single sector erase.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool erase(uint32_t firstSector, uint32_t lastSector);
  /**
   * \return code for the last error. See SdCardInfo.h for a list of error codes.
   */
  uint8_t errorCode() const;
  /** \return error data for last error. */
  uint32_t errorData() const;
  /** \return error line for last error. Tmp function for debug. */
  uint32_t errorLine() const;
  /**
   * Check for busy with CMD13.
   *
   * \return true if busy else false.
   */
  bool isBusy();
  /** \return the SD clock frequency in kHz. */
  uint32_t kHzSdClk();
  /**
   * Read a 512 byte sector from an SD card.
   *
   * \param[in] sector Logical sector to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readSector(uint32_t sector, uint8_t* dst);
  /**
   * Read multiple 512 byte sectors from an SD card.
   *
   * \param[in] sector Logical sector to be read.
   * \param[in] ns Number of sectors to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readSectors(uint32_t sector, uint8_t* dst, size_t ns);
  /**
   * Read a card's CID register. The CID contains card identification
   * information such as Manufacturer ID, Product name, Product serial
   * number and Manufacturing date.
   *
   * \param[out] cid pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCID(cid_t* cid);
  /**
   * Read a card's CSD register. The CSD contains Card-Specific Data that
   * provides information regarding access to the card's contents.
   *
   * \param[out] csd pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCSD(csd_t* csd);
  /** Read one data sector in a multiple sector read sequence
   *
   * \param[out] dst Pointer to the location for the data to be read.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readData(uint8_t *dst);
  /** Read OCR register.
   *
   * \param[out] ocr Value of OCR register.
   * \return true for success else false.
   */
  bool readOCR(uint32_t* ocr);
  /** Start a read multiple sectors sequence.
   *
   * \param[in] sector Address of first sector in sequence.
   *
   * \note This function is used with readData() and readStop() for optimized
   * multiple sector reads.  SPI chipSelect must be low for the entire sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStart(uint32_t sector);
  /** Start a read multiple sectors sequence.
   *
   * \param[in] sector Address of first sector in sequence.
   * \param[in] count Maximum sector count.
   * \note This function is used with readData() and readStop() for optimized
   * multiple sector reads.  SPI chipSelect must be low for the entire sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStart(uint32_t sector, uint32_t count);
  /** End a read multiple sectors sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStop();
  /** \return SDIO card status. */
  uint32_t status();
  /** \return success if sync successful. Not for user apps. */
  bool syncDevice();
  /** Return the card type: SD V1, SD V2 or SDHC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
   */
  uint8_t type() const;
  /**
   * Writes a 512 byte sector to an SD card.
   *
   * \param[in] sector Logical sector to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeSector(uint32_t sector, const uint8_t* src);
  /**
   * Write multiple 512 byte sectors to an SD card.
   *
   * \param[in] sector Logical sector to be written.
   * \param[in] ns Number of sectors to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeSectors(uint32_t sector, const uint8_t* src, size_t ns);
  /** Write one data sector in a multiple sector write sequence.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeData(const uint8_t* src);
  /** Start a write multiple sectors sequence.
   *
   * \param[in] sector Address of first sector in sequence.
   *
   * \note This function is used with writeData() and writeStop()
   * for optimized multiple sector writes.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStart(uint32_t sector);
  /** Start a write multiple sectors sequence.
   *
   * \param[in] sector Address of first sector in sequence.
   * \param[in] count Maximum sector count.
   * \note This function is used with writeData() and writeStop()
   * for optimized multiple sector writes.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStart(uint32_t sector, uint32_t count);

  /** End a write multiple sectors sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStop();

 private:
  static const uint8_t IDLE_STATE = 0;
  static const uint8_t READ_STATE = 1;
  static const uint8_t WRITE_STATE = 2;
  uint32_t m_curSector;
  uint32_t m_limitSector;
  SdioConfig m_sdioConfig;
  uint8_t m_curState;
};
#endif  // SdioCard_h
