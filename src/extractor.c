#include <tarsau.h>

/* 4. EXTRACTOR MODULE (Deserialization & Unpacking)                         */

/**
 * @brief Hedef dizin yoksa hiyerarşik olarak oluşturur.
 */
StatusCode ensure_directory_exists(const char *dirpath) {
    /* TODO: mkdir() çağrıları ve sistem kontrolleri */
    return TARSAU_SUCCESS;
}

/**
 * @brief Arşiv dosyasının başlığını ayrıştırarak metadata kayıtlarını çıkarır.
 */
StatusCode parse_archive_header(FILE *archive_fp, FileMetadata *metadata_list, int *file_count) {
    /* TODO: İlk 10 baytın okunması ve '|' ile ayrılmış token'ların parse edilmesi */
    return TARSAU_SUCCESS;
}

/**
 * @brief Paketleri çözerek dosyaları orijinal izinleriyle diske yazar.
 */
StatusCode execute_archive_extraction(const AppConfig *config) {
    /* TODO: Dosyaların stream edilerek yazılması ve chmod() ile izin ataması */
    return TARSAU_SUCCESS;
}
