#include "mrtd_helpers.h"

#include <stdio.h> //TODO: remove

#include <mbedtls/sha1.h>
#include <mbedtls/des.h>

static inline unsigned char *ucstr(const char *str) { return (unsigned char *)str; }

uint8_t mrtd_bac_check_digit(const char* input, const uint8_t length) {
    const uint8_t num_weights = 3;
    uint8_t weights[] = {7, 3, 1};
    uint8_t check_digit = 0;
    uint8_t idx;

    for(uint8_t i=0; i<length; ++i) {
        char c = input[i];
        if(c >= 'A' && c <= 'Z') {
            idx = c - 'A' + 10;
        } else if(c >= 'a' && c <= 'z') {
            idx = c - 'a' + 10;
        } else if(c >= '0' && c <= '9') {
            idx = c - '0';
        } else {
            idx = 0;
        }
        check_digit = (check_digit + idx * weights[i%num_weights]) % 10;
    }
    return check_digit;
}

void mrtd_print_date(char* output, MrtdDate* date) {
    output[0] = (date->year / 10) + '0';
    output[1] = (date->year % 10) + '0';
    output[2] = (date->month / 10) + '0';
    output[3] = (date->month % 10) + '0';
    output[4] = (date->day / 10) + '0';
    output[5] = (date->day % 10) + '0';
}

bool mrtd_bac_get_kmrz(MrtdAuthData* auth, char* output, uint8_t output_size) {
    uint8_t idx = 0;
    uint8_t docnr_length = strlen(auth->doc_number);
    uint8_t cd_idx = 0;
    if(output_size < docnr_length + 16) {
        return false;
    }

    cd_idx = idx;
    for(uint8_t i=0; i<docnr_length; ++i) {
        char c = auth->doc_number[i];
        if(c >= 'a' && c <= 'z') {
            c = c - 'a' + 'A';
        }
        output[idx++] = c;
    }

    if(docnr_length < 9) {
        memset(output+idx, '<', 9-docnr_length);
        idx += 9-docnr_length;
    }

    output[idx++] = mrtd_bac_check_digit(output+cd_idx, docnr_length) + '0';

    cd_idx = idx;
    mrtd_print_date(output+idx, &auth->birth_date);
    idx += 6;
    output[idx++] = mrtd_bac_check_digit(output+cd_idx, 6) + '0';

    cd_idx = idx;
    mrtd_print_date(output+idx, &auth->expiry_date);
    idx += 6;
    output[idx++] = mrtd_bac_check_digit(output+cd_idx, 6) + '0';

    output[idx++] = '\x00';
    return true;
}

bool mrtd_bac_keys_from_seed(const uint8_t kseed[16], uint8_t ksenc[16], uint8_t ksmac[16]) {
    uint8_t hash[20];
    mbedtls_sha1_context ctx;
    mbedtls_sha1_init(&ctx);

    do {
        for(uint8_t i=1; i<=2; ++i) {
            if(mbedtls_sha1_starts(&ctx)) break;
            if(mbedtls_sha1_update(&ctx, kseed, 16)) break;
            if(mbedtls_sha1_update(&ctx, ucstr("\x00\x00\x00"), 3)) break;
            if(mbedtls_sha1_update(&ctx, &i, 1)) break;
            if(mbedtls_sha1_finish(&ctx, hash)) break;

            switch(i) {
                case 1:
                    memcpy(ksenc, hash, 16);
                    mbedtls_des_key_set_parity(ksenc);
                    mbedtls_des_key_set_parity(ksenc+8);
                    break;
                case 2:
                    memcpy(ksmac, hash, 16);
                    mbedtls_des_key_set_parity(ksmac);
                    mbedtls_des_key_set_parity(ksmac+8);
                    break;
            }
        }
    } while(false);

    mbedtls_sha1_free(&ctx);
    return true;
}

bool mrtd_bac_keys(MrtdAuthData* auth, uint8_t ksenc[16], uint8_t ksmac[16]) {
    uint8_t kmrz_max_length = MRTD_DOCNR_MAX_LENGTH + 16;
    char kmrz[kmrz_max_length];
    if(!mrtd_bac_get_kmrz(auth, kmrz, kmrz_max_length)) {
        return false;
    }

    printf("kmrz: %s\r\n", kmrz); //TODO: remove

    uint8_t hash[20];
    mbedtls_sha1((uint8_t*)kmrz, strlen(kmrz), hash);

    if(!mrtd_bac_keys_from_seed(hash, ksenc, ksmac)) {
        return false;
    }

    return true;
}

//NOTE: output size will be ((data_length+8)/8)*8
bool mrtd_bac_encrypt(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output) {
    uint8_t IV[8] = "\x00\x00\x00\x00\x00\x00\x00\x00";

    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set2key_enc(&ctx, key);
    if(mbedtls_des3_crypt_cbc(&ctx, MBEDTLS_DES_ENCRYPT, data_length, IV, data, output)) {
        return false;
    }
    mbedtls_des3_free(&ctx);

    return true;
}

bool mrtd_bac_decrypt(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output) {
    uint8_t IV[8] = "\x00\x00\x00\x00\x00\x00\x00\x00";

    mbedtls_des3_context ctx;
    mbedtls_des3_init(&ctx);
    mbedtls_des3_set2key_dec(&ctx, key);
    if(mbedtls_des3_crypt_cbc(&ctx, MBEDTLS_DES_DECRYPT, data_length, IV, data, output)) {
        return false;
    }
    mbedtls_des3_free(&ctx);

    return true;
}

bool mrtd_bac_decrypt_verify(const uint8_t* data, size_t data_length, uint8_t* key_enc, uint8_t* key_mac, uint8_t* output) {
    mrtd_bac_decrypt(data, data_length - 8, key_enc, output);

    uint8_t mac_calc[8];
    mrtd_bac_padded_mac(data, data_length - 8, key_mac, mac_calc);

    if(memcmp(mac_calc, data + data_length - 8, 8)) {
        printf( "MAC failed\n");
        return false;
    }
    return true;
}

bool mrtd_bac_mac(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output) {
    // MAC
    uint8_t mac[8];
    uint8_t xormac[8];
    uint8_t tmp[8];
    mbedtls_des_context ctx;

    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, key);

    memset(mac, 0, 8);
    for(size_t i=0; i<data_length / 8; ++i) {
        for(uint8_t j=0; j<8; ++j) {
            xormac[j] = mac[j] ^ data[i * 8 + j];
        }
        mbedtls_des_crypt_ecb(&ctx, xormac, mac);
    }

    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_dec(&ctx, key+8);
    mbedtls_des_crypt_ecb(&ctx, mac, tmp);

    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, key);
    mbedtls_des_crypt_ecb(&ctx, tmp, output);

    mbedtls_des_free(&ctx);

    return true;
}

bool mrtd_bac_padded_mac(const uint8_t* data, size_t data_length, uint8_t* key, uint8_t* output) {
    size_t newlength = ((data_length+8)/8)*8; // TODO: return this value too?
    uint8_t padded[newlength]; //TODO: input parameter
    memset(padded, 0, newlength);
    memcpy(padded, data, data_length);
    padded[data_length] = 0x80;

    if(!mrtd_bac_mac(padded, newlength, key, output)) {
        return false;
    }

    return true;
}

