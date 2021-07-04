#include <jni.h>
#include <string>
#include <iostream>
#include <unistd.h>

#include <cstdint>
#include <cstdio>

const uint32_t blockSize = 128; // words
const uint32_t chunksize = 128 * 4; // bytes
const uint32_t delta = 0x9e3779b9;

static const uint32_t key[] = {
        0x8ac14c27,
        0x42845ac1,
        0x136506bb,
        0x05d47c66,
};

static const char EXT_LKF[] = ".lkf", EXT_MP3[] = ".mp3";

/**/
uint32_t calcKey(uint32_t leftWord, uint32_t rightWord, uint32_t r, uint32_t k) {
    uint32_t n1 = (leftWord>>5 ^ rightWord<<2) + (rightWord>>3 ^ leftWord<<4);
    uint32_t n2 = (key[(r>>2^k)&3] ^ leftWord) + (r ^ rightWord);
    return n1 ^ n2;
}

extern "C" JNIEXPORT jstring JNICALL Java_com_jetteam_hansolo_MainActivity_stringFromJNI(JNIEnv *env, jobject /* this */) {
    char buff[FILENAME_MAX];
    getcwd(buff, FILENAME_MAX);
    printf("Current working dir: %s\n", buff);

    static const char srcPath[] = "/data/data/com.jetteam.hansolo/files/0012.lkf";
    static const char destPath[] = "/data/data/com.jetteam.hansolo/files/0012.mp3";
    static FILE *srcFile = nullptr;
    static FILE *destFile = nullptr;

    // Start decoding
    std::string hello;

    unsigned char buffer[chunksize];
    uint32_t block[blockSize];

    size_t bytesRead = 0;
    uint32_t chunksRead = 0;

    srcFile = fopen(srcPath, "rb");
    if (srcFile != nullptr) {
        hello = "src file found";
        destFile = fopen(destPath, "wb");
        if (destFile != nullptr) {
            printf("File \"%s\" opened!\n", destPath);

            // read up to sizeof(buffer) bytes
            while ( (bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0 ) {
                chunksRead++;
                if (bytesRead == chunksize) {
                    // Convert bytearray to unsigned ints array (Little-endian order)
                    for(int iter = 0, kter = 0; iter < blockSize*4; iter += 4, kter++) {
                        block[kter] = buffer[iter]|buffer[iter+1]<<8|buffer[iter+2]<<16|buffer[iter+3]<<24;
                    }
                    // Do a 3-round decryption for XXTEA algorhitm
                    for (int cycleround = 3; cycleround > 0; cycleround--) {
                        for (int k = blockSize-1; k >= 0; k--) {
                            block[k] -= calcKey(block[(k-1)&(blockSize-1)], block[(k+1)&(blockSize-1)], cycleround*delta, (uint32_t)k);
                        }
                    }
                    // Write out decrypted blocks
                    fwrite(block, 4, blockSize, destFile);
                } else {
                    // Loaded chunk is less than block sequence - write it unencrypted
                    fwrite(buffer, 1, bytesRead, destFile);
                }
            }

            fclose(destFile);
        } else {
            hello = "dest file not found!";
        }

        fclose(srcFile);
    } else {
        hello = "src file not found!";
    }

    return env->NewStringUTF(hello.c_str());
}
