/**
 * @addtogroup module_Common
 * @{
 */

/**
 * @file
 * @brief 消息摘要算法库。
 * @details 提供一些常用的消息摘要（指纹）算法。
 * @version 1.0.0
 * @author kim.xiejinqiang
 * @date 2012-8-5
 */

#ifndef MESSAGE_DIGEST_H_
#define MESSAGE_DIGEST_H_


extern unsigned short MessageDigest_Crc16(unsigned short crc,
        const unsigned char * data, unsigned int len);

extern unsigned short MessageDigest_Crc16Ccitt(unsigned short crc,
        const unsigned char * data, unsigned int len);

extern unsigned char MessageDigest_Crc8_31H(unsigned char crc,
        const unsigned char * data, unsigned int len);


#endif // MESSAGE_DIGEST_H_

/** @} */
