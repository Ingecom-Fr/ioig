#pragma once

#include <stddef.h>
#include <stdio.h>
#include <memory.h>
#include <cstdint>

namespace ioig
{
    /**
     * @brief ioig Protocol Packet 
     */
    class Packet
    {
    public:
        static constexpr unsigned MAX_SIZE = 64;

        enum class Type : unsigned
        {            
            // System
            SYS_INIT = 1,
            SYS_DEINIT,
            SYS_HW_RESET,
            SYS_SW_RESET,
            SYS_GET_FW_VER,

            // GPIO
            GPIO_INIT,
            GPIO_DEINIT,
            GPIO_EVENT,
            GPIO_SET_MODE,         
            GPIO_SET_VALUE,
            GPIO_GET_VALUE,
            GPIO_SET_IRQ,
            GPIO_SET_DIR,         
            GPIO_PULSE_IN, 

            // SPI
            SPI_INIT,
            SPI_DEINIT,
            SPI_SET_FREQ,
            SPI_WRITE,
            SPI_READ,
            SPI_TRANSFER,
            SPI_SET_FORMAT,

            // I2C
            I2C_INIT,
            I2C_DEINIT,
            I2C_SET_FREQ,
            I2C_SET_TIMEOUT,
            I2C_WRITE,
            I2C_READ,
            

            // ANALOG
            ANALOG_INIT,
            ANALOG_DEINIT,
            ANALOG_WRITE,
            ANALOG_READ,
            ANALOG_READ_TEMP,

            //UART
            SERIAL_INIT,
            SERIAL_DEINIT,
            SERIAL_SET_BAUD,
            SERIAL_SET_FORMAT,
            SERIAL_SET_IRQ,
            SERIAL_SET_FLOW_CONTROL,
            SERIAL_READABLE,
            SERIAL_WRITABLE,
            SERIAL_SET_BREAK,
            SERIAL_GETC,
            SERIAL_PUTC,
            SERIAL_WRITE,
            SERIAL_READ,
            SERIAL_EVENT,
                        
            NONE = 0xFF
        };

        enum class Status: unsigned 
        {
            NONE = 0,
            CMD,
            RSP,
            ERR,
            RSP_I2C_NACK,
            RSP_I2C_TIMEOUT,
            RSP_I2C_BUF_OVERFLOW,
            RSP_SERIAL_NOT_READABLE,
            RSP_SERIAL_NOT_WRITABLE,
            RSP_SPI_BUSY,
            RSP_SPI_NOT_READABLE,
            RSP_SPI_NOT_WRITABLE,
            RSP_SPI_LEN_MISMATCH
        };
        
        Packet() : Packet(MAX_SIZE){};


        Packet(size_t pld_len)
        {
            auto maxPldLen = MAX_SIZE-Header::SIZE;

            if (pld_len > maxPldLen) 
            {
                _bufSize = MAX_SIZE;
            }else
            {
                _bufSize = Header::SIZE + pld_len;
            }
            _buffer = new uint8_t[_bufSize];
            memset(_buffer, 0, _bufSize);
        }
        
        
        ~Packet()
        {            
            delete[] _buffer;
        }

        //Copy constructor
        Packet(const Packet& other) : _bufLength(other._bufLength),
                                      _bufSize(other._bufSize)
        {
           _buffer = new uint8_t[_bufSize];
           memcpy(_buffer, other._buffer, _bufSize);
        }

        // Move Constructor
        Packet(Packet &&other) noexcept : _bufLength(other._bufLength),
                                          _bufSize(other._bufSize),
                                          _buffer(other._buffer)
        {
            other._buffer = nullptr;
            other._bufLength = 0;
            other._bufSize = 0;
        }

        // Move Assignment Operator
        Packet &operator=(Packet &&other) noexcept
        {
            if (this != &other)
            {
                delete[] _buffer;        
                _bufLength = other._bufLength;
                _buffer = other._buffer;
                _bufSize = other._bufSize;
                other._buffer = nullptr;
                other._bufLength = 0;
                other._bufSize = 0;
            }
            return *this;
        }            

        inline void reset()
        {            
            _buffer[Header::TYPE]    = static_cast<uint8_t>(Type::NONE);
            _buffer[Header::SEQ_NUM] = 0;
            _buffer[Header::PLD_LEN] = 0;
            _buffer[Header::STATUS]  = static_cast<uint8_t>(Status::NONE);  
        }

        inline void cloneHeader(const Packet &other)
        {
            _buffer[Header::TYPE] = other._buffer[Header::TYPE];
            _buffer[Header::SEQ_NUM] = other._buffer[Header::SEQ_NUM];
        }

        int getPayloadItem8(const unsigned index)
        {
            if ((index + Header::SIZE) < getBufferLength()) 
            {
                return _buffer[Header::SIZE + index];
            }            
            return -1; 
        }        

        int setPayloadItem8(const unsigned index, const uint8_t value)
        {
            if ((index + Header::SIZE) < getBufferLength()) 
            {
                _buffer[Header::SIZE + index] = value;
                return value;
            }            
            return -1; 
        }

        int addPayloadItem8(const uint8_t item)
        {
            if (getFreePayloadSlots() > 0)
            {
                _buffer[getBufferLength()] = item;
                increasePayloadLength(1);
                return item; 
            }             
            return -1;                        
        }  

        inline int addPayloadItem16(const uint16_t value) 
        {
            int ret=0;
            ret |= addPayloadItem8((value >> 8) & 0xFF);             
            ret |= addPayloadItem8(value & 0xFF);    

            return ret >= 0 ? value : -1;
        }

        inline int getPayloadItem16(const unsigned i)
        {
            if ((i + 1) < getBufferLength()) 
            { 
                uint8_t * pldBuf = &_buffer[Header::SIZE + i];
                uint16_t value = 0;             
                value |= static_cast<uint16_t>(pldBuf[0]) << 8;
                value |= static_cast<uint16_t>(pldBuf[1]);
                return value;
            }
            return -1;
        }

        inline int addPayloadItem32(const uint32_t value) 
        {
            int ret=0;
            ret |= addPayloadItem8((value >> 24) & 0xFF);
            ret |= addPayloadItem8((value >> 16) & 0xFF);
            ret |= addPayloadItem8((value >> 8) & 0xFF);
            ret |= addPayloadItem8(value & 0xFF);    
            
            return ret >= 0 ? value : -1;
        }


        inline int32_t getPayloadItem32(const unsigned i)
        {
            if ((i + 3) < getBufferLength()) 
            {
                uint8_t * pldBuf = &_buffer[Header::SIZE + i];
                uint32_t value = 0;
                value |= static_cast<uint32_t>(pldBuf[0]) << 24;
                value |= static_cast<uint32_t>(pldBuf[1]) << 16;
                value |= static_cast<uint32_t>(pldBuf[2]) << 8;
                value |= static_cast<uint32_t>(pldBuf[3]);
                return value;
            }
            return -1;
        }

        inline int64_t addPayloadItem64(const uint64_t value) 
        {
            int ret=0;
            ret |= addPayloadItem8((value >> 56) & 0xFF);
            ret |= addPayloadItem8((value >> 48) & 0xFF);
            ret |= addPayloadItem8((value >> 40) & 0xFF);
            ret |= addPayloadItem8((value >> 32) & 0xFF);
            ret |= addPayloadItem8((value >> 24) & 0xFF);
            ret |= addPayloadItem8((value >> 16) & 0xFF);
            ret |= addPayloadItem8((value >> 8) & 0xFF);
            ret |= addPayloadItem8(value & 0xFF);             
            return ret >= 0 ? value : -1;
        }

        inline int64_t getPayloadItem64(const unsigned i)
        {
            if ((i + 7) < getBufferLength()) 
            {
               uint8_t * pldBuf = &_buffer[Header::SIZE + i]; 
               uint64_t value = 0;
               value |= static_cast<uint64_t>(pldBuf[0]) << 56;
               value |= static_cast<uint64_t>(pldBuf[1]) << 48;
               value |= static_cast<uint64_t>(pldBuf[2]) << 40;
               value |= static_cast<uint64_t>(pldBuf[3]) << 32;
               value |= static_cast<uint64_t>(pldBuf[4]) << 24;
               value |= static_cast<uint64_t>(pldBuf[5]) << 16;
               value |= static_cast<uint64_t>(pldBuf[6]) << 8;
               value |= static_cast<uint64_t>(pldBuf[7]);
               return value;            
            }            
            return -1;
        }

        inline float addPayloadItemFloat(float val) 
        {
            if (getFreePayloadSlots() > sizeof(float))
            {
                memcpy(&_buffer[getBufferLength()], &val, sizeof(float));
                increasePayloadLength(sizeof(float));
                return val;
            }             
            return -1;              
        }

        inline float getPayloadItemFloat(const unsigned i) 
        {
            if ((i + sizeof(float)) < getBufferLength()) 
            { 
                float value=0;
                uint8_t * pldBuf = &_buffer[Header::SIZE + i];
                memcpy(&value, pldBuf, sizeof(float));
                return value;
            }
            return -1;            
        }        

        inline int addRepeatedPayloadItems(const uint8_t item, const unsigned n)
        {              
            if ( n <= getFreePayloadSlots() )  
            {
                auto idx = getBufferLength();                                                
                memset(_buffer + idx , item , n); 
                increasePayloadLength(n);
                return n;
            }
            return -1;            
        }
        
        inline int addPayloadBuffer(const uint8_t *buf, const unsigned len)
        {                              
            if ( len <= getFreePayloadSlots() ) 
            {               
                auto idx = getBufferLength();
                memcpy(_buffer + idx , buf , len); 
                increasePayloadLength(len);
                return len;
            }
            return -1;   
        }
        
        inline unsigned getFreePayloadSlots() const
        {
            return _bufSize - getBufferLength(); 
        }

        inline size_t getPayloadLength() const
        {
            return _buffer[Header::PLD_LEN];
        }

        inline uint8_t *getPayloadBuffer(const unsigned offset = 0)
        {
            return &_buffer[Header::SIZE + offset];
        }
      
        inline uint8_t *getBuffer()
        {            
            return _buffer;
        }

        inline size_t getBufferLength() const
        {        
            return Header::SIZE + getPayloadLength();
        }

        inline int increasePayloadLength(const unsigned len)
        {
            if (len <= getFreePayloadSlots())
            {
                auto & curLen = _buffer[Header::PLD_LEN];
                curLen += len;
                return curLen;
            }
            return -1;
        }        

        //updates the _bufLength after usb rw operation 
        inline void flush() 
        {
            _bufLength = getBufferLength();
        }        
        
        inline bool checkOk(size_t transferred_bytes) 
        {
            bool ret =
                 transferred_bytes >= getHeaderLength()  &&
                 transferred_bytes <= _bufSize;
            return ret;
        }
        
        inline bool checkErr(size_t transfer_bytes) 
        {                 
            return !checkOk(transfer_bytes);
        }        

        inline size_t getBufferSize() const
        {
            return _bufSize;
        }        
        
        inline Type getType() const 
        {
            return static_cast<Type>(_buffer[Header::TYPE]);
        }

        inline void setType(const Type t)
        {
            _buffer[Header::TYPE] = static_cast<uint8_t>(t);
        }

        inline uint8_t getSeqNum() const
        {
            return _buffer[Header::SEQ_NUM];
        }

        inline void setSeqNum(const uint8_t s)
        {
            _buffer[Header::SEQ_NUM] = static_cast<uint8_t>(s);
        }

        inline void setStatus(const Status st) 
        {
            _buffer[Header::STATUS] = static_cast<uint8_t>(st);
        }

        inline Status getStatus() const
        {
            return static_cast<Status>(_buffer[Header::STATUS]);
        }

        static inline unsigned getHeaderLength() 
        {
            return Header::SIZE;
        }        

        void print() const
        {
            // Lambda function to get Type name based on Type enum
            auto getTypeName = [](const Type type)
            {
                switch (type)
                {                    
                case Type::SYS_INIT:
                    return "SYS_INIT";
                case Type::SYS_DEINIT:
                    return "SYS_DEINIT";
                case Type::SYS_SW_RESET:
                    return "SYS_SW_RESET";
                case Type::SYS_HW_RESET:
                    return "SYS_HW_RESET";
                case Type::SYS_GET_FW_VER:
                    return "SYS_GET_FW_VER";
                case Type::GPIO_INIT:
                    return "GPIO_INIT";
                case Type::GPIO_DEINIT:
                    return "GPIO_DEINIT";
                case Type::GPIO_EVENT:
                    return "GPIO_EVENT";
                case Type::GPIO_SET_VALUE:
                    return "GPIO_SET_VALUE";
                case Type::GPIO_GET_VALUE:
                    return "GPIO_GET_VALUE";
                case Type::GPIO_SET_IRQ:
                    return "GPIO_SET_IRQ";
                case Type::GPIO_SET_DIR:
                    return "GPIO_SET_DIR";
                case Type::GPIO_SET_MODE:
                    return "GPIO_SET_MODE";
                case Type::GPIO_PULSE_IN:
                    return "GPIO_PULSE_IN";              
                case Type::SPI_INIT:
                    return "SPI_INIT";
                case Type::SPI_DEINIT:
                    return "SPI_DEINIT";
                case Type::SPI_SET_FREQ:
                    return "SPI_SET_FREQ";
                case Type::SPI_WRITE:
                    return "SPI_WRITE";
                case Type::SPI_READ:
                    return "SPI_READ";
                case Type::SPI_TRANSFER:
                    return "SPI_TRANSFER";
                case Type::SPI_SET_FORMAT:
                    return "SPI_SET_FORMAT";
                case Type::I2C_INIT:
                    return "I2C_INIT";
                case Type::I2C_DEINIT:
                    return "I2C_DEINIT";
                case Type::I2C_SET_FREQ:
                    return "I2C_SET_FREQ";
                case Type::I2C_SET_TIMEOUT: 
                    return "I2C_SET_TIMEOUT";
                case Type::I2C_WRITE:
                    return "I2C_WRITE";
                case Type::I2C_READ:
                    return "I2C_READ";
                case Type::ANALOG_INIT:
                    return "ANALOG_INIT";
                case Type::ANALOG_DEINIT:
                    return "ANALOG_DEINIT";
                case Type::ANALOG_WRITE:
                    return "ANALOG_WRITE";
                case Type::ANALOG_READ:
                    return "ANALOG_READ";    
                case Type::ANALOG_READ_TEMP:
                    return "ANALOG_READ_TEMP";
                case Type::SERIAL_INIT:
                    return "SERIAL_INIT";
                case Type::SERIAL_DEINIT:
                    return "SERIAL_DEINIT";
                case Type::SERIAL_SET_BAUD:
                    return "SERIAL_SET_BAUD";
                case Type::SERIAL_SET_FORMAT:
                    return "SERIAL_SET_FORMAT";
                case Type::SERIAL_SET_IRQ:
                    return "SERIAL_SET_IRQ";
                case Type::SERIAL_SET_FLOW_CONTROL:
                    return "SERIAL_SET_FLOW_CONTROL";
                case Type::SERIAL_READABLE:
                    return "SERIAL_READABLE";
                case Type::SERIAL_WRITABLE:
                    return "SERIAL_WRITABLE";
                case Type::SERIAL_SET_BREAK:
                    return "SERIAL_SET_BREAK";
                case Type::SERIAL_GETC:
                    return "SERIAL_GETC";
                case Type::SERIAL_PUTC:
                    return "SERIAL_PUTC";
                case Type::SERIAL_WRITE:
                    return "SERIAL_WRITE";
                case Type::SERIAL_READ:
                    return "SERIAL_READ";
                case Type::SERIAL_EVENT:
                    return "SERIAL_EVENT";
                case Type::NONE:
                    return "NONE";
                default:
                    return "UNKNOWN";
                }
            };

            // Lambda function to get Status name based on Status enum
            auto getStatusName = [](const Status st)
            {
                switch (st)
                {
                case Status::NONE:
                    return "NONE";
                case Status::CMD:
                    return "CMD";
                case Status::RSP:
                    return "RSP";         
                case Status::ERR:
                    return "ERR";
                case Status::RSP_I2C_NACK:
                    return "RSP_I2C_NACK";
                case Status::RSP_I2C_TIMEOUT:
                    return "RSP_I2C_TIMEOUT";
                case Status::RSP_I2C_BUF_OVERFLOW:
                    return "RSP_I2C_BUF_OVERFLOW";
                case Status::RSP_SERIAL_NOT_READABLE:
                    return "RSP_SERIAL_NOT_READABLE";
                case Status::RSP_SERIAL_NOT_WRITABLE:
                    return "RSP_SERIAL_NOT_WRITABLE";
                case Status::RSP_SPI_BUSY:
                    return "RSP_SPI_BUSY";
                case Status::RSP_SPI_NOT_READABLE:
                    return "RSP_SPI_NOT_READABLE";
                case Status::RSP_SPI_NOT_WRITABLE:
                    return "RSP_SPI_NOT_WRITABLE";
                case Status::RSP_SPI_LEN_MISMATCH:
                    return "RSP_SPI_LEN_MISMATCH";
                default:
                    return "UNKNOWN";
                }
            };            

            printf("Pkt: Typ: %s (0x%02x), St:%s, Buf(Sz:%d,Len:%d), SeqN: %d, PldLen: %d, Pld: ",
                   getTypeName(getType()), 
                   static_cast<int>(getType()) , 
                   getStatusName(getStatus()),
                   static_cast<int>(getBufferSize()),
                   static_cast<int>(getBufferLength()),
                   static_cast<int>(getSeqNum()),
                   static_cast<int>(getPayloadLength()));
            // Print the payload data
            for (size_t i = Header::SIZE; i < _bufSize; ++i)
            {
                printf("%02x ", static_cast<int>(_buffer[i]));

                // Add a newline character after every 32 bytes for better readability
                if ((i + 1) % 32 == 0)
                {
                    printf("\n");
                }
            }

            // Add a newline character if the last line is not complete
            if (getPayloadLength() % 32 != 0 || getPayloadLength() == 0)
            {
                printf("\n");
            }
        }

    private:        
        enum Header : unsigned
        {
            TYPE = 0,
            SEQ_NUM,
            PLD_LEN,
            STATUS,
            SIZE
        };

        size_t _bufLength;
        size_t _bufSize;
        uint8_t *_buffer;

    };
}