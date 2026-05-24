#include <tarsau.h>

/* 2. VALIDATION & INTEGRITY ENGINE                                           */

/**
 * @brief Dosyanın her bir baytının 0-127 aralığında (ASCII) olup olmadığını kontrol eder.
 */
StatusCode validate_ascii_integrity(const char *filepath) {
    /* TODO: Dosya akışını stream ederek karakter bütünlük kontrolü */
    return TARSAU_SUCCESS;
}

/**
 * @brief Giriş dosyalarının toplam boyutunu ve adet sınırını denetler.
 */
StatusCode validate_constraints(const AppConfig *config) {
    /* TODO: 200MB ve 32 dosya limit kontrolleri */
    return TARSAU_SUCCESS;
}
