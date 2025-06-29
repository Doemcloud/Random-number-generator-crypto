#include <iostream>
#include <vector>
#include <random>
#include <cstdint>

using namespace std;

const uint64_t speckKeySize = 128;
const uint64_t speckBlockSize = 128;
const uint64_t speckRounds = 32;

vector<uint8_t> generateRandomKey(size_t keySize) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<uint8_t> dis(0, 255);

    vector<uint8_t> key(keySize);
    for (size_t i = 0; i < keySize; ++i) {
        key[i] = dis(gen);
    }
    return key;
}

uint64_t bytesToUint64(const vector<uint8_t>& bytes, size_t offset) {
    uint64_t result = 0;
    for (size_t i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(bytes[offset + i]) << (8 * i);
    }
    return result;
}

vector<uint8_t> uint64ToBytes(uint64_t value) {
    vector<uint8_t> bytes(8);
    for (size_t i = 0; i < 8; ++i) {
        bytes[i] = (value >> (8 * i)) & 0xFF;
    }
    return bytes;
}

void speckRound(uint64_t& x, uint64_t& y, uint64_t k) {
    x = (x >> 8) | ((x << 56) & 0xFFFFFFFFFFFFFFFF);
    x += y;
    x ^= k;
    y = (y << 3) | ((y >> 61) & 0xFFFFFFFFFFFFFFFF);
    y ^= x;
}

vector<uint8_t> speckEncrypt(const vector<uint8_t>& plaintext, const vector<uint8_t>& key) {
    uint64_t x = bytesToUint64(plaintext, 0);
    uint64_t y = bytesToUint64(plaintext, 8);

    vector<uint64_t> roundKeys(speckRounds);
    roundKeys[0] = bytesToUint64(key, 0);

    for (size_t i = 1; i < speckRounds; ++i) {
        uint64_t prevKey = roundKeys[i - 1];
        uint64_t prevKeyPart = bytesToUint64(key, i * 8);
        roundKeys[i] = (prevKey >> 8) | ((prevKey << 56) & 0xFFFFFFFFFFFFFFFF);
        roundKeys[i] += prevKeyPart;
    }

    for (size_t i = 0; i < speckRounds; ++i) {
        speckRound(x, y, roundKeys[i]);
    }

    vector<uint8_t> ciphertext(16);
    vector<uint8_t> xBytes = uint64ToBytes(x);
    vector<uint8_t> yBytes = uint64ToBytes(y);
    copy(xBytes.begin(), xBytes.end(), ciphertext.begin());
    copy(yBytes.begin(), yBytes.end(), ciphertext.begin() + 8);
    return ciphertext;
}

class Speck{
private:
    vector<uint8_t> key;
    vector<uint8_t> iv;
    vector<uint8_t> state;

public:

    Speck(const vector<uint8_t>& key, const vector<uint8_t>& iv)
        : key(key), iv(iv), state(iv) {}

    uint8_t generateByte() {
        vector<uint8_t> encryptedState = speckEncrypt(state, key);
        state = encryptedState;
        return encryptedState[0];
    }

    uint32_t generateNum() {
        uint32_t result = 0;
        for (int i = 0; i < 4; ++i) {
            result = (result << 8) | generateByte();
        }
        return result;
    }
};

int main() {

    vector<uint8_t> key = generateRandomKey(speckKeySize / 8);
    vector<uint8_t> iv = generateRandomKey(speckBlockSize / 8);

    Speck prng(key, iv);

    for (int i = 0; i < 10; ++i) {
        uint32_t randomNum = prng.generateNum();
        cout << "ГСЧ" << i + 1 << ": " << randomNum << endl;
    }

    return 0;
}
