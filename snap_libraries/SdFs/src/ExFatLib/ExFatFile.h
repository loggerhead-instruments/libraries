/* ExFat Library
 * Copyright (C) 2016..2017 by William Greiman
 *
 * This file is part of the ExFat Library
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
 * along with the ExFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef ExFatFile_h
#define ExFatFile_h
/**
 * \file
 * \brief ExFatFile class
 */
#include <limits.h>
#include <string.h>
#include "ExFatConfig.h"
#include "../common/FsDateTime.h"
#include "../common/FsStructs.h"
#include "../common/FsApiConstants.h"
#include "ExFatTypes.h"
#include "ExFatPartition.h"

class ExFatVolume;

//------------------------------------------------------------------------------
/** Expression for path name separator. */
#define isDirSeparator(c) ((c) == '/')
//------------------------------------------------------------------------------
/** test for legal character.
 *
 * \param[in] c character to be tested.
 *
 * \return true for legal character else false.
 */
inline bool lfnLegalChar(ExChar_t c) {
  if (c == '/' || c == '\\' || c == '"' || c == '*' ||
      c == ':' || c == '<' || c == '>' || c == '?' || c == '|') {
    return false;
  }
#if USE_UNICODE_NAMES
  return 0X1F < c;
#else  // USE_UNICODE_NAMES
  return 0X1F < c && c < 0X7F;
#endif  // USE_UNICODE_NAMES
}
//-----------------------------------------------------------------------------
/**
 * \struct ExName_t
 * \brief Internal type for file name - do not use in user apps.
 */
struct ExName_t {
  /** length of Long File Name */
  size_t len;
  /** Long File Name start. */
  const ExChar_t* lfn;
};
//------------------------------------------------------------------------------
/**
 * \struct ExFatPos_t
 * \brief Internal type for file position - do not use in user apps.
 */
struct ExFatPos_t {
  /** stream position */
  uint64_t position;
  /** cluster for position */
  uint32_t cluster;
  ExFatPos_t() : position(0), cluster(0) {}
};
//-----------------------------------------------------------------------------
/**
 * \class ExFatFile
 * \brief Basic file class.
 */
class ExFatFile {
 public:
  ExFatFile() : m_attributes(FILE_ATTR_CLOSED), m_error(0) {}

  /** The parenthesis operator.
    *
    * \return true if a file is open.
    */
  operator bool() {
    return isOpen();
  }
  /** \return The number of bytes available from the current position
   * to EOF for normal files.  Zero is returned for directory files.
   */
  uint64_t available64() {
    return isFile() ? fileSize() - curPosition() : 0;
  }
  /** \return The number of bytes available from the current position
   * to EOF for normal files.  INT_MAX is returned for very large files.
   *
   * available64() is recomended for very large files.
   *
   * Zero is returned for directory files.
   *
   */
  int available() {
    uint64_t n = available64();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Close a file and force cached data and directory information
   *  to be written to the storage device.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool close();
  /** \return The current position for a file or directory. */
  uint64_t curPosition() const {return m_curPosition;}

  /** \return Total data length for file. */
  uint64_t dataLength() {return m_dataLength;}
  /** Test for the existence of a file in a directory
   *
   * \param[in] path Path of the file to be tested for.
   *
   * The calling instance must be an open directory file.
   *
   * dirFile.exists("TOFIND.TXT") searches for "TOFIND.TXT" in  the directory
   * dirFile.
   *
   * \return true if the file exists else false.
   */
  bool exists(const ExChar_t* path) {
    ExFatFile file;
    return file.open(this, path, O_READ);
  }
  /** get position for streams
   * \param[out] pos struct to receive position
   */
  void fgetpos(fspos_t* pos);
 /**
   * Get a string from a file.
   *
   * fgets() reads bytes from a file into the array pointed to by \a str, until
   * \a num - 1 bytes are read, or a delimiter is read and transferred to \a str,
   * or end-of-file is encountered. The string is then terminated
   * with a null byte.
   *
   * fgets() deletes CR, '\\r', from the string.  This insures only a '\\n'
   * terminates the string for Windows text files which use CRLF for newline.
   *
   * \param[out] str Pointer to the array where the string is stored.
   * \param[in] num Maximum number of characters to be read
   * (including the final null byte). Usually the length
   * of the array \a str is used.
   * \param[in] delim Optional set of delimiters. The default is "\n".
   *
   * \return For success fgets() returns the length of the string in \a str.
   * If no data is read, fgets() returns zero for EOF or -1 if an error occurred.
   */
  int fgets(char* str, int num, char* delim = nullptr);
  /** \return The total number of bytes in a file. */
  uint64_t fileSize() {return m_validLength;}
  /** set position for streams
   * \param[out] pos struct with value for new position
   */
  void fsetpos(fspos_t* pos);
  /** Arduino name for sync() */
  void flush() {sync();}
  /**
   * Get a file's name followed by a zero byte.
   *
   * \param[out] name An array of characters for the file's name.
   * \param[in] size The size of the array in characters.
   * \return the name length.
   */
  size_t getName(ExChar_t *name, size_t size);
  /** Clear all error bits. */
  void clearError() {
    m_error = 0;
  }
  /** Set writeError to zero */
  void clearWriteError() {
    m_error &= ~WRITE_ERROR;
  }
  /** \return All error bits. */
  uint8_t getError() {
    return m_error;
  }
  /** \return value of writeError */
  bool getWriteError() {
    return m_error & WRITE_ERROR;
  }
  /** \return True if the file is contiguous. */
  bool isContiguous() const {return m_flags & FILE_FLAG_CONTIGUOUS;}
  /** \return True if this is a directory. */
  bool isDir() const  {return m_attributes & FILE_ATTR_DIR;}
  /** \return True if this is a normal file. */
  bool isFile() const {return m_attributes & FILE_ATTR_FILE;}
  /** \return True if this is a hidden. */
  bool isHidden() const {return m_attributes & FILE_ATTR_HIDDEN;}
  /** \return true if the file is open. */
  bool isOpen() const {return m_attributes;}
  /** \return True if file is read-only */
  bool isReadOnly() const {return m_attributes & FILE_ATTR_READ_ONLY;}
  /** \return True if this is a subdirectory. */
  bool isSubDir() const {return m_attributes & FILE_ATTR_SUBDIR;}
  /** \return True if this is the root directory. */
  bool isRoot() const {return m_attributes & FILE_ATTR_ROOT;}
  /** List directory contents.
   *
   * \param[in] pr Print stream for list.
   *
   */
  void ls(print_t* pr);
  /** List directory contents.
   *
   * \param[in] pr Print stream for list.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   *
   * \param[in] indent Amount of space before file name. Used for recursive
   * list to indicate subdirectory level.
   */
  void ls(print_t* pr, uint8_t flags, uint8_t indent = 0);
  /** Make a new directory.
   *
   * \param[in] parent An open directory file that will
   *                   contain the new directory.
   *
   * \param[in] path A path with a valid name for the new directory.
   *
   * \param[in] pFlag Create missing parent directories if true.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool mkdir(ExFatFile* parent, const ExChar_t* path, bool pFlag = true);
  /** Open a file or directory by name.
   *
   * \param[in] dirFile An open directory containing the file to be opened.
   *
   * \param[in] path The path for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   *                  bitwise-inclusive OR of flags from the following list
   *
   * O_READ - Open for reading.
   *
   * O_RDONLY - Same as O_READ.
   *
   * O_WRITE - Open for writing.
   *
   * O_WRONLY - Same as O_WRITE.
   *
   * O_RDWR - Open for reading and writing.
   *
   * O_APPEND - If set, the file offset shall be set to the end of the
   * file prior to each write.
   *
   * O_AT_END - Set the initial position at the end of the file.
   *
   * O_CREAT - If the file exists, this flag has no effect except as noted
   * under O_EXCL below. Otherwise, the file shall be created
   *
   * O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
   *
   * O_SYNC - Call sync() after each write.  This flag should not be used with
   * write(uint8_t) or any functions do character at a time writes since sync()
   * will be called after each byte.
   *
   * O_TRUNC - If the file exists and is a regular file, and the file is
   * successfully opened and is not read only, its length shall be truncated to 0.
   *
   * WARNING: A given file must not be opened by more than one file object
   * or file corruption may occur.
   *
   * \note Directory files must be opened read only.  Write and truncation is
   * not allowed for directory files.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(ExFatFile* dirFile, const ExChar_t* path, uint8_t oflag);
  /** Open a file in the volume root directory.
   *
   * \param[in] vol Volume where the file is located.
   *
   * \param[in] path with a valid name for a file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see open(ExFatFile*, const char*, uint8_t).
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(ExFatVolume* vol, const ExChar_t* path, int oflag);

  /** Open the next file or subdirectory in a directory.
   *
   * \param[in] dirFile An open instance for the directory
   *                    containing the file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see open(ExFatFile*, const char*, uint8_t).
   *
   * \return true for success or false for failure.
   */
  bool openNext(ExFatFile* dirFile, uint8_t oflag = O_READ);
  /** Open a file in the current working directory.
   *
   * \param[in] path A path with a valid name for a file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool open(const ExChar_t* path, int oflag = O_READ);

  /** Open a volume's root directory.
   *
   * \param[in] vol The FAT volume containing the root directory to be opened.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool openRoot(ExFatVolume* vol);
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek();
  /** Allocate contiguous clusters to an empty file.
   *
   * The file must be empty with no clusters allocated.
   *
   * The file will have zero validLength and dataLength
   * will equal the requested length.
   * 
   * \param[in] length size of allocated space in bytes.
   * \return true for success else false.
   */
  bool preAllocate(uint64_t length);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(int16_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(uint16_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(int32_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(uint32_t value, char term);

  /** Print a file's size in bytes.
   * \param[in] pr Prtin stream for the output.
   * \return The number of bytes printed.
   */
  size_t printFileSize(print_t* pr);
  /** Formatted print.
   *
   * \param[in] fmt format string.
   *
   * \return number of character printed for success else a negative value.
   */
  int printf(const char* fmt, ...);
  /** Formatted print.
   *
   * \param[in] fmt format string.
   *
   * \return number of character printed for success else a negative value.
   */
  int mprintf(const char* fmt, ...);
#if ENABLE_ARDUINO_FEATURES
  /** Minimal formatted print.
   *
   * \param[in] ifsh format string in flash using F() macro.
   *
   * \return number of character printed for success else a negative value.
   */
  int mprintf(const __FlashStringHelper *ifsh, ...);
#endif  // ENABLE_ARDUINO_FEATURES
  /** Print a file's modify date and time
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  size_t printCreateDateTime(print_t* pr);

  /** Print a file's modify date and time
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  size_t printModifyDateTime(print_t* pr);
  /** Print a file's name
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  size_t printName(print_t* pr);
  /** Read the next byte from a file.
   *
   * \return For success read returns the next byte in the file as an int.
   * If an error occurs or end of file is reached -1 is returned.
   */
  int read() {
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
  }
  /** Read data from a file starting at the current position.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] count Maximum number of bytes to read.
   *
   * \return For success read() returns the number of bytes read.
   * A value less than \a nbyte, including zero, will be returned
   * if end of file is reached.
   * If an error occurs, read() returns -1.
   */
  int read(void* buf, size_t count);
  /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool remove();
  /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \param[in] path Path for the file to be removed.
   *
   * Example use: dirFile.remove(filenameToRemove);
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool remove(const ExChar_t* path);
   /** Rename a file or subdirectory.
   *
   * \param[in] newPath New path name for the file/directory.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rename(const ExChar_t* newPath);
   /** Rename a file or subdirectory.
   *
   * \param[in] dirFile Directory for the new path.
   * \param[in] newPath New path name for the file/directory.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rename(ExFatFile* dirFile, const ExChar_t* newPath);
  /** Set the file's current position to zero. */
  void rewind() {
    seekSet(0);
  }
  /** Remove a directory file.
   *
   * The directory file will be removed only if it is empty and is not the
   * root directory.  rmdir() follows DOS and Windows and ignores the
   * read-only attribute for the directory.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * directory that has a long name. For example if a directory has the
   * long name "New folder" you should not delete the 8.3 name "NEWFOL~1".
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool rmdir();
  /** Set the files position to current position + \a pos. See seekSet().
   * \param[in] offset The new position in bytes from the current position.
   * \return true for success or false for failure.
   */
  bool seekCur(int64_t offset) {
    return seekSet(m_curPosition + offset);
  }
  /** Set the files position to end-of-file + \a offset. See seekSet().
   * Can't be used for directory files since file size is not defined.
   * \param[in] offset The new position in bytes from end-of-file.
   * \return true for success or false for failure.
   */
  bool seekEnd(int64_t offset = 0) {
    return isFile() ? seekSet(m_validLength + offset) : false;
  }
  /** Sets a file's position.
   *
   * \param[in] pos The new position in bytes from the beginning of the file.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool seekSet(uint64_t pos);
  /** The sync() call causes all modified data and directory fields
   * to be written to the storage device.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool sync();
  /** Truncate a file at the current file position.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool truncate();
   /** Truncate a file to a specified length.  The current file position
   * will be set to end of file.
   *
   * \param[in] length The desired length for the file.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool truncate(uint64_t length) {
    return seekSet(length) && truncate();
  }

  /** \return The valid number of bytes in a file. */
  uint64_t validLength() {return m_validLength;}
  /** Write a string to a file. Used by the Arduino Print class.
   * \param[in] str Pointer to the string.
   * Use getWriteError to check for errors.
   * \return count of characters written for success or -1 for failure.
   */
  int write(const char* str) {
    return write(str, strlen(str));
  }
  /** Write a single byte.
   * \param[in] b The byte to be written.
   * \return +1 for success or zero for failure.
   */
  size_t write(uint8_t b) {return write(&b, 1);}
  /** Write data to an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] count Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a count.
   */
  size_t write(const void* buf, size_t count);
  //============================================================================
#if USE_UNICODE_NAMES
  // Not Implemented when Unicode is selected.
  bool exists(const char* path);
  size_t getName(char *name, size_t size);
  bool mkdir(ExFatFile* parent, const char* path, bool pFlag = true);
  bool open(ExFatVolume* vol, const char* path, int oflag);
  bool open(ExFatFile* dir, const char* path, int oflag);
  bool open(const char* path, int oflag = O_READ);
  bool remove(const char* path);
  bool rename(const char* newPath);
  bool rename(ExFatFile* dirFile, const char* newPath);
#endif  // USE_UNICODE_NAMES

 private:
  friend class ExFatVolume;
  bool addCluster();
  bool addDirCluster();
  uint8_t setCount() {return m_setCount;}
  bool mkdir(ExFatFile* parent, ExName_t* fname);
  bool openRootFile(ExFatFile* dir,
                    const ExChar_t* name, uint8_t nameLength, uint8_t oflag);
  bool open(ExFatFile* dirFile, ExName_t* fname, uint8_t oflag) {
    return openRootFile(dirFile, fname->lfn, fname->len, oflag);
  }
  bool parsePathName(const ExChar_t* path,
                            ExName_t* fname, const ExChar_t** ptr);
  uint32_t curCluster() const {return m_curCluster;}
  ExFatVolume* volume() const {return m_vol;}
  bool syncDir();
  //----------------------------------------------------------------------------
  static const uint8_t WRITE_ERROR = 0X1;
  static const uint8_t READ_ERROR  = 0X2;

  /** This file has not been opened. */
  static const uint8_t FILE_ATTR_CLOSED = 0;
  /** File is read-only. */
  static const uint8_t FILE_ATTR_READ_ONLY = EXFAT_ATTRIB_READ_ONLY;
  /** File should be hidden in directory listings. */
  static const uint8_t FILE_ATTR_HIDDEN = EXFAT_ATTRIB_HIDDEN;
  /** Entry is for a system file. */
  static const uint8_t FILE_ATTR_SYSTEM = EXFAT_ATTRIB_SYSTEM;
  /** Entry for normal data file */
  static const uint8_t FILE_ATTR_FILE = 0X08;
  /** Entry is for a subdirectory */
  static const uint8_t FILE_ATTR_SUBDIR = EXFAT_ATTRIB_DIRECTORY;
  static const uint8_t FILE_ATTR_ARCHIVE = EXFAT_ATTRIB_ARCHIVE;
  /** Root directory */
  static const uint8_t FILE_ATTR_ROOT = 0X40;
  /** Directory type bits */
  static const uint8_t FILE_ATTR_DIR = FILE_ATTR_SUBDIR | FILE_ATTR_ROOT;
  /** Attributes to copy from directory entry */
  static const uint8_t FILE_ATTR_COPY = EXFAT_ATTRIB_READ_ONLY |
                       EXFAT_ATTRIB_HIDDEN | EXFAT_ATTRIB_SYSTEM |
                       EXFAT_ATTRIB_DIRECTORY | EXFAT_ATTRIB_ARCHIVE;

  static const uint8_t FILE_FLAG_OFLAG = (O_ACCMODE | O_APPEND | O_SYNC);
  static const uint8_t FILE_FLAG_CONTIGUOUS  = 0X40;
  static const uint8_t FILE_FLAG_DIR_DIRTY = 0X80;

  uint64_t   m_curPosition;
  uint64_t   m_dataLength;
  uint64_t   m_validLength;
  uint32_t   m_curCluster;
  uint32_t   m_firstCluster;
//  ExFatPartition*  m_vol;
  ExFatVolume*  m_vol;
  DirPos_t   m_dirPos;
  uint8_t    m_setCount;
  uint8_t    m_attributes;
  uint8_t    m_error;
  uint8_t    m_flags;
};
#include "../common/ArduinoFiles.h"
/**
 * \class ExFile
 * \brief exFAT file with Arduino Stream.
 */
class ExFile : public StreamFile<ExFatFile, uint64_t> {
 public:
   /** Opens the next file or folder in a directory.
   *
   * \param[in] mode open mode flags.
   * \return a FatStream object.
   */
  ExFile openNextFile(uint8_t mode = O_READ) {
    ExFile tmpFile;
    tmpFile.openNext(this, mode);
    return tmpFile;
  }
};
#endif  // ExFatFile_h
