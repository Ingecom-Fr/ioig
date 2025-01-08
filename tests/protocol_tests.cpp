#include <gtest/gtest.h>
#include "ioig_protocol.h"
#include <cstdint>

using namespace ioig;


TEST(PacketTestSuite, Basics) 
{
  Packet packet1;
  packet1.setType(Packet::Type::NONE);

  EXPECT_EQ(packet1.getPayloadLength() ,0);
  EXPECT_TRUE(packet1.isEmpty());
  EXPECT_EQ(packet1.getBufferLength() ,  Packet::getHeaderLength() + packet1.getPayloadLength());
  EXPECT_EQ(packet1.getBufferSize() , Packet::MAX_SIZE);
  EXPECT_EQ(packet1.getType(), Packet::Type::NONE);
  EXPECT_EQ(packet1.getSeqNum(), 0);
  EXPECT_TRUE(packet1.getBuffer() != nullptr);

  Packet packet2(5);
  packet2.cloneHeader(packet1);
  EXPECT_EQ(packet2.getType(), Packet::Type::NONE);
  EXPECT_EQ(packet2.getSeqNum(), 0);
  EXPECT_EQ(packet2.getBufferSize(), 5 + Packet::getHeaderLength());
  EXPECT_TRUE(packet2.getBuffer() != packet1.getBuffer());
}


TEST(PacketTestSuite, Object_Constructor_PktSizes) 
{
  Packet packet1(Packet::MAX_SIZE+1);
  packet1.setType(Packet::Type::GPIO_EVENT);

  EXPECT_EQ(packet1.getPayloadLength() ,0);
  EXPECT_EQ(packet1.getBufferLength() ,  Packet::getHeaderLength() + packet1.getPayloadLength());
  EXPECT_EQ(packet1.getBufferSize() , Packet::MAX_SIZE);
  EXPECT_EQ(packet1.getType(), Packet::Type::GPIO_EVENT);
  
  Packet packet2(Packet::MAX_SIZE);
  EXPECT_EQ(packet2.getPayloadLength() ,0);
  EXPECT_EQ(packet2.getBufferLength() ,  Packet::getHeaderLength() + packet2.getPayloadLength());
  EXPECT_EQ(packet2.getBufferSize() , Packet::MAX_SIZE);

  Packet packet3(Packet::MAX_SIZE-1);
  EXPECT_EQ(packet3.getPayloadLength() ,0);
  EXPECT_EQ(packet3.getBufferLength() ,  Packet::getHeaderLength() + packet3.getPayloadLength());
  EXPECT_EQ(packet3.getBufferSize() , Packet::MAX_SIZE);

  Packet packet4(0); //header-only pkt
  EXPECT_EQ(packet4.getPayloadLength() ,0);
  EXPECT_EQ(packet4.getBufferLength() ,  Packet::getHeaderLength());
  EXPECT_EQ(packet4.getBufferSize() , Packet::getHeaderLength());
  EXPECT_EQ(packet4.addPayloadItem8(4) , -1); //no space for payload items
}


TEST(PacketTestSuite, Object_CopyConstructor) 
{
  Packet packet1(34);
  packet1.setType(Packet::Type::NONE);
  packet1.setSeqNum(1);

  Packet packet2 = packet1; //copy constructor
  EXPECT_EQ( packet1.getType() , packet2.getType() );
  EXPECT_EQ( packet1.getSeqNum() , packet2.getSeqNum() );
  EXPECT_TRUE( packet1.getBuffer() != packet2.getBuffer() );

  for (size_t c = 0 ; c < packet2.getBufferSize() ; c++ ) 
  {
     auto pkt1Buf = packet1.getBuffer();
     auto pkt2Buf = packet2.getBuffer();
     EXPECT_EQ( pkt1Buf[c]  , pkt2Buf[c] );
  }
}

TEST(PacketTestSuite, Object_Move_Operatios) 
{
  Packet packet1;
  packet1.setType(Packet::Type::SPI_INIT);
  packet1.setSeqNum(4);
  packet1.setStatus(Packet::Status::CMD);

  Packet packet2(std::move(packet1)); //move constructor  
  EXPECT_EQ( packet2.getType() , Packet::Type::SPI_INIT );
  EXPECT_EQ( packet2.getSeqNum() , 4 );
  EXPECT_EQ( packet2.getBufferSize() , Packet::MAX_SIZE );
  EXPECT_EQ( packet1.getBufferSize(), 0 ); 
  EXPECT_TRUE( packet1.getBuffer() == nullptr);

  Packet packet3 = std::move(packet2); //move constructor
  EXPECT_EQ( packet3.getType() , Packet::Type::SPI_INIT );
  EXPECT_EQ( packet3.getSeqNum() , 4 );
  EXPECT_EQ( packet3.getBufferSize() , Packet::MAX_SIZE );
  EXPECT_EQ( packet2.getBufferSize(), 0 ); 
  EXPECT_TRUE( packet2.getBuffer() == nullptr);

  Packet packet4;
  packet4 = std::move(packet3); //move operator
  EXPECT_EQ( packet4.getType() , Packet::Type::SPI_INIT );
  EXPECT_EQ( packet4.getSeqNum() , 4 );
  EXPECT_EQ( packet4.getBufferSize() , Packet::MAX_SIZE );
  EXPECT_EQ( packet3.getBufferSize(), 0 ); 
  EXPECT_TRUE( packet3.getBuffer() == nullptr);
  //EXPECT_TRUE( packet3.getBufferLength() == nullptr); //this will fail because buff = nullptr

}


TEST(PacketTestSuite, Payload_BasicOperations) 
{
  const int bsz = 16;
  Packet packet1(bsz);
  packet1.setType(Packet::Type::GPIO_EVENT);
  packet1.setSeqNum(1);
  packet1.setStatus(Packet::Status::CMD);

  EXPECT_EQ(packet1.getPayloadLength() ,0);
  EXPECT_EQ(packet1.getBufferLength() ,  Packet::getHeaderLength() + packet1.getPayloadLength());
  EXPECT_EQ(packet1.getBufferSize() , bsz + Packet::getHeaderLength());
  EXPECT_EQ(packet1.getType(), Packet::Type::GPIO_EVENT);
  EXPECT_EQ(packet1.getSeqNum(), 1);
  EXPECT_EQ(packet1.getStatus(), Packet::Status::CMD);
  EXPECT_EQ(packet1.addPayloadItem8(0xAA), 0xAA);
  EXPECT_FALSE(packet1.isEmpty());
  EXPECT_EQ(packet1.addPayloadItem8(0xBB), 0xBB);  
  EXPECT_EQ(packet1.getPayloadItem8(0) , 0xAA);
  EXPECT_EQ(packet1.getPayloadItem8(1) , 0xBB);    
  EXPECT_EQ(packet1.getPayloadItem8(3) , -1); 

  EXPECT_EQ(packet1.getBufferLength() , Packet::getHeaderLength() + packet1.getPayloadLength());
  EXPECT_EQ(packet1.getPayloadLength() , 2);
  EXPECT_EQ(packet1.addPayloadItem8(0x2),0x2);
  EXPECT_EQ(packet1.addPayloadItem8(0x3),0x3);  
  EXPECT_EQ(packet1.getBufferLength() , Packet::getHeaderLength() + packet1.getPayloadLength());
  EXPECT_EQ(packet1.getPayloadLength() , 2+2);
  EXPECT_EQ(packet1.getPayloadItem8(0) , 0xAA);
  EXPECT_EQ(packet1.getPayloadItem8(1) , 0xBB);
  EXPECT_EQ(packet1.getPayloadItem8(2) , 0x2);
  EXPECT_EQ(packet1.getPayloadItem8(3) , 0x3);
  EXPECT_EQ(packet1.getPayloadItem8(4) , -1); 
  EXPECT_EQ(packet1.setPayloadItem8(3,0x18) , 0x18); 
  EXPECT_EQ(packet1.getPayloadItem8(3) , 0x18); 
  EXPECT_EQ(packet1.getPayloadLength() , 2+2);

  
  EXPECT_EQ(packet1.setPayloadItem8(4, 4), -1); //no effect
  EXPECT_EQ(packet1.setPayloadItem8(5, 5), -1); //no effect
  EXPECT_EQ(packet1.setPayloadItem8(6, 6), -1); //no effect
  EXPECT_EQ(packet1.setPayloadItem8(7, 7), -1); //no effect 

  EXPECT_EQ(packet1.getPayloadItem8(4) , -1); 
  EXPECT_EQ(packet1.getPayloadItem8(5) , -1); 
  EXPECT_EQ(packet1.getPayloadItem8(6) , -1); 
  EXPECT_EQ(packet1.getPayloadItem8(7) , -1); 

  //buffer length was not increased by the operator []
  //The operator [] is used to modify existing elements
  EXPECT_EQ(Packet::getHeaderLength() ,  2+2 ); 


  EXPECT_EQ(packet1.setPayloadItem8(255, 0xA), -1);    //no effect, out of range
  EXPECT_EQ(packet1.getPayloadItem8(81924) ,  -1 );    

  EXPECT_EQ(Packet::getHeaderLength() ,  2+2 );   

}


TEST(PacketTestSuite, Payload_PayloadItemsOps) 
{
  const int sz = 12;
  Packet packet1(sz);
  packet1.setType(Packet::Type::NONE);
  EXPECT_EQ(packet1.addRepeatedPayloadItems(0x42, 5) , 5);
  EXPECT_EQ(packet1.getPayloadItem8(0), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(1), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(2), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(3), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(4), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(5), -1);
  EXPECT_EQ(packet1.getPayloadLength() , 5);

  EXPECT_EQ(packet1.addRepeatedPayloadItems(0x43, 3) , 3);
  EXPECT_EQ(packet1.getPayloadItem8(5), 0x43);
  EXPECT_EQ(packet1.getPayloadItem8(6), 0x43);
  EXPECT_EQ(packet1.getPayloadItem8(7), 0x43);
  EXPECT_EQ(packet1.getPayloadLength() , 5+3);

  EXPECT_EQ(packet1.getPayloadLength() , 5+3);

  EXPECT_EQ(packet1.getPayloadItem8(0), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(1), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(2), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(3), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(4), 0x42);
  EXPECT_EQ(packet1.getPayloadItem8(5), 0x43);
  EXPECT_EQ(packet1.getPayloadItem8(6), 0x43);
  EXPECT_EQ(packet1.getPayloadItem8(7), 0x43);

  EXPECT_EQ(packet1.addRepeatedPayloadItems(0x44, 4) , 4);

  EXPECT_EQ(packet1.getPayloadLength() , 5+3+4);

  EXPECT_EQ(packet1.addRepeatedPayloadItems(0x44, 4) , -1); //no more place

  EXPECT_EQ(packet1.getPayloadLength() , 5+3+4);

  Packet packet2;

  EXPECT_EQ(packet2.addPayloadItem16(0x1234),0x1234);
  EXPECT_EQ(packet2.getPayloadItem8(0) , 0x12);
  EXPECT_EQ(packet2.getPayloadItem8(1) , 0x34);
  EXPECT_EQ(packet2.getPayloadItem8(2) , -1); //no data
  uint16_t val16 = packet2.getPayloadItem16(0);
  EXPECT_EQ(val16 , 0x1234);

  EXPECT_EQ(packet2.addPayloadItem32(0xab'cd'56'78), 0xab'cd'56'78);
  EXPECT_EQ(packet2.getPayloadItem8(0) , 0x12);
  EXPECT_EQ(packet2.getPayloadItem8(1) , 0x34);
  EXPECT_EQ(packet2.getPayloadItem8(2) , 0xab); 
  EXPECT_EQ(packet2.getPayloadItem8(3) , 0xcd); 
  EXPECT_EQ(packet2.getPayloadItem8(4) , 0x56); 
  EXPECT_EQ(packet2.getPayloadItem8(5) , 0x78);  
  uint32_t val32=packet2.getPayloadItem32(2); 
  EXPECT_EQ(val32 , 0xab'cd'56'78);  

  EXPECT_EQ(packet2.addPayloadItem64(0xa1'b2'c3'd4'f5'9b'6d'5f), 0xa1'b2'c3'd4'f5'9b'6d'5f);
  EXPECT_EQ(packet2.getPayloadItem8(0) , 0x12);
  EXPECT_EQ(packet2.getPayloadItem8(1) , 0x34);
  EXPECT_EQ(packet2.getPayloadItem8(2) , 0xab); 
  EXPECT_EQ(packet2.getPayloadItem8(3) , 0xcd); 
  EXPECT_EQ(packet2.getPayloadItem8(4) , 0x56); 
  EXPECT_EQ(packet2.getPayloadItem8(5) , 0x78);  
  EXPECT_EQ(packet2.getPayloadItem8(6) , 0xa1); 
  EXPECT_EQ(packet2.getPayloadItem8(7) , 0xb2); 
  EXPECT_EQ(packet2.getPayloadItem8(8) , 0xc3); 
  EXPECT_EQ(packet2.getPayloadItem8(9) , 0xd4);  
  EXPECT_EQ(packet2.getPayloadItem8(10) , 0xf5); 
  EXPECT_EQ(packet2.getPayloadItem8(11) , 0x9b); 
  EXPECT_EQ(packet2.getPayloadItem8(12) , 0x6d); 
  EXPECT_EQ(packet2.getPayloadItem8(13) , 0x5f);  
  uint64_t val64 = packet2.getPayloadItem64(6);
  EXPECT_EQ(val64, 0xa1'b2'c3'd4'f5'9b'6d'5f);
  
  EXPECT_EQ(packet2.getPayloadItem8(14) , -1); //empty

  Packet packet3;
  EXPECT_TRUE(packet3.addPayloadItem32(8000000) >= 0);  
  EXPECT_EQ(packet3.getPayloadItem32(0) , 8000000);  

  EXPECT_TRUE(packet3.addPayloadItem64(0x12'34'ab'cd'ef'56'78'4a) >= 0);
  EXPECT_EQ(packet3.getPayloadItem64(4), 0x12'34'ab'cd'ef'56'78'4a);

  EXPECT_TRUE(packet3.addPayloadItem32(90000000) >= 0);  
  EXPECT_EQ(packet3.getPayloadItem32(12) , 90000000);  

  Packet packet4;
  float pi=3.1415;
  EXPECT_EQ(packet4.addPayloadItemFloat(pi), pi);
  EXPECT_EQ(packet4.getPayloadItemFloat(0), pi);
  
  float e=2.7182;
  EXPECT_EQ(packet4.addPayloadItemFloat(e), e);
  EXPECT_EQ(packet4.getPayloadItemFloat(sizeof(float)), e);
  packet4.addPayloadItem16(0xF0F1);
  EXPECT_EQ(packet4.getPayloadItem16(2*sizeof(float)), 0xF0F1);


  char ch='c';
  Packet packet5;
  EXPECT_EQ((char)packet5.addPayloadItem8(ch), ch);
  EXPECT_EQ((char)packet5.getPayloadItem8(0), ch);

  ch = 'd';
  EXPECT_EQ((char)packet5.addPayloadItem8(ch), ch);
  EXPECT_EQ((char)packet5.getPayloadItem8(1), ch);

  ch = 'a';
  EXPECT_EQ((char)packet5.addPayloadItem8(ch), ch);
  EXPECT_EQ((char)packet5.getPayloadItem8(2), ch);

}

TEST(PacketTestSuite, Payload_addPayloadBuffer)
{
  Packet packet1(11);
  uint8_t buf[5] = {1, 2, 3, 4, 5};

  packet1.setType(Packet::Type::SPI_TRANSFER);
  packet1.addPayloadItem8(0x24);

  EXPECT_EQ(packet1.getPayloadItem8(0), 0x24);
  EXPECT_EQ(packet1.getPayloadLength(), 1);
  EXPECT_EQ(packet1.addPayloadBuffer(buf, sizeof(buf)), sizeof(buf));
  EXPECT_EQ(packet1.getPayloadLength(), 1 + 5);
  EXPECT_EQ(packet1.getPayloadItem8(0), 0x24);
  EXPECT_EQ(packet1.getPayloadItem8(1), 1);
  EXPECT_EQ(packet1.getPayloadItem8(2), 2);
  EXPECT_EQ(packet1.getPayloadItem8(3), 3);
  EXPECT_EQ(packet1.getPayloadItem8(4), 4);
  EXPECT_EQ(packet1.getPayloadItem8(5), 5);
  EXPECT_EQ(packet1.addPayloadBuffer(buf, sizeof(buf)), sizeof(buf));
  EXPECT_EQ(packet1.getPayloadLength(), 1 + 5 + 5);
  EXPECT_EQ(packet1.getPayloadItem8(0), 0x24);
  EXPECT_EQ(packet1.getPayloadItem8(1), 1);
  EXPECT_EQ(packet1.getPayloadItem8(2), 2);
  EXPECT_EQ(packet1.getPayloadItem8(3), 3);
  EXPECT_EQ(packet1.getPayloadItem8(4), 4);
  EXPECT_EQ(packet1.getPayloadItem8(5), 5);
  EXPECT_EQ(packet1.getPayloadItem8(6), 1);
  EXPECT_EQ(packet1.getPayloadItem8(7), 2);
  EXPECT_EQ(packet1.getPayloadItem8(8), 3);
  EXPECT_EQ(packet1.getPayloadItem8(9), 4);
  EXPECT_EQ(packet1.getPayloadItem8(10), 5);
  EXPECT_EQ(packet1.setPayloadItem8(0,0), 0);
  EXPECT_EQ(packet1.getPayloadItem8(0), 0);
  EXPECT_EQ(packet1.getPayloadLength(), 1 + 5 + 5);

  EXPECT_EQ(packet1.addPayloadBuffer(buf, sizeof(buf)), -1); // no more place

  EXPECT_EQ(packet1.addPayloadItem8(0x89), -1); // no more place

  Packet packet2(20);
  uint8_t buf2[20];
  for (size_t i = 0; i < sizeof(buf2); i++)
  {
    buf2[i] = i;
  }
  EXPECT_EQ(packet2.addPayloadBuffer(buf2, sizeof(buf2)), sizeof(buf2));
  for (size_t i = 0; i < sizeof(buf2); i++)
  {
    EXPECT_EQ(packet2.getPayloadItem8(i), buf2[i]);
  }

  EXPECT_EQ(packet2.addPayloadBuffer(buf2, sizeof(buf2)), -1);
}

TEST(PacketTestSuite, ProcessPacket)
{
  Packet packet1;

  auto processPkt = [](Packet &p)
  {
    p.setType(Packet::Type::GPIO_SET_MODE);
    p.setStatus(Packet::Status::CMD);
    p.setSeqNum(2);
    p.addPayloadItem8(42);
  };

  processPkt(packet1);

  EXPECT_EQ(packet1.getType(), Packet::Type::GPIO_SET_MODE);
  EXPECT_EQ(packet1.getStatus(), Packet::Status::CMD);
  EXPECT_EQ(packet1.getSeqNum(), 2);
  EXPECT_EQ(packet1.getPayloadItem8(0), 42);
  EXPECT_EQ(packet1.getPayloadLength(), 1);
}


